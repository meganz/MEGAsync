#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include <QObject>
#include <QSize>

#include "UserMessageTypes.h"

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
    UserMessage(Type type, QObject* parent = nullptr)
        : QObject(parent)
        , mType(type)
    {
    }

    ~UserMessage() = default;

    inline Type getType() const
    {
        return mType;
    }

    inline virtual bool isSeen() const = 0;
    inline virtual bool sort(UserMessage*) const {return false;}
    inline virtual bool isRowAccepted(MessageType type) const = 0;

    inline QSize sizeHint() const {return mSizeHint;}
    inline void setSizeHint(const QSize& newSizeHint) { mSizeHint = newSizeHint; }
    //In case we know the height of an user message can dynamically change
    inline void clearSizeHint(){ mSizeHint = QSize();}

private:
    Type mType;
    QSize mSizeHint;
};

#endif // USER_MESSAGE_H
