#ifndef NOTIFICATIONITEM_H
#define NOTIFICATIONITEM_H

#include "ImageDownloader.h"

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

private slots:
    void updateImage(const QImage& image, const QString& imageUrl);

private:
    Ui::NotificationItem* ui;
    MegaNotificationExt* mNotificationData;
    std::unique_ptr<ImageDownloader> mDownloader;

};

#endif // NOTIFICATIONITEM_H
