#ifndef UPGRADEWIDGET_H
#define UPGRADEWIDGET_H

#include <QWidget>
#include <megaapi.h>
#include <QPushButton>
#include "Utilities.h"

namespace Ui {
class UpgradeWidget;
}

class UpgradeWidget : public QWidget
{
    Q_OBJECT

public:
    typedef enum {
           PRO_I    = 1,
           PRO_II   = 2,
           PRO_III  = 3,
           PRO_LITE = 4
    } ProLevel;

    explicit UpgradeWidget(PlanInfo data, QString userAgent, QWidget *parent = 0);
    void setPlanInfo(PlanInfo data);
    ~UpgradeWidget();

private:
    Ui::UpgradeWidget *ui;
    QPushButton *overlay;
    PlanInfo details;
    QString userAgent;

    void updatePlanInfo();

protected:
    void changeEvent(QEvent* event);

private slots:
    void onOverlayClicked();
};

#endif // UPGRADEWIDGET_H
