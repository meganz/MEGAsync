#ifndef NOTIFICATION_ITEM_H
#define NOTIFICATION_ITEM_H

#include <QWidget>

class MegaNotificationExt;

namespace Ui
{
class NotificationItem;
}

class NotificationItem : public QWidget
{
    Q_OBJECT

public:
    explicit NotificationItem(QWidget *parent = nullptr);
    ~NotificationItem();

    void setNotificationData(MegaNotificationExt* notification);

private:
    Ui::NotificationItem* ui;
    MegaNotificationExt* mNotificationData;

    void setImages();
};

#endif // NOTIFICATION_ITEM_H
