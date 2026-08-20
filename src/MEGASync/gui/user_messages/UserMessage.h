#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include "UserMessageTypes.h"

#include <QObject>
#include <QSize>

#include <cstdint>

class UserMessage : public QObject
{
    Q_OBJECT

public:
    //It follows the required order
    enum class Type
    {
        NONE = 0,
        NOTIFICATION,
        ALERT,
    };

    UserMessage() = delete;
    UserMessage(unsigned id, Type type, QObject* parent = nullptr)
        : QObject(parent)
        , mId(id)
        , mType(type)
    {
    }

    virtual ~UserMessage() = default;

    inline virtual bool isSeen() const = 0;
    inline virtual bool isRowAccepted(MessageType type) const = 0;

    inline virtual bool sort(UserMessage*) const { return false; }

    inline Type getType() const { return mType; }
    inline bool isOfType(Type type) const { return mType == type; }
    inline QSize sizeHint() const { return mSizeHint; }
    inline void setSizeHint(const QSize& newSizeHint) { mSizeHint = newSizeHint; }
    inline unsigned id() const { return mId; }
    inline bool hasSameId(unsigned id) const { return mId == id; }

    //In case we know the height of an user message can dynamically change
    inline void clearSizeHint() { mSizeHint = QSize(); }

    inline void updated()
    {
        emit uiUpdated();
    }

signals:
    // endWhenExpired is the deadline the sender based its expiration decision on, so a
    // receiver can tell a stale event (superseded by a refresh with a later deadline)
    // from a genuine one by comparing it against the message's current deadline.
    void expired(unsigned id, int64_t endWhenExpired);
    void dataReset();
    void uiUpdated();

protected:
    unsigned mId;

private:
    Type mType;
    QSize mSizeHint;

};

#endif // USER_MESSAGE_H
