#include "SyncReminderNotificationManager.h"

#include "AppStatsEvents.h"
#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "StatsEventHandler.h"

namespace
{
constexpr int ONE_HOUR_MS = 1000 * 60 * 60; // 1 hour
constexpr int ONE_DAY_MS = ONE_HOUR_MS * 24; // 1 day
constexpr int TIME_TO_FIRST_REMINDER_MS = ONE_HOUR_MS * 2; // 2 hours
constexpr int TWO_HOURS_S = TIME_TO_FIRST_REMINDER_MS / 1000; // 2 hours in seconds
constexpr int FIFTEEN_MINS_S = TWO_HOURS_S / 8; // 15 minutes in seconds
constexpr quint64 DAYS_TO_SECOND_REMINDER = 10;
constexpr quint64 DAYS_TO_MONTHLY_REMINDER = 30;
constexpr quint64 DAYS_TO_BIMONTHLY_REMINDER = 60;
}

SyncReminderNotificationManager::SyncReminderNotificationManager(QObject* parent):
    QObject(parent),
    mState(ReminderState::FIRST_REMINDER),
    mClickedTime(-1)
{
    connect(&mTimer, &QTimer::timeout, this, &SyncReminderNotificationManager::onTimeout);

    // First reminder is shown after the remaining time to the next time only once,
    // then the timer is triggered every day (see onTimeout).
    mTimer.setSingleShot(true);
}

SyncReminderNotificationManager::~SyncReminderNotificationManager()
{
    if (mTimer.isActive())
    {
        mTimer.stop();
    }
}

void SyncReminderNotificationManager::update(bool isFirstTime)
{
    auto lastTime(Preferences::instance()->lastSyncReminderTime());
    auto lastSyncReminderTime(QDateTime::fromSecsSinceEpoch(lastTime));
    auto currentTime(QDateTime::currentDateTime());
    auto currentTimeSecs(currentTime.toSecsSinceEpoch());
    if (lastTime == 0 || lastTime > currentTimeSecs)
    {
        init(isFirstTime, lastSyncReminderTime, currentTime);
    }
    else
    {
        updateState();
        continuePeriodicProcess(lastSyncReminderTime, currentTime);
    }
}

void SyncReminderNotificationManager::sendEventsIfNeeded()
{
    if (mState == ReminderState::UNDEFINED)
    {
        return;
    }

    int reminderID(static_cast<int>(mState));
    if (mClickedTime != -1)
    {
        MegaSyncApp->getStatsEventHandler()->sendEvent(
            AppStatsEvents::EventType::SYNC_CREATED_AFTER_CLICKING_NOTIFICATION,
            {QString::number(reminderID)});
        resetClickedTime();
    }
    else
    {
        int currentTime(QDateTime::currentDateTime().toSecsSinceEpoch());
        int maxTime(mClickedTime + FIFTEEN_MINS_S);
        if (currentTime <= maxTime)
        {
            MegaSyncApp->getStatsEventHandler()->sendEvent(
                AppStatsEvents::EventType::SYNC_CREATED_AFTER_NOTIFICATION,
                {QString::number(reminderID)});
        }
    }
}

void SyncReminderNotificationManager::resetClickedTime()
{
    mClickedTime = -1;
}

void SyncReminderNotificationManager::init(bool isFirstTime,
                                           const QDateTime& lastTime,
                                           const QDateTime& currentTime)
{
    Preferences::instance()->setSyncReminderTime(currentTime.toSecsSinceEpoch());
    if (isFirstTime)
    {
        // If it is the first time the app is started after onboarding or the time is invalid
        // (for example when the system time is changed), then show the first reminder.
        mState = ReminderState::FIRST_REMINDER;
        mTimer.start(TIME_TO_FIRST_REMINDER_MS);
    }
    else
    {
        // For existing users, we will skip the first notification.
        updateState();
        startNextReminder(lastTime, currentTime);
    }
}

