#ifndef NOTIFICATION_EXT_BASE_H
#define NOTIFICATION_EXT_BASE_H

#include <QObject>

class NotificationExtBase : public QObject
{
    Q_OBJECT

public:
    enum class Type
    {
        NONE = 0,
        ALERT,
        NOTIFICATION
    };

    NotificationExtBase() = delete;
    NotificationExtBase(Type type, QObject* parent = nullptr)
        : QObject(parent)
        , mType(type)
    {
    }
    ~NotificationExtBase() = default;

    Type getType() const
    {
        return mType;
    }

private:
    Type mType;

};

#endif // NOTIFICATION_EXT_BASE_H
