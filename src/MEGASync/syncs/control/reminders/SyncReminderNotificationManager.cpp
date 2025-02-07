#include "SyncReminderNotificationManager.h"

#include "AppStatsEvents.h"
#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "StatsEventHandler.h"
#include "SyncController.h"
#include "SyncReminderAction.h"

namespace
{
constexpr qint64 TIME_TO_FIRST_REMINDER_MS = 1000 * 60 * 60 * 2; // 2 hours
constexpr qint64 TWO_HOURS_S = TIME_TO_FIRST_REMINDER_MS / 1000; // 2 hours in seconds
constexpr qint64 SECS_PER_DAY = 60 * 60 * 24;
constexpr qint64 SECS_TO_FIRST_REMINDER = 2 * 60 * 60; // 2 hours in seconds
constexpr qint64 DAYS_TO_SECOND_REMINDER = 10;
constexpr qint64 DAYS_TO_MONTHLY_REMINDER = 30;
constexpr qint64 DAYS_TO_BIMONTHLY_REMINDER = 60;
constexpr qint64 SECS_TO_BIMONTHLY_REMINDER = DAYS_TO_BIMONTHLY_REMINDER * SECS_PER_DAY;
const std::map<SyncReminderNotificationManager::ReminderState, qint64> STATE_DURATIONS = {
    {SyncReminderNotificationManager::ReminderState::FIRST_REMINDER, TWO_HOURS_S},
    {SyncReminderNotificationManager::ReminderState::SECOND_REMINDER,
     DAYS_TO_SECOND_REMINDER* SECS_PER_DAY},
    {SyncReminderNotificationManager::ReminderState::MONTHLY,
     DAYS_TO_MONTHLY_REMINDER* SECS_PER_DAY}};
const QLatin1String ENV_MEGA_REMINDER_DELAY_SECS("MEGA_REMINDER_DELAY_SECS");
const QLatin1String ENV_MEGA_REMINDER_DELAY_SECS_DEFAULT_VALUE("0");
const QLatin1String
    ENV_MEGA_SYNC_CREATION_MAX_INTERVAL_SECS("MEGA_SYNC_CREATION_MAX_INTERVAL_SECS");
const QLatin1String ENV_MEGA_SYNC_CREATION_MAX_INTERVAL_SECS_DEFAULT_VALUE("900"); // 15 mins
}

SyncReminderNotificationManager::SyncReminderNotificationManager(bool comesFromOnboarding):
    QObject(nullptr),
    mState(std::nullopt),
    mLastState(std::nullopt),
    mLastSyncReminderTime(std::nullopt)
{
    readFromPreferences();

    loadEnvVariables();

    if (mLastState != ReminderState::DONE)
    {
        connect(this,
                &SyncReminderNotificationManager::stateChanged,
                this,
                &SyncReminderNotificationManager::run);
        connect(&mTimer, &QTimer::timeout, this, &SyncReminderNotificationManager::onTimeout);

        initActions();
        init(comesFromOnboarding);
    }
}

SyncReminderNotificationManager::~SyncReminderNotificationManager()
{
    disconnect(this,
               &SyncReminderNotificationManager::stateChanged,
               this,
               &SyncReminderNotificationManager::run);
    mTimer.stop();
    mTimer.disconnect();
}

void SyncReminderNotificationManager::onSyncsDialogClosed()
{
    auto lastState(mLastState.value());
    if (mActions.count(lastState))
    {
        mActions[lastState]->resetClicked();
    }
}

void SyncReminderNotificationManager::onSyncAddRequestStatus(int errorCode,
                                                             int syncErrorCode,
                                                             QString name)
{
    Q_UNUSED(name);

    if (mLastState == ReminderState::DONE || errorCode != mega::MegaError::API_OK ||
        syncErrorCode != mega::MegaSync::NO_SYNC_ERROR)
    {
        return;
    }

    if (!mActions.count(mLastState.value()))
    {
        return;
    }

    int reminderID(static_cast<int>(mLastState.value()));
    if (mActions[mLastState.value()]->isClicked())
    {
        // Send event if a sync has been created after clicking in the notification.
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SYNC_CREATED_AFTER_CLICKING_NOTIFICATION,
            {QString::number(reminderID)});
    }
    else
    {
        auto currentTime(getCurrentTimeSecs());
        auto maxTime(mLastSyncReminderTime.value() + mTimeTestInfo.mSyncCreationMaxInterval);
        if (currentTime <= maxTime)
        {
            // Send event if a sync has been created before the first 15 mins. after
            // showing the notification.
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SYNC_CREATED_AFTER_NOTIFICATION,
                {QString::number(reminderID)});
        }
    }
    moveToDoneState();
    writeToPreferences();
}

