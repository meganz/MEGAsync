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
    struct TimeTestInfo
    {
        qint64 mDelay;
        qint64 mSyncCreationMaxInterval;
    };

    std::map<ReminderState, std::unique_ptr<SyncReminderAction>> mActions;
    std::optional<ReminderState> mState;
    std::optional<ReminderState> mLastState;
    std::optional<qint64> mLastSyncReminderTime;
    QTimer mTimer;
    TimeTestInfo mTimeTestInfo;

    void readFromPreferences();
    void writeToPreferences();
    void initActions();
    void init(bool comesFromOnboarding);
    void initFirstTime(bool comesFromOnboarding);
    void updateStates();
    void updateState();
    bool isNeededToChangeFirstState() const;
    bool isNeededToChangeState(qint64 daysToNextReminder) const;
    qint64 getSecsToNextReminder() const;
    qint64 getDaysToNextReminder() const;
    QString getNotificationTitle(ReminderState state) const;
    QString getNotificationMessage(ReminderState state) const;
    int calculateMsecsToCurrentState() const;
    void startNextTimer();
    void calculateCurrentState();
    void moveToDoneState();
    qint64 getCurrentTimeSecs() const;
    void loadEnvVariables();
    qint64 loadEnvVariable(const QString& envVarName, const QString& envVarDefaultValue);
    ReminderState getPreviousState() const;
    qint64 getDaysToCurrentTime(const QDateTime& lastTime,
                                const QDateTime& currentTime,
                                qint64 remainingDays) const;
    bool isBimonthlyPending() const;

private slots:
    void run();
    void onTimeout();
};

#endif // SYNC_REMINDER_NOTIFICATION_MANAGER_H
