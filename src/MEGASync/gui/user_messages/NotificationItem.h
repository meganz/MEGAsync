#ifndef NOTIFICATION_ITEM_H
#define NOTIFICATION_ITEM_H

#include <QWidget>

class UserNotification;

namespace Ui
{
class NotificationItem;
}

class NotificationItem : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationItem(UserNotification* notification, QWidget* parent = nullptr);
    ~NotificationItem();

    QSize minimumSizeHint() const override;
    QSize sizeHint() const override;

private:
    Ui::NotificationItem* mUi;
    UserNotification* mNotificationData;

    void init();
    void setImages();

};

#endif // NOTIFICATION_ITEM_H
