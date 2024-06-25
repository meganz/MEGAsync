#ifndef NOTIFICATIONITEM_H
#define NOTIFICATIONITEM_H

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

};

#endif // NOTIFICATIONITEM_H
