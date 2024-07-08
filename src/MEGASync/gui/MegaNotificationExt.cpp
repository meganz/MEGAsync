#include "MegaNotificationExt.h"

#include "ImageDownloader.h"
#include "MegaApplication.h"

namespace
{
constexpr char* DefaultImageExtension = ".png";
constexpr char* KeyCallToActionText = "text";
constexpr char* KeyCallToActionLink = "link";
constexpr int SmallImageSize = 48;
constexpr int LargeImageWidth = 370;
constexpr int LargeImageHeight = 115;
}

MegaNotificationExt::MegaNotificationExt(const mega::MegaNotification* notification, QObject* parent)
    : QObject(parent)
    , mNotification(notification)
    , mDownloader(std::make_unique<ImageDownloader>(this))
{
    connect(mDownloader.get(), &ImageDownloader::downloadFinished,
            this, &MegaNotificationExt::onDownloadFinished);

    mDownloader->downloadImage(getImageNamePath(), LargeImageWidth, LargeImageHeight);
    mDownloader->downloadImage(getIconNamePath(), SmallImageSize, SmallImageSize);
}

void MegaNotificationExt::reset(const mega::MegaNotification* notification)
{
    mNotification.reset(notification);
}

int64_t MegaNotificationExt::getID() const
{
    return mNotification->getID();
}

QString MegaNotificationExt::getTitle() const
{
    return QString::fromUtf8(mNotification->getTitle());
}

QString MegaNotificationExt::getDescription() const
{
    return QString::fromUtf8(mNotification->getDescription());
}

bool MegaNotificationExt::showImage() const
{
    return !QString::fromUtf8(mNotification->getImageName()).isEmpty();
}

QString MegaNotificationExt::getImageNamePath() const
{
    return QString::fromUtf8(mNotification->getImagePath())
                + QString::fromUtf8(mNotification->getImageName())
                + QString::fromUtf8(DefaultImageExtension);
}

bool MegaNotificationExt::showIcon() const
{
    return !QString::fromUtf8(mNotification->getIconName()).isEmpty();
}

QPixmap MegaNotificationExt::getImagePixmap() const
{
    return mImage;
}

QPixmap MegaNotificationExt::getIconPixmap() const
{
    return mIcon;
}

QString MegaNotificationExt::getIconNamePath() const
{
    return QString::fromUtf8(mNotification->getImagePath())
                + QString::fromUtf8(mNotification->getIconName())
                + QString::fromUtf8(DefaultImageExtension);
}

int64_t MegaNotificationExt::getStart() const
{
    return mNotification->getStart();
}

int64_t MegaNotificationExt::getEnd() const
{
    return mNotification->getEnd();
}

const char* MegaNotificationExt::getActionText() const
{
    return mNotification->getCallToAction1()->get(KeyCallToActionText);
}

const char* MegaNotificationExt::getActionLink() const
{
    return mNotification->getCallToAction1()->get(KeyCallToActionLink);
}

void MegaNotificationExt::onDownloadFinished(const QImage& image, const QString& imageUrl)
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
