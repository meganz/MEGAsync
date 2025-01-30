#ifndef SYNC_REMINDER_NOTIFICATION_MANAGER_H
#define SYNC_REMINDER_NOTIFICATION_MANAGER_H

#include <QObject>
#include <QString>
#include <QTimer>

#include <memory>
#include <optional>

class SyncReminderAction;

class SyncReminderNotificationManager: public QObject
{
    Q_OBJECT

    friend class SyncReminderAction;

public:
    enum class ReminderState
    {
        INITIAL = 1,
        FIRST_REMINDER = 2,
        SECOND_REMINDER = 3,
        MONTHLY = 4,
        BIMONTHLY = 5,
        DONE = 6
    };

    SyncReminderNotificationManager(bool comesFromOnboarding);
    ~SyncReminderNotificationManager();

public slots:
    void onSyncsDialogClosed();
    void onSyncAddRequestStatus(int errorCode, int syncErrorCode, QString name);

signals:
    void stateChanged();

private:
    std::map<ReminderState, std::unique_ptr<SyncReminderAction>> mActions;
    std::optional<ReminderState> mState;
    std::optional<ReminderState> mLastState;
    std::optional<qint64> mLastSyncReminderTime;
    QTimer mTimer;

    void readFromPreferences();
    void writeToPreferences();
    void initActions();
    void init(bool comesFromOnboarding);
    void initFirstTime(bool comesFromOnboarding);
    void updateStates();
    void updateState();
    bool isNeededToChangeFirstState() const;
    bool isNeededToChangeState(quint64 daysToNextReminder) const;
    quint64 getSecsToNextReminder() const;
    quint64 getDaysToNextReminder() const;
    QString getNotificationTitle(ReminderState state) const;
    QString getNotificationMessage(ReminderState state) const;
    int calculateMsecsToCurrentState() const;
    void startNextTimer();
    void calculateCurrentState();
    void moveToDoneState();

private slots:
    void run();
    void onTimeout();
};

#endif // SYNC_REMINDER_NOTIFICATION_MANAGER_H