void SyncReminderNotificationManager::continuePeriodicProcess(const QDateTime& lastTime,
                                                              const QDateTime& currentTime)
{
    if (isPendingNotificationRequired())
    {
        // Show the last reminder if it is pending.
        showNotification();
        updatePreferences();
    }

    if (lastTime.isValid())
    {
        startNextReminder(lastTime, currentTime);
    }
    else
    {
        MegaSyncApp->getMegaApi()->log(mega::MegaApi::LOG_LEVEL_WARNING,
                                       QString::fromUtf8("Invalid last sync reminder time: %s")
                                           .arg(lastTime.toString())
                                           .toUtf8()
                                           .constData());
    }
}

void SyncReminderNotificationManager::startNextReminder(const QDateTime& lastTime,
                                                        const QDateTime& currentTime)
{
    // Trigger the timer to show the reminder using the remaining time to the next day
    // of the reminder.
    int msecsToNextReminder(calculateMsecsToNextReminder(lastTime, currentTime));
    mTimer.start(msecsToNextReminder);
}

void SyncReminderNotificationManager::updateState()
{
    quint64 daysCount(getDaysToNextReminder());
    auto lastState(static_cast<ReminderState>(Preferences::instance()->lastSyncReminderState()));
    if (daysCount > DAYS_TO_MONTHLY_REMINDER)
    {
        mState = ReminderState::BIMONTHLY;
    }
    else if (daysCount > DAYS_TO_SECOND_REMINDER)
    {
        mState = ReminderState::MONTHLY;
    }
    else if (lastState == ReminderState::UNDEFINED)
    {
        mState = ReminderState::FIRST_REMINDER;
    }
    else
    {
        mState = ReminderState::SECOND_REMINDER;
    }
}

void SyncReminderNotificationManager::updatePreferences()
{
    Preferences::instance()->setSyncReminderTime(QDateTime::currentDateTime().toSecsSinceEpoch());
    Preferences::instance()->setLastSyncReminderState(static_cast<int>(mState));
}

void SyncReminderNotificationManager::showNotification()
{
    auto [title, message] = getMessageForState();

    DesktopNotifications::NotificationInfo reminder;
    reminder.title = title;
    reminder.message = message;
    reminder.activatedFunction = [this](DesktopAppNotificationBase::Action)
    {
        mClickedTime = QDateTime::currentDateTime().toSecsSinceEpoch();
        CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::OS_NOTIFICATION_ORIGIN);
    };

    MegaSyncApp->showInfoMessage(reminder);

    sendShownEvents();
}

bool SyncReminderNotificationManager::isNotificationRequired()
{
    quint64 daysCount(getDaysToNextReminder());
    bool show(false);
    switch (mState)
    {
        case ReminderState::FIRST_REMINDER:
        {
            show = true;
            break;
        }
        case ReminderState::SECOND_REMINDER:
        {
            show = (daysCount >= DAYS_TO_SECOND_REMINDER);
            break;
        }
        case ReminderState::MONTHLY:
        {
            show = (daysCount >= DAYS_TO_MONTHLY_REMINDER);
            break;
        }
        case ReminderState::BIMONTHLY:
        {
            show = (daysCount >= DAYS_TO_BIMONTHLY_REMINDER);
            break;
        }
        default:
        {
            break;
        }
    }

    return show;
}

bool SyncReminderNotificationManager::isPendingNotificationRequired()
{
    bool isPending(false);

    auto lastState(static_cast<ReminderState>(Preferences::instance()->lastSyncReminderState()));
    switch (lastState)
    {
        case ReminderState::UNDEFINED:
        {
            isPending = isPendingNotificationRequiredUndefined(lastState);
            break;
        }
        case ReminderState::FIRST_REMINDER:
        // Fallthrough
        case ReminderState::SECOND_REMINDER:
        // Fallthrough
        case ReminderState::MONTHLY:
        {
            isPending = mState > lastState;
            break;
        }
        case ReminderState::BIMONTHLY:
        {
            quint64 daysCount(getDaysToNextReminder());
            isPending = daysCount > DAYS_TO_BIMONTHLY_REMINDER;
            break;
        }
        default:
        {
            break;
        }
    }

    return isPending;
}

