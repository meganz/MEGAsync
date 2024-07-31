#ifndef NOTIFICATION_EXPIRATION_TIMER_H
#define NOTIFICATION_EXPIRATION_TIMER_H

#include "Utilities.h"

#include <QTimer>
#include <QObject>

class NotificationExpirationTimer : public QTimer
{
    Q_OBJECT

public:
    explicit NotificationExpirationTimer(QObject* parent = nullptr);
    explicit NotificationExpirationTimer(int64_t expirationTimeSecs,
                                         QObject* parent = nullptr);
    virtual ~NotificationExpirationTimer() = default;

    void startExpirationTime(int64_t expirationTimeSecs);
    int64_t getRemainingTime() const;

signals:
    void expired(int remainingTimeSecs);

private slots:
    void onTimeout();

private:
    TimeInterval mLastTimeInterval;
    int64_t mExpirationTimeSecs;

    void singleShot(int64_t remainingTimeSecs);

};

#endif // NOTIFICATION_EXPIRATION_TIMER_H
