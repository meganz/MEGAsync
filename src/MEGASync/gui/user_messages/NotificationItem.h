#ifndef NOTIFICATION_ITEM_H
#define NOTIFICATION_ITEM_H

#include "UserMessageWidget.h"
#include "Utilities.h"

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

private slots:
    void onCTAClicked();
    void onTimerExpirated(int64_t remainingTimeSecs);

private:
    Ui::NotificationItem* mUi;
    UserNotification* mNotificationData;
    IntervalTimer mExpirationTimer;

    void setNotificationData(UserNotification* newNotificationData);
    void setImages();
    void updateExpirationText();

};

#endif // NOTIFICATION_ITEM_H
