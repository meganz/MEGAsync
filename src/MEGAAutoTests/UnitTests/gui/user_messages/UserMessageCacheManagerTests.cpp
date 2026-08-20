#include "FakeUserMessages.h"
#include "NotificationItem.h"
#include "UserMessageCacheManager.h"
#include "UserMessageWidget.h"
#include <catch.hpp>

#include <QPointer>
#include <QWidget>

/**
 * Tests for the widget recycling behind the InfoDialog notifications tab.
 *
 * UserMessageCacheManager hands UserMessageDelegate the widget that paints a row.
 * Widgets are parented to the view but owned by the caches, hence the declaration
 * order in the fixture: the cache manager has to be destroyed before the parent
 * widget, exactly like InfoDialog destroys the delegate before its view.
 */

namespace
{
class UserMessageCacheManagerTests
{
protected:
    // Declared first, so it is destroyed after mCache.
    QWidget mParent;
    UserMessageCacheManager mCache;
};
}

TEST_CASE_METHOD(UserMessageCacheManagerTests,
                 "A row keeps its widget across lookups",
                 "[user-message-cache]")
{
    auto notification = createNotificationEndingIn(3600);

    // Any row index can be asked for first: it says nothing about how many widgets
    // are already cached.
    UserMessageWidget* widget = mCache.createOrGetWidget(5, notification.get(), &mParent);
    REQUIRE(widget != nullptr);
    CHECK(dynamic_cast<NotificationItem*>(widget) != nullptr);

    CHECK(mCache.createOrGetWidget(5, notification.get(), &mParent) == widget);
}

TEST_CASE_METHOD(UserMessageCacheManagerTests,
                 "A row of another type does not discard the widget of a colliding row",
                 "[user-message-cache]")
{
    auto notification = createNotificationEndingIn(3600);
    FakeAlertUserMessage alert(1);

    // A cache holds 16 widgets, so rows 0 and 16 share an entry.
    QPointer<UserMessageWidget> notificationWidget =
        mCache.createOrGetWidget(0, notification.get(), &mParent);
    REQUIRE(!notificationWidget.isNull());

    UserMessageWidget* alertWidget = mCache.createOrGetWidget(16, &alert, &mParent);
    REQUIRE(alertWidget != nullptr);
    CHECK(alertWidget != notificationWidget);

    // Building the alert widget must not have destroyed the notification widget,
    // which the view may still be using to paint or edit its own row.
    REQUIRE(!notificationWidget.isNull());
    CHECK(mCache.createOrGetWidget(0, notification.get(), &mParent) == notificationWidget);
}

TEST_CASE_METHOD(UserMessageCacheManagerTests,
                 "A recycled widget displays the message it is asked for",
                 "[user-message-cache]")
{
    auto first = createNotificationEndingIn(3600, 1);
    auto second = createNotificationEndingIn(3600, 2);

    UserMessageWidget* widget = mCache.createOrGetWidget(0, first.get(), &mParent);
    REQUIRE(widget != nullptr);
    REQUIRE(widget->getData() == first.get());

    // Rows 0 and 16 share an entry, so the same widget is reused for both, and it
    // has to display the message of the row being painted.
    CHECK(mCache.createOrGetWidget(16, second.get(), &mParent) == widget);
    CHECK(widget->getData() == second.get());
}
