#ifndef ALERT_DELEGATE_H
#define ALERT_DELEGATE_H

#include <QWidget>
#include <QCache>

class AlertItem;
class MegaUserAlertExt;

class AlertDelegate
{

public:
    AlertDelegate();
    QWidget* getWidget(MegaUserAlertExt* alert, QWidget* parent);

private:
    QCache<unsigned, AlertItem> mItems;

};

#endif // ALERT_DELEGATE_H
