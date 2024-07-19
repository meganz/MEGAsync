#include "UserNotification.h"

#include "ImageDownloader.h"
#include "MegaApplication.h"

namespace
{
const QLatin1String DefaultImageExtension(".png");
constexpr char* KeyCallToActionText = "text";
constexpr char* KeyCallToActionLink = "link";
constexpr int SmallImageSize = 48;
constexpr int LargeImageWidth = 370;
constexpr int LargeImageHeight = 115;
}

UserNotification::UserNotification(const mega::MegaNotification* notification, QObject* parent)
    : UserMessage(UserMessage::Type::NOTIFICATION, parent)
    , mNotification(notification)
    , mDownloader(std::make_unique<ImageDownloader>(nullptr))
    , mSeen(false)
{
    connect(mDownloader.get(), &ImageDownloader::downloadFinished,
            this, &UserNotification::onDownloadFinished);

    mDownloader->downloadImage(getImageNamePath(), LargeImageWidth, LargeImageHeight);
    mDownloader->downloadImage(getIconNamePath(), SmallImageSize, SmallImageSize);
}

void UserNotification::reset(const mega::MegaNotification* notification)
{
    mNotification.reset(notification);
}

bool UserNotification::isSeen() const
{
    return mSeen;
}

void UserNotification::markAsSeen()
{
    mSeen = true;
}

int64_t UserNotification::getID() const
{
    return mNotification->getID();
}

QString UserNotification::getTitle() const
{
    return QString::fromUtf8(mNotification->getTitle());
}

QString UserNotification::getDescription() const
{
    return QString::fromUtf8(mNotification->getDescription());
}

bool UserNotification::showImage() const
{
    return !QString::fromUtf8(mNotification->getImageName()).isEmpty();
}

QString UserNotification::getImageNamePath() const
{
    return QString::fromUtf8(mNotification->getImagePath())
                + QString::fromUtf8(mNotification->getImageName())
                + DefaultImageExtension;
}

bool UserNotification::showIcon() const
{
    return !QString::fromUtf8(mNotification->getIconName()).isEmpty();
}

QPixmap UserNotification::getImagePixmap() const
{
    return mImage;
}

QPixmap UserNotification::getIconPixmap() const
{
    return mIcon;
}

QString UserNotification::getIconNamePath() const
{
    return QString::fromUtf8(mNotification->getImagePath())
                + QString::fromUtf8(mNotification->getIconName())
                + DefaultImageExtension;
}

int64_t UserNotification::getStart() const
{
    return mNotification->getStart();
}

int64_t UserNotification::getEnd() const
{
    return mNotification->getEnd();
}

const char* UserNotification::getActionText() const
{
    return mNotification->getCallToAction1()->get(KeyCallToActionText);
}

const char* UserNotification::getActionLink() const
{
    return mNotification->getCallToAction1()->get(KeyCallToActionLink);
}

bool UserNotification::isRowAccepted(MessageType type) const
{
    return type == MessageType::ALL;
}

void UserNotification::onDownloadFinished(const QImage& image, const QString& imageUrl)
{
    if (image.isNull())
    {
        return;
    }

    if (imageUrl == getImageNamePath())
    {
        QSize size = QSize(LargeImageWidth, LargeImageHeight);
        mImage = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        emit imageChanged();
    }
    else if (imageUrl == getIconNamePath())
    {
        QSize size = QSize(SmallImageSize, SmallImageSize);
        mIcon = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        emit iconChanged();
    }
}