void SyncReminderNotificationManager::readFromPreferences()
{
    int lastState(Preferences::instance()->lastSyncReminderState());
    if (lastState != 0)
    {
        mLastState = static_cast<ReminderState>(lastState);
    }
    mLastSyncReminderTime = Preferences::instance()->lastSyncReminderTime();
}

void SyncReminderNotificationManager::writeToPreferences()
{
    Preferences::instance()->setSyncReminderTime(mLastSyncReminderTime.value());
    Preferences::instance()->setLastSyncReminderState(static_cast<int>(mLastState.value()));
}

void SyncReminderNotificationManager::initActions()
{
    auto createAction = [this](ReminderState state, AppStatsEvents::EventType eventType)
    {
        return std::make_unique<SyncReminderAction>(this,
                                                    getNotificationTitle(state),
                                                    getNotificationMessage(state),
                                                    eventType);
    };

    mActions[ReminderState::FIRST_REMINDER] =
        createAction(ReminderState::FIRST_REMINDER,
                     AppStatsEvents::EventType::FIRST_SYNC_NOTIFICATION_SHOWN);
    mActions[ReminderState::SECOND_REMINDER] =
        createAction(ReminderState::SECOND_REMINDER,
                     AppStatsEvents::EventType::SECOND_SYNC_NOTIFICATION_SHOWN);
    mActions[ReminderState::MONTHLY] =
        createAction(ReminderState::MONTHLY,
                     AppStatsEvents::EventType::MONTHLY_SYNC_NOTIFICATION_SHOWN);
    mActions[ReminderState::BIMONTHLY] =
        createAction(ReminderState::BIMONTHLY,
                     AppStatsEvents::EventType::BIMONTHLY_SYNC_NOTIFICATION_SHOWN);
}

void SyncReminderNotificationManager::init(bool comesFromOnboarding)
{
    mTimer.setSingleShot(true);

    if (!mLastState.has_value())
    {
        // First execution, set the initial state and start the next timer.
        initFirstTime(comesFromOnboarding);
        startNextTimer();
    }
    else
    {
        // There is a previous state (info in preferences), so check if the current state should be
        // updated or not.
        if (calculateCurrentState())
        {
            // In case there is a time jump since the last time the application was launched and the
            // last pending notification has to be displayed (pending notification required when the
            // current time has passed the time to show the last reminder).
            updateWritePreferencesAndNotifyStateChanged();
        }
        else
        {
            // Normal case: no pending notification required, so start the next timer.
            startNextTimer();
        }
    }
}

void SyncReminderNotificationManager::initFirstTime(bool comesFromOnboarding)
{
    if (comesFromOnboarding)
    {
        // If it is the first time the app is started after onboarding or the time is invalid
        // (for example when the system time is changed), then show the first reminder.
        mLastState = ReminderState::INITIAL;
        mState = ReminderState::FIRST_REMINDER;
    }
    else
    {
        // For existing users, we will skip the first notification.
        mLastState = ReminderState::FIRST_REMINDER;
        mState = ReminderState::SECOND_REMINDER;
    }
    mLastSyncReminderTime = getCurrentTimeSecs();
    writeToPreferences();
}

void SyncReminderNotificationManager::onTimeout()
{
    if (updateState())
    {
        updateWritePreferencesAndNotifyStateChanged();
    }
}

bool SyncReminderNotificationManager::updateState()
{
    auto currentState(mState);
    switch (mState.value())
    {
        case ReminderState::FIRST_REMINDER:
        {
            if (isNeededToChangeFirstState())
            {
                mState = ReminderState::SECOND_REMINDER;
            }
            break;
        }
        case ReminderState::SECOND_REMINDER:
        {
            if (isNeededToChangeState(DAYS_TO_SECOND_REMINDER))
            {
                mState = ReminderState::MONTHLY;
            }
            break;
        }
        case ReminderState::MONTHLY:
        {
            if (isNeededToChangeState(DAYS_TO_MONTHLY_REMINDER))
            {
                mState = ReminderState::BIMONTHLY;
            }
            break;
        }
        case ReminderState::INITIAL:
        // Fallthrough
        case ReminderState::BIMONTHLY:
        // Fallthrough
        case ReminderState::DONE:
        // Fallthrough
        default:
        {
            break;
        }
    }

    mLastState = currentState;

    return currentState != mState || (mState == ReminderState::BIMONTHLY &&
                                      isNeededToChangeState(DAYS_TO_BIMONTHLY_REMINDER));
}

