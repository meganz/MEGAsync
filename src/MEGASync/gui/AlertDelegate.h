#ifndef ALERT_DELEGATE_H
#define ALERT_DELEGATE_H

#include <QWidget>
#include <QCache>

class AlertItem;
class UserAlert;

class AlertDelegate
{

public:
    AlertDelegate();
    QWidget* getWidget(UserAlert* alert, QWidget* parent);

private:
    QCache<unsigned, AlertItem> mItems;

};

#endif // ALERT_DELEGATE_H
