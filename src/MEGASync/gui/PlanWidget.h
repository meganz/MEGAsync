#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include <QWidget>
#include <megaapi.h>
#include <QPushButton>

struct PlanInfo
{
    int amount;
    QString currency;
    unsigned long long gbStorage;
    unsigned long long gbTransfer;
    int level;
};

namespace Ui {
class PlanWidget;
}

class PlanWidget : public QWidget
{
    Q_OBJECT

public:
    typedef enum {
           PRO_I    = 1,
           PRO_II   = 2,
           PRO_III  = 3,
           PRO_LITE = 4
    } ProLevel;

    explicit PlanWidget(mega::MegaApi *megaApi, PlanInfo data, QWidget *parent = 0);
    void setPlanInfo(PlanInfo data);
    ~PlanWidget();

private:
    Ui::PlanWidget *ui;
    QPushButton *overlay;
    mega::MegaApi *api;
    PlanInfo details;

    void updatePlanInfo();

private slots:
    void onOverlayClicked();
};

#endif // PLANWIDGET_H
