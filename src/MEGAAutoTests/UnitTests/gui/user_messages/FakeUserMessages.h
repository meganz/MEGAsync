#ifndef FAKE_USER_MESSAGES_H
#define FAKE_USER_MESSAGES_H

#include "megaapi.h"
#include "UserNotification.h"

#include <QDateTime>

#include <memory>
#include <utility>
#include <vector>

// Minimal notification, to build a UserNotification without the SDK.
class FakeMegaNotification: public mega::MegaNotification
{
public:
    FakeMegaNotification(int64_t id, int64_t start, int64_t end):
        mId(id),
        mStart(start),
        mEnd(end),
        mCallToAction(mega::MegaStringMap::createInstance())
    {
        mCallToAction->set("text", "Get the offer");
        mCallToAction->set("link", "https://mega.nz/pro");
    }

    int64_t getID() const override
    {
        return mId;
    }

    const char* getTitle() const override
    {
        return "Limited time discount";
    }

    const char* getDescription() const override
    {
        return "Get 30% off any Pro plan";
    }

    // No image and no icon: nothing is downloaded while testing.
    const char* getImageName() const override
    {
        return "";
    }

    const char* getIconName() const override
    {
        return "";
    }

    const char* getImagePath() const override
    {
        return "";
    }

    int64_t getStart() const override
    {
        return mStart;
    }

    int64_t getEnd() const override
    {
        return mEnd;
    }

    bool showBanner() const override
    {
        return false;
    }

    const mega::MegaStringMap* getCallToAction1() const override
    {
        return mCallToAction.get();
    }

    const mega::MegaStringMap* getCallToAction2() const override
    {
        return mCallToAction.get();
    }

    mega::MegaStringList* getRenderModes() const override
    {
        return nullptr;
    }

    mega::MegaStringMap* getRenderModeFields(const char*) const override
    {
        return nullptr;
    }

    MegaNotification* copy() const override
    {
        return new FakeMegaNotification(mId, mStart, mEnd);
    }

private:
    int64_t mId;
    int64_t mStart;
    int64_t mEnd;
    std::unique_ptr<mega::MegaStringMap> mCallToAction;
};

// UserNotification takes ownership of the MegaNotification.
inline std::unique_ptr<UserNotification> createNotificationEndingIn(int64_t secs, int64_t id = 4321)
{
    const int64_t now = QDateTime::currentSecsSinceEpoch();
    return std::make_unique<UserNotification>(
        new FakeMegaNotification(id, now - 24 * 3600, now + secs));
}

// Minimal notification list, to feed UserMessageModel without the SDK. It does not
// own the notifications it is given.
class FakeMegaNotificationList: public mega::MegaNotificationList
{
public:
    explicit FakeMegaNotificationList(std::vector<const mega::MegaNotification*> notifications):
        mNotifications(std::move(notifications))
    {}

    const mega::MegaNotification* get(unsigned i) const override
    {
        return i < size() ? mNotifications[i] : nullptr;
    }

    unsigned size() const override
    {
        return static_cast<unsigned>(mNotifications.size());
    }

    MegaNotificationList* copy() const override
    {
        return new FakeMegaNotificationList(mNotifications);
    }

private:
    std::vector<const mega::MegaNotification*> mNotifications;
};

// A user message of the alert type which is not a UserAlert: AlertItem ignores its
// data, which is all that is needed to have an alert widget built for a row.
class FakeAlertUserMessage: public UserMessage
{
public:
    explicit FakeAlertUserMessage(unsigned id):
        UserMessage(id, UserMessage::Type::ALERT)
    {}

    bool isSeen() const override
    {
        return true;
    }

    bool isRowAccepted(MessageType) const override
    {
        return true;
    }
};

#endif // FAKE_USER_MESSAGES_H