bool SyncReminderNotificationManager::isNeededToChangeFirstState() const
{
    qint64 secsCount(getSecsFromLastReminder());
    return secsCount >= SECS_TO_FIRST_REMINDER;
}

bool SyncReminderNotificationManager::isNeededToChangeState(qint64 daysToNextReminder) const
{
    qint64 daysCount(getDaysFromLastReminder());
    return daysCount >= daysToNextReminder;
}

qint64 SyncReminderNotificationManager::getSecsFromLastReminder() const
{
    auto lastTime(QDateTime::fromSecsSinceEpoch(mLastSyncReminderTime.value()));
    auto currentTime(QDateTime::fromSecsSinceEpoch(getCurrentTimeSecs()));
    return static_cast<qint64>(lastTime.secsTo(currentTime));
}

qint64 SyncReminderNotificationManager::getDaysFromLastReminder() const
{
    auto lastTime(QDateTime::fromSecsSinceEpoch(mLastSyncReminderTime.value()));
    auto currentTime(QDateTime::fromSecsSinceEpoch(getCurrentTimeSecs()));
    return static_cast<qint64>(lastTime.daysTo(currentTime));
}

void SyncReminderNotificationManager::run()
{
    if (mState.value() == ReminderState::DONE ||
        mActions.find(mLastState.value()) == mActions.end())
    {
        return;
    }

    mActions[mLastState.value()]->run();
}

QString SyncReminderNotificationManager::getNotificationTitle(ReminderState state) const
{
    QString title;
    switch (state)
    {
        case ReminderState::FIRST_REMINDER:
        {
            title = tr("You’re almost done");
            break;
        }
        case ReminderState::SECOND_REMINDER:
        // Fallthrough
        case ReminderState::MONTHLY:
        // Fallthrough
        case ReminderState::BIMONTHLY:
        {
            title = tr("Sync your data");
            break;
        }
        default:
        {
            break;
        }
    }
    return title;
}

QString SyncReminderNotificationManager::getNotificationMessage(ReminderState state) const
{
    QString message;
    switch (state)
    {
        case ReminderState::FIRST_REMINDER:
        {
            message =
                tr("Set up your first sync and backup to get the most out of the desktop app");
            break;
        }
        case ReminderState::SECOND_REMINDER:
        // Fallthrough
        case ReminderState::MONTHLY:
        // Fallthrough
        case ReminderState::BIMONTHLY:
        {
            message = tr("Access your data from anywhere, collaborate with ease, and instantly get "
                         "the most up-to-date version of your files");
            break;
        }
        default:
        {
            break;
        }
    }
    return message;
}

bool SyncReminderNotificationManager::calculateCurrentState()
{
    // Update the state based on the elapsed time since the last reminder to check
    // if the current state should be changed (jump time).
    qint64 elapsedSeconds = getSecsFromLastReminder();
    auto lastState(mLastState.value());
    const auto state(std::find_if(STATE_DURATIONS.cbegin(),
                                  STATE_DURATIONS.cend(),
                                  [elapsedSeconds, lastState](const auto& entry)
                                  {
                                      return entry.first > lastState &&
                                             elapsedSeconds <= entry.second;
                                  }));
    mState = (state != STATE_DURATIONS.end()) ? state->first : ReminderState::BIMONTHLY;

    // Check if the previous state for the new state is the expected one for the
    // saved last state. If not, then the state should be updated to the expected one
    // and the pending notification should be shown.
    // Bimonthly is a special case because it doesn't have a next state until it is done.
    ReminderState expectedLastState(getPreviousState());
    bool existPendingNotification(expectedLastState > mLastState.value() || isBimonthlyPending());
    if (existPendingNotification)
    {
        mLastState = expectedLastState;
    }

    return existPendingNotification;
}

