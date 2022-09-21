#ifndef UPGRADEDIALOG_H
#define UPGRADEDIALOG_H

#include "HighDpiResize.h"

#include <megaapi.h>

#include <QDialog>
#include <QHBoxLayout>
#include <QMovie>
#include <QTimer>

#include <memory>

namespace Ui {
class UpgradeDialog;
}

class UpgradeDialog : public QDialog
{
    Q_OBJECT

public:
    explicit UpgradeDialog(mega::MegaApi* megaApi, std::shared_ptr<mega::MegaPricing> pricing,
                           std::shared_ptr<mega::MegaCurrency> currency,
                           QWidget* parent = nullptr);
    ~UpgradeDialog();

    void setPricing(std::shared_ptr<mega::MegaPricing> pricing,
                    std::shared_ptr<mega::MegaCurrency> currency);
    void setTimestamp(long long finishTime);

protected:
    void changeEvent(QEvent* event) override;

private:
    void updatePlans();
    void clearPlans();
    void configureAnimation();

    Ui::UpgradeDialog* mUi;
    std::unique_ptr<QMovie> mAnimation;
    HighDpiResize mHighDpiResize;
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaPricing> mPricing;
    std::shared_ptr<mega::MegaCurrency> mCurrency;
    long long mFinishTime;
    QTimer* mTimer;

private slots:
    void unitTimeElapsed();
};

#endif // UPGRADEDIALOG_H
