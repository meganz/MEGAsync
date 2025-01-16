#ifndef SYNC_REMINDER_NOTIFICATION_MANAGER_H
#define SYNC_REMINDER_NOTIFICATION_MANAGER_H

#include <QTimer>

class SyncReminderNotificationManager: public QObject
{
    Q_OBJECT

public:
    SyncReminderNotificationManager(QObject* parent = nullptr);
    ~SyncReminderNotificationManager();

    void update(bool isFirstTime = false);

private:
    enum class ReminderState
    {
        UNDEFINED = 0,
        FIRST_REMINDER = 1,
        SECOND_REMINDER = 2,
        MONTHLY = 3,
        BIMONTHLY = 4
    };

    ReminderState mState;
    QTimer mTimer;

    void init(bool isFirstTime, const QDateTime& lastTime, const QDateTime& currentTime);
    void continuePeriodicProcess(const QDateTime& lastTime, const QDateTime& currentTime);
    void startNextReminder(const QDateTime& lastTime, const QDateTime& currentTime);

    void updateState();
    void updatePreferences();

    void showNotification();

    bool isNotificationRequired();
    bool isPendingNotificationRequired();
    bool isPendingNotificationRequiredUndefined(ReminderState lastState);

    int calculateMsecsToNextReminder(const QDateTime& lastTime, const QDateTime& currentTime) const;
    QPair<QString, QString> getMessageForState() const;
    quint64 getDaysToNextReminder() const;

    ReminderState getNextState(ReminderState state) const;

private slots:
    void onTimeout();
};

#endif // SYNC_REMINDER_NOTIFICATION_MANAGER_H
