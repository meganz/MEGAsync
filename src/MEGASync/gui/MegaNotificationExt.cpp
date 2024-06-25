#include "MegaNotificationExt.h"

MegaNotificationExt::MegaNotificationExt(const mega::MegaNotification* notification, QObject* parent)
    : QObject(parent)
    , mNotification(notification)
{
}

int64_t MegaNotificationExt::getID() const
{
    return mNotification->getID();
}

QString MegaNotificationExt::getTitle() const
{
    return QString::fromLatin1(mNotification->getTitle());
}

QString MegaNotificationExt::getDescription() const
{
    return QString::fromLatin1(mNotification->getDescription());
}

bool MegaNotificationExt::showImage() const
{
    return !QString::fromLatin1(mNotification->getImageName()).isEmpty();
}

QString MegaNotificationExt::getImageNamePath() const
{
    return QString::fromLatin1(mNotification->getImagePath())
                + QString::fromLatin1(mNotification->getImageName());
}

bool MegaNotificationExt::showIcon() const
{
    return !QString::fromLatin1(mNotification->getIconName()).isEmpty();
}

QString MegaNotificationExt::getIconNamePath() const
{
    return QString::fromLatin1(mNotification->getImagePath())
                + QString::fromLatin1(mNotification->getIconName());
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
