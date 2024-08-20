#include "UserNotification.h"

#include "ImageDownloader.h"
#include "MegaApplication.h"

#include "megaapi.h"

#include <cstring>

namespace
{
const QLatin1String DEFAULT_IMAGE_EXTENSION(".png");
constexpr char* KEY_CALL_TO_ACTION_TEXT = "text";
constexpr char* KEY_CALL_TO_ACTION_LINK = "link";
constexpr int SMALL_IMAGE_SIZE = 48;
constexpr int LARGE_IMAGE_WIDTH = 370;
constexpr int LARGE_IMAGE_HEIGHT = 115;
}

UserNotification::UserNotification(const mega::MegaNotification* notification, QObject* parent)
    : UserMessage(static_cast<unsigned>(notification->getID()), UserMessage::Type::NOTIFICATION, parent)
    , mNotification(notification)
    , mDownloader(std::make_unique<ImageDownloader>(nullptr))
    , mSeen(false)
{
    connect(mDownloader.get(), &ImageDownloader::downloadFinished,
            this, &UserNotification::onDownloadFinished);

    mDownloader->downloadImage(getImageNamePath(), LARGE_IMAGE_WIDTH, LARGE_IMAGE_HEIGHT);
    mDownloader->downloadImage(getIconNamePath(), SMALL_IMAGE_SIZE, SMALL_IMAGE_SIZE);
}

void UserNotification::reset(const mega::MegaNotification* notification)
{
    QString oldImageNamePath = getImageNamePath();
    QString oldIconNamePath = getIconNamePath();

    mNotification.reset(notification);

    if(oldImageNamePath != getImageNamePath())
    {
        mDownloader->downloadImage(getImageNamePath(), LARGE_IMAGE_WIDTH, LARGE_IMAGE_HEIGHT);
    }

    if(oldIconNamePath != getIconNamePath())
    {
        mDownloader->downloadImage(getIconNamePath(), SMALL_IMAGE_SIZE, SMALL_IMAGE_SIZE);
    }

    emit dataChanged();
}

bool UserNotification::equals(const mega::MegaNotification* notification) const
{
    if (strcmp(mNotification->getTitle(), notification->getTitle()) != 0
            || strcmp(mNotification->getDescription(), notification->getDescription()) != 0
            || strcmp(mNotification->getImagePath(), notification->getImagePath()) != 0
            || strcmp(mNotification->getImageName(), notification->getImageName()) != 0
            || strcmp(mNotification->getIconName(), notification->getIconName()) != 0
            || mNotification->getStart() != notification->getStart()
            || mNotification->getEnd() != notification->getEnd()
            || strcmp(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_TEXT),
                      notification->getCallToAction1()->get(KEY_CALL_TO_ACTION_TEXT)) != 0
            || strcmp(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_LINK),
                      notification->getCallToAction1()->get(KEY_CALL_TO_ACTION_LINK)) != 0)
    {
        return false;
    }

    return true;
}

bool UserNotification::isSeen() const
{
    return mSeen;
}

void UserNotification::markAsSeen()
{
    mSeen = true;
}

void UserNotification::markAsExpired()
{
    emit expired(mId);
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
                + DEFAULT_IMAGE_EXTENSION;
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
                + DEFAULT_IMAGE_EXTENSION;
}

int64_t UserNotification::getStart() const
{
    return mNotification->getStart();
}

int64_t UserNotification::getEnd() const
{
    return mNotification->getEnd();
}

const QString UserNotification::getActionText() const
{
    return QString::fromUtf8(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_TEXT));
}

const QUrl UserNotification::getActionUrl() const
{
    return QUrl(QString::fromUtf8(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_LINK)));
}

bool UserNotification::isRowAccepted(MessageType type) const
{
    return type == MessageType::ALL;
}

bool UserNotification::sort(UserMessage* checkWith) const
{
    if(auto checkNotification = dynamic_cast<UserNotification*>(checkWith))
    {
        auto thisID = mNotification->getID();
        auto checkID = checkNotification->mNotification->getID();
        return thisID > checkID;
    }

    return UserMessage::sort(checkWith);
}

void UserNotification::onDownloadFinished(const QImage& image, const QString& imageUrl)
{
    if (image.isNull())
    {
        return;
    }

    if (imageUrl == getImageNamePath())
    {
        QSize size = QSize(LARGE_IMAGE_WIDTH, LARGE_IMAGE_HEIGHT);
        mImage = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        emit imageChanged();
    }
    else if (imageUrl == getIconNamePath())
    {
        QSize size = QSize(SMALL_IMAGE_SIZE, SMALL_IMAGE_SIZE);
        mIcon = QPixmap::fromImage(image).scaled(size, Qt::KeepAspectRatio, Qt::SmoothTransformation);
        emit iconChanged();
    }
}
