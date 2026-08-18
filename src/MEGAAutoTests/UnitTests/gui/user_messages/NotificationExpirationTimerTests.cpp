#include "NotificationExpirationTimer.h"
#include "Utilities.h"
#include <catch.hpp>

#include <QDateTime>
#include <QThread>

/**
 * Tests for the countdown shown by a notification with an end timestamp
 * (i.e. a discount offer) in the InfoDialog notifications tab.
 *
 * NotificationExpirationTimer does not tick once per second: it tries to sleep
 * until the next change of the *displayed* text, which is rounded down to the
 * nearest day, hour, minute or second. So the contract under test is:
 *
 *   after arming the timer with an absolute expiration timestamp, the timer
 *   must be active and its interval must be the time left until the displayed
 *   text changes.
 *
 * Arming is synchronous (startExpirationTime() calls onTimeout() itself), so
 * these tests read interval()/isActive() straight away and need no event loop.
 */

namespace
{
constexpr int64_t SECS_IN_1_MIN = 60;
constexpr int64_t SECS_IN_1_HOUR = 3600;
constexpr int64_t SECS_IN_1_DAY = 86400;
constexpr int MSEC_IN_1_SEC = 1000;

// The timer reads the wall clock on its own, so a deadline computed here as
// "now + X" is off by one second if the current second changes in between.
// Waiting for the beginning of a second leaves ~800 ms of margin.
void alignToSecondStart()
{
    while (QDateTime::currentMSecsSinceEpoch() % MSEC_IN_1_SEC > 200)
    {
        QThread::msleep(10);
    }
}

int64_t deadlineIn(int64_t secs)
{
    return QDateTime::currentSecsSinceEpoch() + secs;
}

void checkIntervalIs(const NotificationExpirationTimer& timer, int64_t expectedSecs)
{
    // An inactive timer means the countdown has been dropped and the displayed
    // text will never be updated again.
    REQUIRE(timer.isActive());

    const int64_t expectedMSecs = expectedSecs * MSEC_IN_1_SEC;
    const int64_t deviation = std::abs(static_cast<int64_t>(timer.interval()) - expectedMSecs);
    CAPTURE(timer.interval(), expectedMSecs);
    CHECK(deviation <= MSEC_IN_1_SEC);
}
}

TEST_CASE("TimeInterval splits seconds into day, hour, minute and second fields",
          "[notification-timer]")
{
    // The fields are a broken down representation (hours are modulo 24), not
    // cumulative totals. NotificationExpirationTimer relies on this.
    const TimeInterval interval(4 * SECS_IN_1_DAY + 23 * SECS_IN_1_HOUR + 59 * SECS_IN_1_MIN + 59);

    CHECK(interval.days == 4);
    CHECK(interval.hours == 23);
    CHECK(interval.minutes == 59);
    CHECK(interval.seconds == 59);
}

TEST_CASE("The remaining time is derived from the absolute expiration timestamp",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    const int64_t end = deadlineIn(90);
    timer.startExpirationTime(end);

    CHECK(timer.getRemainingTime() == 90);

    // Re-arming with the same timestamp cannot move the deadline: the countdown
    // target is absolute, so only its scheduling can be wrong.
    timer.startExpirationTime(end);
    CHECK(timer.getRemainingTime() == 90);
}

TEST_CASE("Under one hour left, the countdown updates every second", "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(30 * SECS_IN_1_MIN));

    checkIntervalIs(timer, 1);
}

TEST_CASE("With hours left, the countdown wakes up at the next change of hour",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(5 * SECS_IN_1_HOUR + 30 * SECS_IN_1_MIN));

    // Text says "5 hours", it changes to "4 hours" in 30 minutes.
    checkIntervalIs(timer, 30 * SECS_IN_1_MIN);
}

TEST_CASE("With days left, the countdown wakes up at the next change of day",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(4 * SECS_IN_1_DAY + 6 * SECS_IN_1_HOUR));

    // Text says "4 days", it changes to "3 days" in 6 hours.
    checkIntervalIs(timer, 6 * SECS_IN_1_HOUR);
}

TEST_CASE("Stopping the countdown deactivates the timer", "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(30 * SECS_IN_1_MIN));
    REQUIRE(timer.isActive());

    timer.stopExpirationTime();
    CHECK_FALSE(timer.isActive());

    // ... and arming it again resumes the countdown.
    timer.startExpirationTime(deadlineIn(30 * SECS_IN_1_MIN));
    CHECK(timer.isActive());
}

TEST_CASE("Re-arming the same expiration timestamp keeps the same schedule", "[notification-timer]")
{
    // NotificationItem re-arms the countdown from updateNotificationData(), which
    // runs every time the delegate hands the widget its data again (navigating to
    // the notifications tab, a data reset, a widget rebuilt by the cache). The
    // deadline has not moved, so the schedule must not change either.
    alignToSecondStart();

    NotificationExpirationTimer timer;
    const int64_t end = deadlineIn(4 * SECS_IN_1_DAY + 6 * SECS_IN_1_HOUR);

    timer.startExpirationTime(end);
    const int firstInterval = timer.interval();

    timer.startExpirationTime(end);
    const int secondInterval = timer.interval();

    CAPTURE(firstInterval, secondInterval);
    CHECK(secondInterval == firstInterval);
}

TEST_CASE("After a change of day, the countdown wakes up at the next change of day",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;

    // Arm with 5 days 3 hours left...
    timer.startExpirationTime(deadlineIn(5 * SECS_IN_1_DAY + 3 * SECS_IN_1_HOUR));
    // ... then re-arm just after the change of day, as the running countdown does.
    timer.startExpirationTime(
        deadlineIn(4 * SECS_IN_1_DAY + 23 * SECS_IN_1_HOUR + 59 * SECS_IN_1_MIN + 30));

    // Text says "4 days", it changes to "3 days" in 23 h 59 m 30 s.
    checkIntervalIs(timer, 23 * SECS_IN_1_HOUR + 59 * SECS_IN_1_MIN + 30);
}

TEST_CASE("The countdown stays armed when the remaining time is an exact number of hours",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(2 * SECS_IN_1_HOUR));

    // Text says "1 hour" (rounded down), it changes to "59 minutes" in 1 hour.
    checkIntervalIs(timer, SECS_IN_1_HOUR);
}

TEST_CASE("The countdown stays armed when the remaining time is an exact number of days",
          "[notification-timer]")
{
    alignToSecondStart();

    NotificationExpirationTimer timer;
    timer.startExpirationTime(deadlineIn(3 * SECS_IN_1_DAY));

    // Text says "2 days" (rounded down), it changes to "1 day" in 1 day.
    checkIntervalIs(timer, SECS_IN_1_DAY);
}

TEST_CASE("A countdown longer than 24 days is scheduled with a sane interval",
          "[notification-timer]")
{
    // QTimer takes its interval in milliseconds as an int, which caps it at
    // ~24.8 days: a long offer must never be scheduled beyond that.
    alignToSecondStart();

    NotificationExpirationTimer timer;

    timer.startExpirationTime(deadlineIn(40 * SECS_IN_1_DAY + 5 * SECS_IN_1_HOUR));
    timer.startExpirationTime(
        deadlineIn(30 * SECS_IN_1_DAY + 23 * SECS_IN_1_HOUR + 30 * SECS_IN_1_MIN));

    // Text says "30 days", it changes to "29 days" in 23 h 30 m.
    checkIntervalIs(timer, 23 * SECS_IN_1_HOUR + 30 * SECS_IN_1_MIN);
}
