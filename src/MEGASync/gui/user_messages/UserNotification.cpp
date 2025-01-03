#include "UserNotification.h"

#include "megaapi.h"

#include <cstring>

namespace
{
const QLatin1String DEFAULT_IMAGE_EXTENSION(".png");
constexpr const char* KEY_CALL_TO_ACTION_TEXT = "text";
constexpr const char* KEY_CALL_TO_ACTION_LINK = "link";
}

UserNotification::UserNotification(const mega::MegaNotification* notification, QObject* parent):
    UserMessage(static_cast<unsigned>(notification->getID()),
                UserMessage::Type::NOTIFICATION,
                parent),
    mNotification(notification),
    mSeen(false)
{}

void UserNotification::reset(const mega::MegaNotification* notification)
{
    mNotification.reset(notification);

    emit dataReset();
}

bool UserNotification::equals(const mega::MegaNotification* notification) const
{
    if (strcmp(mNotification->getTitle(), notification->getTitle()) != 0 ||
        strcmp(mNotification->getDescription(), notification->getDescription()) != 0 ||
        strcmp(mNotification->getImagePath(), notification->getImagePath()) != 0 ||
        strcmp(mNotification->getImageName(), notification->getImageName()) != 0 ||
        strcmp(mNotification->getIconName(), notification->getIconName()) != 0 ||
        mNotification->getStart() != notification->getStart() ||
        mNotification->getEnd() != notification->getEnd() ||
        strcmp(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_TEXT),
               notification->getCallToAction1()->get(KEY_CALL_TO_ACTION_TEXT)) != 0 ||
        strcmp(mNotification->getCallToAction1()->get(KEY_CALL_TO_ACTION_LINK),
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
    return QString::fromUtf8(mNotification->getImagePath()) +
           QString::fromUtf8(mNotification->getImageName()) + DEFAULT_IMAGE_EXTENSION;
}

bool UserNotification::showIcon() const
{
    return !QString::fromUtf8(mNotification->getIconName()).isEmpty();
}

QString UserNotification::getIconNamePath() const
{
    return QString::fromUtf8(mNotification->getImagePath()) +
           QString::fromUtf8(mNotification->getIconName()) + DEFAULT_IMAGE_EXTENSION;
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
    if (auto checkNotification = dynamic_cast<UserNotification*>(checkWith))
    {
        auto thisID = mNotification->getID();
        auto checkID = checkNotification->mNotification->getID();
        return thisID > checkID;
    }

    return UserMessage::sort(checkWith);
}
