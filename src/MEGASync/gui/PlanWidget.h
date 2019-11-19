#ifndef PLANWIDGET_H
#define PLANWIDGET_H

#include <QWidget>
#include <megaapi.h>
#include <QPushButton>
#include "Utilities.h"

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
           PRO_LITE = 4,
           BUSINESS = 100,
    } ProLevel;

    enum {
        STORAGE = 1,
        BANDWIDTH = 2,
    };

    explicit PlanWidget(PlanInfo data, QString userAgent, QWidget *parent = 0);
    void setPlanInfo(PlanInfo data);
    ~PlanWidget();

private:
    Ui::PlanWidget *ui;
    QPushButton *overlay;
    PlanInfo details;
    QString userAgent;

    void updatePlanInfo();
    QString formatRichString(QString str, int type);

protected:
    void changeEvent(QEvent* event);

private slots:
    void onOverlayClicked();
};

#endif // PLANWIDGET_H
