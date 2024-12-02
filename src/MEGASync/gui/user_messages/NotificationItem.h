#ifndef NOTIFICATION_ITEM_H
#define NOTIFICATION_ITEM_H

#include "NotificationExpirationTimer.h"
#include "UserMessageWidget.h"

#include <QPointer>

class UserNotification;

namespace Ui
{
class NotificationItem;
}

class NotificationItem : public UserMessageWidget
{
    Q_OBJECT

public:
    explicit NotificationItem(QWidget* parent = nullptr);
    ~NotificationItem();

    void setData(UserMessage* data) override;
    UserMessage* getData() const override;

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

protected:
    void changeEvent(QEvent* event) override;
    void showEvent(QShowEvent* event) override;
    void mousePressEvent(QMouseEvent* event) override;

private slots:
    void onCTAClicked();
    void onTimerExpirated(int64_t remainingTimeSecs);

    void onUserNotificationDestroyed(QObject* UserNotification);

private:
    Ui::NotificationItem* mUi;
    QPointer<UserNotification> mNotificationData;
    NotificationExpirationTimer mExpirationTimer;
    bool mDisplayEventSent;
    QMetaObject::Connection mTimerConnection;

    void setNotificationData(UserNotification* newNotificationData);
    void updateNotificationData(UserNotification* newNotificationData);
    void updateNotificationData(bool downloadImage = false,
                                bool downloadIcon = false);
    void setImage();
    void setIcon();
    void updateExpirationText();
    bool updateExpiredTimeAndClicks(int64_t remainingTimeSecs);

};

#endif // NOTIFICATION_ITEM_H
