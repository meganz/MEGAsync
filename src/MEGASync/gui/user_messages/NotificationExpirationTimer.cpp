#include "NotificationExpirationTimer.h"

#include <QDateTime>

namespace
{
constexpr int MSEC_IN_1_SEC = 1000;
constexpr int SECS_IN_1_HOUR = 3600;
constexpr int SECS_IN_1_DAY = 86400;
}

NotificationExpirationTimer::NotificationExpirationTimer(QObject* parent):
    QTimer(parent),
    mLastTimeInterval(0),
    mExpirationTimeSecs(0),
    mStopped(false)
{
    connect(this, &QTimer::timeout, this, &NotificationExpirationTimer::onTimeout);
}

NotificationExpirationTimer::NotificationExpirationTimer(int64_t expirationTimeSecs,
                                                         QObject* parent):
    QTimer(parent),
    mLastTimeInterval(0),
    mStopped(false)
{
    connect(this, &QTimer::timeout, this, &NotificationExpirationTimer::onTimeout);
    start(static_cast<int>(expirationTimeSecs));
}

void NotificationExpirationTimer::startExpirationTime(int64_t expirationTimeSecs)
{
    stop();
    mExpirationTimeSecs = expirationTimeSecs;
    mStopped = false;
    onTimeout();
}

void NotificationExpirationTimer::stopExpirationTime()
{
    stop();
    mStopped = true;
}

int64_t NotificationExpirationTimer::getRemainingTime() const
{
    auto currentSecs = QDateTime::currentSecsSinceEpoch();
    auto diff = mExpirationTimeSecs - currentSecs;
    return diff;
}

void NotificationExpirationTimer::singleShot(int64_t remainingTimeSecs)
{
    TimeInterval timeInterval(remainingTimeSecs);
    int interval = 0;

    if (timeInterval.days > 0 && timeInterval.days > mLastTimeInterval.days)
    {
        // Time until the next change of day
        int secondsUntilNextDay =
            static_cast<int>(remainingTimeSecs) - timeInterval.days * SECS_IN_1_DAY;
        interval = secondsUntilNextDay * MSEC_IN_1_SEC;
    }
    else if ((timeInterval.days == 1 && timeInterval.hours == 0) ||
             (timeInterval.hours > 0 && timeInterval.hours > mLastTimeInterval.hours))
    {
        // Time until the next change of hour
        int secondsUntilNextHour =
            static_cast<int>(remainingTimeSecs) - timeInterval.hours * SECS_IN_1_HOUR;
        interval = secondsUntilNextHour * MSEC_IN_1_SEC;
    }
    else
    {
        // Update every second
        interval = MSEC_IN_1_SEC;
    }

    if (interval > 0)
    {
        stop();
        mLastTimeInterval = timeInterval;
        setInterval(interval);
        start();
    }
}

void NotificationExpirationTimer::onTimeout()
{
    if (mStopped)
    {
        return;
    }

    int64_t remainingTimeSecs = getRemainingTime();
    emit expired(static_cast<int>(remainingTimeSecs));
    singleShot(remainingTimeSecs);
}
