#ifndef USER_NOTIFICATION_H
#define USER_NOTIFICATION_H

#include "UserMessage.h"

#include <QPixmap>

#include <memory>

namespace mega
{
class MegaNotification;
}

class ImageDownloader;

class UserNotification : public UserMessage
{
    Q_OBJECT

public:
    UserNotification() = delete;
    UserNotification(const mega::MegaNotification* notification, QObject* parent = nullptr);
    ~UserNotification() = default;

    void reset(const mega::MegaNotification* notification);

    bool isSeen() const override;
    void markAsSeen();

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

    bool isRowAccepted(MessageType type) const override;

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
    bool mSeen;

    QString getImageNamePath() const;
    QString getIconNamePath() const;

};

#endif // USER_NOTIFICATION_H
