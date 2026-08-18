#include "megaapi.h"
#include "NotificationItem.h"
#include "UserNotification.h"
#include <catch.hpp>

#include <QDateTime>
#include <QEventLoop>
#include <QLabel>
#include <QPushButton>
#include <QThread>
#include <QTimer>

#include <memory>

/**
 * Tests for the widget that renders a notification with an end timestamp
 * (i.e. a discount offer) in the InfoDialog notifications tab.
 *
 * Test cases tagged [!shouldfail] describe the expected behaviour of a bug that
 * is not fixed yet: Catch2 reports them as passing while they fail, and as
 * failing once the behaviour is correct (i.e. remove the tag with the fix).
 */

namespace
{
constexpr const char* KEY_CALL_TO_ACTION_TEXT = "text";
constexpr const char* KEY_CALL_TO_ACTION_LINK = "link";
constexpr int64_t NOTIFICATION_ID = 4321;

class FakeMegaNotification: public mega::MegaNotification
{
public:
    FakeMegaNotification(int64_t start, int64_t end):
        mStart(start),
        mEnd(end),
        mCallToAction(mega::MegaStringMap::createInstance())
    {
        mCallToAction->set(KEY_CALL_TO_ACTION_TEXT, "Get the offer");
        mCallToAction->set(KEY_CALL_TO_ACTION_LINK, "https://mega.nz/pro");
    }

    int64_t getID() const override
    {
        return NOTIFICATION_ID;
    }

    const char* getTitle() const override
    {
        return "Limited time discount";
    }

    const char* getDescription() const override
    {
        return "Get 30% off any Pro plan";
    }

    // No image and no icon: nothing is downloaded while testing.
    const char* getImageName() const override
    {
        return "";
    }

    const char* getIconName() const override
    {
        return "";
    }

    const char* getImagePath() const override
    {
        return "";
    }

    int64_t getStart() const override
    {
        return mStart;
    }

    int64_t getEnd() const override
    {
        return mEnd;
    }

    bool showBanner() const override
    {
        return false;
    }

    const mega::MegaStringMap* getCallToAction1() const override
    {
        return mCallToAction.get();
    }

    const mega::MegaStringMap* getCallToAction2() const override
    {
        return mCallToAction.get();
    }

    mega::MegaStringList* getRenderModes() const override
    {
        return nullptr;
    }

    mega::MegaStringMap* getRenderModeFields(const char*) const override
    {
        return nullptr;
    }

    MegaNotification* copy() const override
    {
        return new FakeMegaNotification(mStart, mEnd);
    }

private:
    int64_t mStart;
    int64_t mEnd;
    std::unique_ptr<mega::MegaStringMap> mCallToAction;
};

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

// UserNotification takes ownership of the MegaNotification.
std::unique_ptr<UserNotification> createNotificationEndingIn(int64_t secs)
{
    const int64_t now = QDateTime::currentSecsSinceEpoch();
    return std::make_unique<UserNotification>(
        new FakeMegaNotification(now - 24 * 3600, now + secs));
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
                 "[notification-item][!shouldfail]")
{
    // One second later than the case above: the expired state is only applied on
    // an exact match, so a tick that lands a second late (a coarse timer, a busy
    // or suspended machine) leaves the notification stuck as if it were live.
    auto notification = createNotificationEndingIn(0);
    mItem.setData(notification.get());

    CHECK(timeText() == QLatin1String("Offer expired"));
    CHECK_FALSE(isCallToActionEnabled());
}

TEST_CASE_METHOD(NotificationItemTests,
                 "An offer that ended a while ago shows it is expired and is dropped",
                 "[notification-item][!shouldfail]")
{
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
                 "A running countdown asks the model to refresh the row",
                 "[notification-item][!shouldfail]")
{
    // The delegate renders this widget off screen into the view, so repainting a
    // label is not enough to update the notifications tab: the row has to be
    // invalidated. UserMessage::updated() is the hook for that, it ends up in
    // UserMessageModel::onUiUpdated() which emits dataChanged() for the row.
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
