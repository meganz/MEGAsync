#include "ApiImageLabel.h"
#include "FakeUserMessages.h"
#include "NotificationItem.h"
#include <catch.hpp>

#include <QDateTime>
#include <QEventLoop>
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QTimer>

/**
 * Tests for the widget that renders a notification with an end timestamp
 * (i.e. a discount offer) in the InfoDialog notifications tab.
 */

namespace
{
// The countdown reads the wall clock on its own, so an end timestamp computed
// here as "now + X" is off by one second if the current second changes in
// between. Waiting for the beginning of a second leaves ~800 ms of margin.
void alignToSecondStart()
{
    while (QDateTime::currentMSecsSinceEpoch() % 1000 > 200)
    {
        QThread::msleep(10);
    }
}

class NotificationItemTests
{
protected:
    NotificationItemTests()
    {
        mTimeLabel = mItem.findChild<QLabel*>(QLatin1String("lTime"));
        mCallToAction = mItem.findChild<QPushButton*>(QLatin1String("bCTA"));

        alignToSecondStart();
    }

    QString timeText() const
    {
        REQUIRE(mTimeLabel != nullptr);
        return mTimeLabel->text();
    }

    bool isCallToActionEnabled() const
    {
        REQUIRE(mCallToAction != nullptr);
        return mCallToAction->isEnabled();
    }

    void waitFor(int msecs)
    {
        QEventLoop loop;
        QTimer::singleShot(msecs, &loop, &QEventLoop::quit);
        loop.exec();
    }

    NotificationItem mItem;

private:
    QLabel* mTimeLabel = nullptr;
    QPushButton* mCallToAction = nullptr;
};
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer with minutes left shows its remaining time",
                 "[notification-item]")
{
    auto notification = createNotificationEndingIn(91);
    mItem.setData(notification.get());

    CHECK(timeText() == QLatin1String("Offer expires in 1 m 30 s"));
    CHECK(isCallToActionEnabled());
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer expiring on this very second shows it is expired",
                 "[notification-item]")
{
    auto notification = createNotificationEndingIn(1);
    mItem.setData(notification.get());

    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK_FALSE(isCallToActionEnabled());
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer whose end has just been reached shows it is expired",
                 "[notification-item]")
{
    // One second later than the case above: the expired state must not depend on
    // catching the exact expiration second, as a tick can land late (a coarse
    // timer, a busy machine, a resume from sleep).
    auto notification = createNotificationEndingIn(0);
    mItem.setData(notification.get());

    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK_FALSE(isCallToActionEnabled());
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer that ended a while ago shows it is expired and is dropped",
                 "[notification-item]")
{
    // The removal has to be requested exactly once: UserMessageModel::onExpired()
    // deletes the notification, so asking twice means deleting an already deleted
    // notification if anything ever keeps it alive between both requests.
    auto notification = createNotificationEndingIn(-30);

    // UserMessageModel removes the notification when it reports itself expired.
    int expiredCount = 0;
    QObject::connect(notification.get(),
                     &UserMessage::expired,
                     &mItem,
                     [&expiredCount](unsigned)
                     {
                         ++expiredCount;
                     });

    mItem.setData(notification.get());

    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK_FALSE(isCallToActionEnabled());
    CHECK(expiredCount == 1);
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer reaching its end stays displayed as expired before being dropped",
                 "[notification-item]")
{
    // NUM_SECS_TO_WAIT_BEFORE_REMOVE keeps the expired offer in the list, so the
    // user reads "Offer expired" instead of watching a row vanish.
    auto notification = createNotificationEndingIn(2);

    int expiredCount = 0;
    QObject::connect(notification.get(),
                     &UserMessage::expired,
                     &mItem,
                     [&expiredCount](unsigned)
                     {
                         ++expiredCount;
                     });

    mItem.setData(notification.get());
    REQUIRE(timeText() == QLatin1String("Offer expires in 1 second"));

    // One second later the offer is displayed as expired, and still listed.
    waitFor(1500);
    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK_FALSE(isCallToActionEnabled());
    CHECK(expiredCount == 0);

    // It is only dropped once it has been displayed as expired for a while.
    waitFor(2000);
    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK(expiredCount == 1);
}

TEST_CASE_METHOD(NotificationItemTests,
                 "A running countdown asks the model to refresh the row",
                 "[notification-item]")
{
    // The delegate renders this widget off screen into the view, so repainting a
    // label is not enough to update the notifications tab, the row has to be
    // invalidated. UserMessage::updated() is the hook for that, it ends up in
    // UserMessageModel::onUiUpdated() which emits dataChanged() for the row.
    // Without it the countdown would only move under the mouse, as a hovered row
    // is turned into an editor and an editor widget does paint itself.
    auto notification = createNotificationEndingIn(90);

    int uiUpdatedCount = 0;
    QObject::connect(notification.get(),
                     &UserMessage::uiUpdated,
                     &mItem,
                     [&uiUpdatedCount]()
                     {
                         ++uiUpdatedCount;
                     });

    mItem.setData(notification.get());
    const QString textOnArming = timeText();

    // The countdown updates every second below one hour, so two seconds are
    // enough to see the displayed text change.
    waitFor(2200);

    CHECK(timeText() != textOnArming);
    CHECK(uiUpdatedCount >= 1);
}

TEST_CASE_METHOD(NotificationItemTests,
                 "A downloaded image asks the model to refresh the row",
                 "[notification-item]")
{
    // Images are downloaded, so they are not there yet when the row is first
    // painted and the row has to be invalidated once they arrive.
    auto notification = createNotificationEndingIn(90);

    int uiUpdatedCount = 0;
    QObject::connect(notification.get(),
                     &UserMessage::uiUpdated,
                     &mItem,
                     [&uiUpdatedCount]()
                     {
                         ++uiUpdatedCount;
                     });

    mItem.setData(notification.get());

    // Displaying the data must not ask for a refresh: setData() runs while the
    // delegate is painting the row, so the model must not be touched from there.
    REQUIRE(uiUpdatedCount == 0);

    auto* image = mItem.findChild<ApiImageLabel*>(QLatin1String("lImageLarge"));
    REQUIRE(image != nullptr);
    emit image->imageReady(true);

    CHECK(uiUpdatedCount == 1);
}