bool SyncReminderNotificationManager::isPendingNotificationRequiredUndefined(
    ReminderState lastState)
{
    bool pending(false);
    if (mState == ReminderState::FIRST_REMINDER)
    {
        auto lastTime(
            QDateTime::fromSecsSinceEpoch(Preferences::instance()->lastSyncReminderTime()));
        auto currentTime(QDateTime::currentDateTime());
        auto secsDiff(lastTime.secsTo(currentTime));
        pending = (secsDiff > TWO_HOURS_S);
    }
    else
    {
        pending = mState > lastState;
    }
    return pending;
}

int SyncReminderNotificationManager::calculateMsecsToNextReminder(
    const QDateTime& lastTime,
    const QDateTime& currentTime) const
{
    QDateTime nextReminderTime;
    if (mState == ReminderState::FIRST_REMINDER)
    {
        nextReminderTime = lastTime.addSecs(TWO_HOURS_S);
    }
    else
    {
        auto remainingDays(0);
        switch (mState)
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

        // Calculate the time to the next reminder.
        nextReminderTime = lastTime.addDays(daysToCurrentTime);
    }

    return static_cast<int>(currentTime.msecsTo(nextReminderTime));
}

QPair<QString, QString> SyncReminderNotificationManager::getMessageForState() const
{
    QString title;
    QString message;

    if (mState == ReminderState::FIRST_REMINDER)
    {
        title = tr("You’re almost done");
        message = tr("Set up your first sync and backup to get the most out of the desktop app");
    }
    else
    {
        title = tr("Sync your data");
        message = tr("Access your data from anywhere, collaborate with ease, and instantly get "
                     "the most up-to-date version of your files");
    }

    return {title, message};
}

quint64 SyncReminderNotificationManager::getDaysToNextReminder() const
{
    auto lastTime(QDateTime::fromSecsSinceEpoch(Preferences::instance()->lastSyncReminderTime()));
    quint64 daysCount(0);
    if (lastTime.isValid())
    {
        auto currentTime(QDateTime::currentDateTime());
        daysCount = static_cast<quint64>(lastTime.daysTo(currentTime));
    }
    return daysCount;
}

SyncReminderNotificationManager::ReminderState
    SyncReminderNotificationManager::getNextState(ReminderState state) const
{
    return static_cast<ReminderState>(static_cast<int>(state) + 1);
}

void SyncReminderNotificationManager::sendShownEvents() const
{
    AppStatsEvents::EventType event(AppStatsEvents::EventType::NONE);
    switch (mState)
    {
        case ReminderState::FIRST_REMINDER:
        {
            event = AppStatsEvents::EventType::FIRST_SYNC_NOTIFICATION_SHOWN;
            break;
        }
        case ReminderState::SECOND_REMINDER:
        {
            event = AppStatsEvents::EventType::SECOND_SYNC_NOTIFICATION_SHOWN;
            break;
        }
        case ReminderState::MONTHLY:
        {
            event = AppStatsEvents::EventType::MONTHLY_SYNC_NOTIFICATION_SHOWN;
            break;
        }
        case ReminderState::BIMONTHLY:
        {
            event = AppStatsEvents::EventType::BIMONTHLY_SYNC_NOTIFICATION_SHOWN;
            break;
        }
        default:
        {
            break;
        }
    }
    MegaSyncApp->getStatsEventHandler()->sendEvent(event);
}

void SyncReminderNotificationManager::onTimeout()
{
    if (Preferences::instance()->isFirstSyncDone())
    {
        if (mTimer.isActive())
        {
            mTimer.stop();
        }
        return;
    }

    if (isNotificationRequired())
    {
        // Show the reminder if the conditions are met, update the time and update the states.
        showNotification();
        updatePreferences();
        if (mState != ReminderState::BIMONTHLY)
        {
            mState = getNextState(mState);
        }
    }

    // Trigger the timer every day if it is not already triggered.
    if (mTimer.isSingleShot())
    {
        mTimer.setSingleShot(false);
        mTimer.start(ONE_DAY_MS);
    }
}
