#ifndef USER_MESSAGE_H
#define USER_MESSAGE_H

#include <QObject>

class UserMessage : public QObject
{
    Q_OBJECT

public:
    enum class Type
    {
        NONE = 0,
        ALERT,
        NOTIFICATION
    };

    UserMessage() = delete;
    UserMessage(Type type, QObject* parent = nullptr)
        : QObject(parent)
        , mType(type)
    {
    }
    ~UserMessage() = default;

    Type getType() const
    {
        return mType;
    }

    virtual bool isSeen() const = 0;

private:
    Type mType;

};

#endif // USER_MESSAGE_H
