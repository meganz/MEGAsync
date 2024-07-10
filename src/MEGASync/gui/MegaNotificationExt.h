#ifndef MEGA_NOTIFICATION_EXT_H
#define MEGA_NOTIFICATION_EXT_H

#include "NotificationExtBase.h"
#include "ImageDownloader.h"

#include <QPixmap>

#include <memory>

namespace mega
{
class MegaNotification;
}

class MegaNotificationExt : public NotificationExtBase
{
    Q_OBJECT

public:
    MegaNotificationExt() = delete;
    MegaNotificationExt(const mega::MegaNotification* notification, QObject* parent = nullptr);
    ~MegaNotificationExt() = default;

    void reset(const mega::MegaNotification* notification);

    int64_t getID() const;
    QString getTitle() const;
    QString getDescription() const;

    bool showImage() const;
    bool showIcon() const;
    QPixmap getImagePixmap() const;
    QPixmap getIconPixmap() const;

    int64_t getStart() const;
    int64_t getEnd() const;
    const char* getActionText() const;
    const char* getActionLink() const;

signals:
    void imageChanged();
    void iconChanged();

private slots:
    void onDownloadFinished(const QImage& image, const QString& imageUrl);

private:
    std::unique_ptr<const mega::MegaNotification> mNotification;
    std::unique_ptr<ImageDownloader> mDownloader;
    QPixmap mImage;
    QPixmap mIcon;

    QString getImageNamePath() const;
    QString getIconNamePath() const;

};

#endif // MEGA_NOTIFICATION_EXT_H
