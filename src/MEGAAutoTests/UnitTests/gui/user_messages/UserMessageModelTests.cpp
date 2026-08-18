#include "FakeUserMessages.h"
#include "UserMessageModel.h"
#include <catch.hpp>

#include <QCoreApplication>
#include <QEvent>
#include <QPointer>

/**
 * Tests for the model behind the InfoDialog notifications tab.
 */

namespace
{
constexpr int64_t NOTIFICATION_ID = 77;

class UserMessageModelTests
{
protected:
    // Puts a live notification in the model and returns it.
    UserNotification* givenANotification()
    {
        const int64_t now = QDateTime::currentSecsSinceEpoch();
        mNotification =
            std::make_unique<FakeMegaNotification>(NOTIFICATION_ID, now - 3600, now + 3600);
        FakeMegaNotificationList list({mNotification.get()});

        mModel.processNotifications(&list);
        REQUIRE(mModel.rowCount() == 1);

        auto* message = static_cast<UserMessage*>(mModel.index(0, 0).internalPointer());
        REQUIRE(message != nullptr);

        auto* notification = qobject_cast<UserNotification*>(message);
        REQUIRE(notification != nullptr);
        return notification;
    }

    // The model is told about an expiration through a queued connection, so the
    // removal only happens once the posted call has been delivered.
    void deliverQueuedCalls()
    {
        QCoreApplication::sendPostedEvents(&mModel);
    }

    // Deferred deletes are only sent when they are explicitly asked for.
    void deliverDeferredDeletes()
    {
        QCoreApplication::sendPostedEvents(nullptr, QEvent::DeferredDelete);
    }

    UserMessageModel mModel;

private:
    // The model copies it, so it has to stay alive while it is being processed.
    std::unique_ptr<FakeMegaNotification> mNotification;
};
}

TEST_CASE_METHOD(UserMessageModelTests,
                 "An expired notification is removed from the model",
                 "[user-message-model]")
{
    auto* notification = givenANotification();

    notification->markAsExpired();

    // The widget reports the expiration while the view may be painting or measuring
    // its row, so the row cannot be removed from under it right away.
    CHECK(mModel.rowCount() == 1);

    deliverQueuedCalls();
    CHECK(mModel.rowCount() == 0);
}

TEST_CASE_METHOD(UserMessageModelTests,
                 "An expired notification is destroyed after it has reported its expiration",
                 "[user-message-model]")
{
    QPointer<UserNotification> notification(givenANotification());

    notification->markAsExpired();
    deliverQueuedCalls();
    REQUIRE(mModel.rowCount() == 0);

    // Removing the row must not destroy the notification, as the widget that
    // reported the expiration still displays it.
    REQUIRE(!notification.isNull());

    deliverDeferredDeletes();
    CHECK(notification.isNull());
}

TEST_CASE_METHOD(UserMessageModelTests,
                 "Refreshing a notification invalidates its row",
                 "[user-message-model]")
{
    auto* notification = givenANotification();

    int refreshedRow = -1;
    QObject::connect(&mModel,
                     &QAbstractItemModel::dataChanged,
                     &mModel,
                     [&refreshedRow](const QModelIndex& topLeft, const QModelIndex&)
                     {
                         refreshedRow = topLeft.row();
                     });

    notification->updated();

    CHECK(refreshedRow == 0);
}

TEST_CASE_METHOD(UserMessageModelTests,
                 "Refreshing a notification already removed from the model does nothing",
                 "[user-message-model]")
{
    // A widget outlives the notification it displays, so it can ask for a refresh
    // once the notification has been removed. That must not be reported as a change
    // of an invalid row.
    QPointer<UserNotification> notification(givenANotification());
    notification->markAsExpired();
    deliverQueuedCalls();
    REQUIRE(mModel.rowCount() == 0);
    REQUIRE(!notification.isNull());

    bool rowRefreshed = false;
    QObject::connect(&mModel,
                     &QAbstractItemModel::dataChanged,
                     &mModel,
                     [&rowRefreshed](const QModelIndex&, const QModelIndex&)
                     {
                         rowRefreshed = true;
                     });

    notification->updated();

    CHECK_FALSE(rowRefreshed);
}
