#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <QDialog>
#include <QTimer>
#include "gui/PlanWidget.h"
#include <QHBoxLayout>
#include <megaapi.h>

namespace Ui {
class UpgradeDialog;
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(mega::MegaPricing *pricing, QWidget *parent = 0);
    void setTimestamp(long long finishTime);
    void refreshAccountDetails();
    void setPricing(mega::MegaPricing *pricing);
    ~UpgradeDialog();

private:
    Ui::UpgradeDialog *ui;
    long long finishTime;
    QTimer *timer;
    QHBoxLayout* plansLayout;

    void updatePlans();
    QString convertCurrency(const char *currency);
    void clearPlans(QLayout *layout);

private slots:
    void unitTimeElapsed();

protected:
    mega::MegaPricing *pricing;
};

#endif // UPGRADEDIALOG_H
