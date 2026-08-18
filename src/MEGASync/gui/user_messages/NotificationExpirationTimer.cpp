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
    mExpirationTimeSecs(0),
    mStopped(false)
{
    connect(this, &QTimer::timeout, this, &NotificationExpirationTimer::onTimeout);
}

NotificationExpirationTimer::NotificationExpirationTimer(int64_t expirationTimeSecs,
                                                         QObject* parent):
    NotificationExpirationTimer(parent)
{
    startExpirationTime(expirationTimeSecs);
}

void NotificationExpirationTimer::startExpirationTime(int64_t expirationTimeSecs)
{
    stop();
    mExpirationTimeSecs = expirationTimeSecs;
    mStopped = false;

    // Only schedule the next update. The current remaining time is not reported
    // here because the caller displays it itself when it arms the countdown, and
    // reporting it would repeat the expiration handling on every re-arm.
    singleShot(getRemainingTime());
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
    // NotificationItem displays the remaining time rounded down, built from
    // remainingTimeSecs - 1. So the text changes when the remaining time
    // reaches the next lower multiple of the unit being displayed.
    const int64_t displayedSecs = remainingTimeSecs - 1;
    int64_t unitSecs = 1;

    if (displayedSecs >= SECS_IN_1_DAY)
    {
        unitSecs = SECS_IN_1_DAY;
    }
    else if (displayedSecs >= SECS_IN_1_HOUR)
    {
        unitSecs = SECS_IN_1_HOUR;
    }

    // Never zero: (displayedSecs / unitSecs) * unitSecs is at most displayedSecs,
    // which is remainingTimeSecs - 1.
    const int64_t secsUntilNextChange =
        (unitSecs > 1) ? remainingTimeSecs - (displayedSecs / unitSecs) * unitSecs : 1;

    stop();
    setInterval(static_cast<int>(secsUntilNextChange * MSEC_IN_1_SEC));
    start();
}

void NotificationExpirationTimer::onTimeout()
{
    if (mStopped)
    {
        return;
    }

    int64_t remainingTimeSecs = getRemainingTime();
    emit expired(static_cast<int>(remainingTimeSecs));

    // A receiver may have stopped the countdown while handling the expiration, in
    // which case the timer must not be armed again: it repeats, so it would keep
    // waking up every second only to return here.
    if (mStopped)
    {
        return;
    }

    singleShot(remainingTimeSecs);
}
