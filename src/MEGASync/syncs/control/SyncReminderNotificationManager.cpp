#include "SyncReminderNotificationManager.h"

#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"
#include "Preferences.h"

namespace
{
constexpr int ONE_HOUR_MS = 1000 * 60 * 60; // 1 hour
constexpr int ONE_DAY_MS = ONE_HOUR_MS * 24; // 1 day
constexpr int TIME_TO_FIRST_REMINDER_MS = ONE_HOUR_MS * 2; // 2 hours
constexpr quint64 DAYS_TO_SECOND_REMINDER = 10;
constexpr quint64 DAYS_TO_MONTHLY_REMINDER = 30;
constexpr quint64 DAYS_TO_BIMONTHLY_REMINDER = 60;
}

SyncReminderNotificationManager::SyncReminderNotificationManager(QObject* parent):
    QObject(parent),
    mState(ReminderState::FIRST_REMINDER)
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
    if (lastTime == 0 || lastTime > currentTime.currentSecsSinceEpoch())
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
            update(lastSyncReminderTime, currentTime);
        }
    }
    else
    {
        if (lastSyncReminderTime.isValid())
        {
            update(lastSyncReminderTime, currentTime);
        }
        else
        {
            MegaSyncApp->getMegaApi()->log(mega::MegaApi::LOG_LEVEL_WARNING,
                                           QString::fromUtf8("Invalid last sync reminder time: %s")
                                               .arg(lastSyncReminderTime.toString())
                                               .toUtf8()
                                               .constData());
        }
    }
}

void SyncReminderNotificationManager::update(const QDateTime& lastSyncReminderTime,
                                             const QDateTime& currentTime)
{
    auto daysCount(lastSyncReminderTime.daysTo(currentTime));
    if (daysCount >= DAYS_TO_MONTHLY_REMINDER)
    {
        mState = ReminderState::BIMONTHLY;
    }
    else if (daysCount >= DAYS_TO_SECOND_REMINDER)
    {
        mState = ReminderState::MONTHLY;
    }
    else
    {
        mState = ReminderState::SECOND_REMINDER;
    }

    // Trigger the timer to show the reminder using the remaining time to the next day
    // of the reminder.
    auto msecsToNextReminder(
        calculateMsecsToNextReminder(lastSyncReminderTime, currentTime, daysCount));
    mTimer.start(msecsToNextReminder);
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

void SyncReminderNotificationManager::showNotification()
{
    auto [title, message] = getMessageForState();

    DesktopNotifications::NotificationInfo reminder;
    reminder.title = title;
    reminder.message = message;
    reminder.activatedFunction = [](DesktopAppNotificationBase::Action)
    {
        CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::OS_NOTIFICATION_ORIGIN);
    };

    MegaSyncApp->showInfoMessage(reminder);
}

int SyncReminderNotificationManager::calculateMsecsToNextReminder(const QDateTime& lastTime,
                                                                  const QDateTime& currentTime,
                                                                  quint64 daysToCurrentTime) const
{
    auto daysToNextReminder(0);
    switch (mState)
    {
        case ReminderState::SECOND_REMINDER:
        {
            daysToNextReminder = DAYS_TO_SECOND_REMINDER;
            break;
        }
        case ReminderState::MONTHLY:
        {
            daysToNextReminder = DAYS_TO_MONTHLY_REMINDER;
            break;
        }
        case ReminderState::BIMONTHLY:
        {
            daysToNextReminder = DAYS_TO_BIMONTHLY_REMINDER;
            break;
        }
        default:
        {
            break;
        }
    }

    // Increment the days to the next reminder if the current time is closer to the next reminder
    // than to the current time or use the days to the next reminder.
    daysToCurrentTime =
        (daysToCurrentTime < daysToNextReminder) ? daysToCurrentTime++ : daysToNextReminder;

    // Calculate the time to the next reminder.
    auto nextReminderTime(lastTime.addDays(daysToCurrentTime));
    return currentTime.msecsTo(nextReminderTime);
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

    auto currentTime(QDateTime::currentDateTime());
    if (showReminder(currentTime))
    {
        // Show the reminder if the conditions are met, update the time in preferences and update
        // the state.
        showNotification();
        Preferences::instance()->setSyncReminderTime(currentTime.toSecsSinceEpoch());
        if (mState != ReminderState::BIMONTHLY)
        {
            mState = static_cast<ReminderState>(static_cast<int>(mState) + 1);
        }
    }

    // Trigger the timer every day if it is not already triggered.
    if (mTimer.isSingleShot())
    {
        mTimer.setSingleShot(false);
        mTimer.start(ONE_DAY_MS);
    }
}

bool SyncReminderNotificationManager::showReminder(const QDateTime& currentTime)
{
    auto lastSyncReminderTime(
        QDateTime::fromSecsSinceEpoch(Preferences::instance()->lastSyncReminderTime()));
    quint64 daysCount(0);
    if (lastSyncReminderTime.isValid())
    {
        daysCount = lastSyncReminderTime.daysTo(currentTime);
    }

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
            // auto secs(lastSyncReminderTime.msecsTo(currentTime));
            // show = secs >= 30000;
            show = (daysCount >= DAYS_TO_SECOND_REMINDER);
            break;
        }
        case ReminderState::MONTHLY:
        {
            // auto secs(lastSyncReminderTime.msecsTo(currentTime));
            // show = secs >= 60000;
            show = (daysCount >= DAYS_TO_MONTHLY_REMINDER);
            break;
        }
        case ReminderState::BIMONTHLY:
        {
            // auto secs(lastSyncReminderTime.msecsTo(currentTime));
            // show = secs >= 90000;
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
