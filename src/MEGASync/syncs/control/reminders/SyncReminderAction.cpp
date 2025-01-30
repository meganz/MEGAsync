#include "SyncReminderAction.h"

#include "StatsEventHandler.h"
#include "SyncReminderNotificationManager.h"

SyncReminderAction::SyncReminderAction(SyncReminderNotificationManager* manager,
                                       const QString& title,
                                       const QString& message,
                                       AppStatsEvents::EventType eventType):
    mManager(manager),
    mTitle(title),
    mMessage(message),
    mEventType(eventType),
    mClicked(false)
{}

void SyncReminderAction::run()
{
    showNotification();
    sendShownEvent();
    mManager->startNextTimer();
}

void SyncReminderAction::resetClicked()
{
    mClicked = false;
}

bool SyncReminderAction::isClicked() const
{
    return mClicked;
}

void SyncReminderAction::showNotification()
{
    DesktopNotifications::NotificationInfo reminder;
    reminder.title = mTitle;
    reminder.message = mMessage;
    reminder.activatedFunction = [this](DesktopAppNotificationBase::Action)
    {
        mClicked = true;
        CreateRemoveSyncsManager::addSync(SyncInfo::SyncOrigin::OS_NOTIFICATION_ORIGIN);
    };

    MegaSyncApp->showInfoMessage(reminder);
}

void SyncReminderAction::sendShownEvent() const
{
    MegaSyncApp->getStatsEventHandler()->sendEvent(mEventType);
}