int SyncReminderNotificationManager::calculateMsecsToCurrentState() const
{
    QDateTime nextReminderTime;
    auto lastTime(QDateTime::fromSecsSinceEpoch(mLastSyncReminderTime.value()));
    auto currentTime(QDateTime::fromSecsSinceEpoch(getCurrentTimeSecs()));
    if (mState.value() == ReminderState::FIRST_REMINDER)
    {
        nextReminderTime = lastTime.addSecs(TWO_HOURS_S);
    }
    else
    {
        auto remainingDays(0);
        switch (mState.value())
        {
            case ReminderState::SECOND_REMINDER:
            {
                remainingDays = DAYS_TO_SECOND_REMINDER;
                break;
            }
            case ReminderState::MONTHLY:
            {
                remainingDays = DAYS_TO_MONTHLY_REMINDER;
                break;
            }
            case ReminderState::BIMONTHLY:
            {
                remainingDays = DAYS_TO_BIMONTHLY_REMINDER;
                break;
            }
            default:
            {
                break;
            }
        }

        // Calculate the time to the next reminder.
        auto daysToCurrentTime(getDaysToCurrentTime(lastTime, currentTime, remainingDays));
        nextReminderTime = lastTime.addDays(daysToCurrentTime);
    }

    return static_cast<int>(currentTime.msecsTo(nextReminderTime));
}

void SyncReminderNotificationManager::startNextTimer()
{
    if (mTimer.isActive())
    {
        mTimer.stop();
    }

    auto msecsToNextReminder(calculateMsecsToCurrentState());
    if (msecsToNextReminder <= 0)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_WARNING,
                           QString::fromLatin1("Msecs to next sync reminder is negative: %1")
                               .arg(msecsToNextReminder)
                               .toUtf8()
                               .constData());
        return;
    }

    mTimer.start(msecsToNextReminder);
}

void SyncReminderNotificationManager::moveToDoneState()
{
    mState = ReminderState::DONE;
    mLastState = ReminderState::DONE;
    mLastSyncReminderTime = getCurrentTimeSecs();
    if (mTimer.isActive())
    {
        mTimer.stop();
    }
}

qint64 SyncReminderNotificationManager::getCurrentTimeSecs() const
{
    return QDateTime::currentDateTime().toSecsSinceEpoch() + mTimeTestInfo.mDelay;
}

void SyncReminderNotificationManager::loadEnvVariables()
{
    mTimeTestInfo.mDelay =
        loadEnvVariable(ENV_MEGA_REMINDER_DELAY_SECS, ENV_MEGA_REMINDER_DELAY_SECS_DEFAULT_VALUE);
    mTimeTestInfo.mSyncCreationMaxInterval =
        loadEnvVariable(ENV_MEGA_SYNC_CREATION_MAX_INTERVAL_SECS,
                        ENV_MEGA_SYNC_CREATION_MAX_INTERVAL_SECS_DEFAULT_VALUE);
}

qint64 SyncReminderNotificationManager::loadEnvVariable(const QString& envVarName,
                                                        const QString& envVarDefaultValue)
{
    QString value(
        QProcessEnvironment::systemEnvironment().value(envVarName, envVarDefaultValue).trimmed());
    return static_cast<qint64>(QVariant(value).toLongLong());
}

SyncReminderNotificationManager::ReminderState
    SyncReminderNotificationManager::getPreviousState() const
{
    return static_cast<ReminderState>(static_cast<int>(mState.value()) - 1);
}

qint64 SyncReminderNotificationManager::getDaysToCurrentTime(const QDateTime& lastTime,
                                                             const QDateTime& currentTime,
                                                             qint64 remainingDays) const
{
    // Increment the days to the next reminder if the current time is closer to the next
    // reminder than to the current time or use the days to the next reminder.
    auto daysToCurrentTime(lastTime.daysTo(currentTime));
    if (daysToCurrentTime < remainingDays)
    {
        daysToCurrentTime++;
    }
    else
    {
        daysToCurrentTime = remainingDays;
    }
    return daysToCurrentTime;
}

bool SyncReminderNotificationManager::isBimonthlyPending() const
{
    if (mState != ReminderState::BIMONTHLY)
    {
        return false;
    }

    auto secsToNextReminder(getSecsFromLastReminder());
    return secsToNextReminder >= SECS_TO_BIMONTHLY_REMINDER;
}

void SyncReminderNotificationManager::updateWritePreferencesAndNotifyStateChanged()
{
    mLastSyncReminderTime = getCurrentTimeSecs();
    writeToPreferences();
    emit stateChanged();
}
