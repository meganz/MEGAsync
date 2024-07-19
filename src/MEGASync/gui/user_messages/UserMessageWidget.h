#ifndef USER_MESSAGE_WIDGET_H
#define USER_MESSAGE_WIDGET_H

#include "UserMessage.h"

#include <QWidget>

class UserMessageWidget : public QWidget
{
    Q_OBJECT

public:
    explicit UserMessageWidget(QWidget* parent = nullptr)
        : QWidget(parent)
    {
    }
    virtual ~UserMessageWidget() = default;

    virtual void setData(UserMessage* data) = 0;
    virtual UserMessage* getData() const = 0;

signals:
    void needsUpdate();

};

#endif // USER_MESSAGE_WIDGET_H
