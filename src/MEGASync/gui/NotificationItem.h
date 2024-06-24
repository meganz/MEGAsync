#ifndef NOTIFICATIONITEM_H
#define NOTIFICATIONITEM_H

#include <QWidget>

class NotifTest;

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

    void setNotificationData(NotifTest* notification);

private:
    Ui::NotificationItem *ui;
    NotifTest* mNotificationData;

};

#endif // NOTIFICATIONITEM_H
