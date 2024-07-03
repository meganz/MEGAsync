#include "MegaNotificationExt.h"

#include "MegaApplication.h"

namespace
{
constexpr char* DefaultImageExtension = ".png";
}

MegaNotificationExt::MegaNotificationExt(const mega::MegaNotification* notification, QObject* parent)
    : QObject(parent)
    , mNotification(notification)
{
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
    const char* value = mNotification->getCallToAction1()->get("text");
    if (value != NULL)
    {
        return value;
    }
    else
    {
        return "";
    }
}

const char* MegaNotificationExt::getActionLink() const
{
    const char* value = mNotification->getCallToAction1()->get("link");
    if (value != NULL)
    {
        return value;
    }
    else
    {
        return "";
    }
}
