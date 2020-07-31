#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include <QDialog>
#include <QTimer>
#include "gui/PlanWidget.h"
#include <QHBoxLayout>
#include <megaapi.h>
#include "HighDpiResize.h"
#ifdef __APPLE__
    #include "macx/DynamicTransferQuotaPopOver.h"
    #import <objc/runtime.h>
#else
    #include "DynamicTransferQuotaPopOver.h"
#endif

#include <memory>

namespace Ui {
class UpgradeDialog;
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(mega::MegaApi *megaApi, mega::MegaPricing *pricing, QWidget *parent = 0);
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
    void clearPlans();
    void mousePressEvent(QMouseEvent *event) override;

#ifdef __APPLE__
    std::unique_ptr<DynamicTransferQuotaPopOver> mPopOver{new DynamicTransferQuotaPopOver()};
    id m_NativePopOver;
#else
    std::unique_ptr<DynamicTransferQuotaPopOver> mPopOver{new DynamicTransferQuotaPopOver(this)};
#endif

private slots:
    void unitTimeElapsed();

protected:
    void changeEvent(QEvent* event) override;

    mega::MegaPricing *pricing;
    mega::MegaApi *megaApi;
    HighDpiResize highDpiResize;
};

#endif // UPGRADEDIALOG_H
