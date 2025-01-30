#ifndef SYNC_REMINDER_ACTION_H
#define SYNC_REMINDER_ACTION_H

#include "AppStatsEvents.h"
#include "CreateRemoveSyncsManager.h"
#include "DesktopNotifications.h"
#include "MegaApplication.h"

class SyncReminderNotificationManager;

class SyncReminderAction
{
public:
    SyncReminderAction(SyncReminderNotificationManager* manager,
                       const QString& title,
                       const QString& message,
                       AppStatsEvents::EventType eventType);
    virtual ~SyncReminderAction() = default;

    void run();
    void resetClicked();
    bool isClicked() const;

private:
    SyncReminderNotificationManager* mManager;
    QString mTitle;
    QString mMessage;
    AppStatsEvents::EventType mEventType;
    bool mClicked;

    void showNotification();
    void sendShownEvent() const;
};

#endif // SYNC_REMINDER_ACTION_H
