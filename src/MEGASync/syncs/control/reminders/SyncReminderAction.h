#ifndef SYNC_REMINDER_ACTION_H
#define SYNC_REMINDER_ACTION_H

#include "AppStatsEvents.h"
#include "CreateRemoveSyncsManager.h"
#include "DesktopNotifications.h"
#include "MegaApplication.h"

#include <QObject>

class SyncReminderNotificationManager;

class SyncReminderAction: public QObject
{
    Q_OBJECT

public:
    SyncReminderAction(SyncReminderNotificationManager* manager,
                       AppStatsEvents::EventType eventType);
    virtual ~SyncReminderAction() = default;

    void run();
    void resetClicked();
    bool isClicked() const;

protected:
    virtual QString getTitle() const = 0;
    virtual QString getMessage() const = 0;

private:
    SyncReminderNotificationManager* mManager;
    AppStatsEvents::EventType mEventType;
    bool mClicked;

    void showNotification();
    void sendShownEvent() const;
};

#endif // SYNC_REMINDER_ACTION_H
