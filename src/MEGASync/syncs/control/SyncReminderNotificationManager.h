#ifndef SYNC_REMINDER_NOTIFICATION_MANAGER_H
#define SYNC_REMINDER_NOTIFICATION_MANAGER_H

#include <QTimer>

class SyncReminderNotificationManager: public QObject
{
    Q_OBJECT

public:
    SyncReminderNotificationManager(QObject* parent = nullptr);
    ~SyncReminderNotificationManager();

    void update();

private:
    enum class ReminderState
    {
        FIRST_REMINDER = 0,
        SECOND_REMINDER = 1,
        MONTHLY = 2,
        BIMONTHLY = 3
    };

    ReminderState mState;
    QTimer mTimer;

    QPair<QString, QString> getMessageForState() const;
    void showNotification();
    int calculateMsecsToNextReminder(const QDateTime& lastTime,
                                     const QDateTime& currentTime,
                                     quint64 daysToCurrentTime) const;
    void update(const QDateTime& lastSyncReminderTime, const QDateTime& currentTime);
    bool showReminder(const QDateTime& currentTime);

private slots:
    void onTimeout();
};

#endif // SYNC_REMINDER_NOTIFICATION_MANAGER_H
