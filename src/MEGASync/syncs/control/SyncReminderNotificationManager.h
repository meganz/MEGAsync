#ifndef SYNC_REMINDER_NOTIFICATION_MANAGER_H
#define SYNC_REMINDER_NOTIFICATION_MANAGER_H

#include <QDateTime>
#include <QObject>
#include <QPair>
#include <QString>
#include <QTimer>

class SyncReminderNotificationManager: public QObject
{
    Q_OBJECT

public:
    SyncReminderNotificationManager();
    ~SyncReminderNotificationManager();

    void update(bool isFirstTime = false);

public slots:
    void onSyncsDialogClosed();
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);

private:
    enum class ReminderState
    {
        UNDEFINED = 0,
        FIRST_REMINDER = 1,
        SECOND_REMINDER = 2,
        MONTHLY = 3,
        BIMONTHLY = 4,
        DONE = 5
    };

    ReminderState mState;
    QTimer mTimer;
    qint64 mLastShowedTime;
    bool mClicked;

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

    void sendShownEvents() const;

    void moveToDoneState();
    void resetClickedInfo();

private slots:
    void onTimeout();
};

#endif // SYNC_REMINDER_NOTIFICATION_MANAGER_H
