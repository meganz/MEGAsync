#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include <QObject>
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

    virtual bool isSeen() const = 0;
    virtual bool sort(UserMessage*) const {return false;}
    virtual bool isRowAccepted(MessageType type) const = 0;

private:
    Type mType;

};

#endif // USER_MESSAGE_H
