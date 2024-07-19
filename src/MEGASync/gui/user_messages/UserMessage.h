#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include "UserMessageTypes.h"

#include <QObject>
#include <QSize>
#include <QVariant>

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
    UserMessage(QVariant id, Type type, QObject* parent = nullptr)
        : QObject(parent)
        , mType(type)
    {
    }

    ~UserMessage() = default;

    inline virtual bool isSeen() const = 0;
    inline virtual bool isRowAccepted(MessageType type) const = 0;

    inline virtual bool sort(UserMessage*) const { return false; }

    inline Type getType() const { return mType; }
    inline bool isOfType(Type type) const { return mType == type; }
    inline QSize sizeHint() const { return mSizeHint; }
    inline void setSizeHint(const QSize& newSizeHint) { mSizeHint = newSizeHint; }
    inline QVariant id() const { return mId; }
    inline bool hasSameId(QVariant id) const { return mId == id; }

    //In case we know the height of an user message can dynamically change
    inline void clearSizeHint(){ mSizeHint = QSize();}

protected:
    QVariant mId;

private:
    Type mType;
    QSize mSizeHint;

};

#endif // USER_MESSAGE_H
