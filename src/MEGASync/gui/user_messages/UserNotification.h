#ifndef USER_NOTIFICATION_H
#define USER_NOTIFICATION_H

#include "UserMessage.h"
#include "ImageDownloader.h"

#include <QPixmap>
#include <QUrl>

#include <memory>

namespace mega
{
class MegaNotification;
}

class UserNotification : public UserMessage
{
    Q_OBJECT

public:
    UserNotification() = delete;
    UserNotification(const mega::MegaNotification* notification, QObject* parent = nullptr);
    ~UserNotification() = default;

    bool isSeen() const override;
    bool isRowAccepted(MessageType type) const override;
    bool sort(UserMessage* checkWith) const override;

    void reset(const mega::MegaNotification* notification);
    bool equals(const mega::MegaNotification* notification) const;

    void markAsSeen();
    void markAsExpired();

    QString getTitle() const;
    QString getDescription() const;

    bool showImage() const;
    bool showIcon() const;
    QString getImageNamePath() const;
    QString getIconNamePath() const;
    QPixmap getImagePixmap() const;
    QPixmap getIconPixmap() const;

    int64_t getStart() const;
    int64_t getEnd() const;

    // For now, call to action 2 is not used
    const QString getActionText() const;
    const QUrl getActionUrl() const;

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

};

#endif // USER_NOTIFICATION_H
