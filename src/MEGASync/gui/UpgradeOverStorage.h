#ifndef UPGRADEOVERSTORAGE_H
#define UPGRADEOVERSTORAGE_H

#include <megaapi.h>

#include <QDialog>
#include <QHBoxLayout>
#include <QMovie>
#include "control/Utilities.h"

#include <memory>

namespace Ui {
class UpgradeOverStorage;
}

class UpgradeOverStorage : public QDialog, public IAccountObserver
{
    Q_OBJECT

public:
    explicit UpgradeOverStorage(mega::MegaApi* megaApi, std::shared_ptr<mega::MegaPricing> pricing,
                                std::shared_ptr<mega::MegaCurrency> currency,
                                QWidget* parent = nullptr);
    ~UpgradeOverStorage();

    void refreshStorageDetails();
    void setPricing(std::shared_ptr<mega::MegaPricing> pricing,
                    std::shared_ptr<mega::MegaCurrency> currency);

    void updateAccountElements() override;

protected:
    void changeEvent(QEvent* event) override;

private:
    void updatePlans();
    void clearPlans();
    void configureAnimation();

    Ui::UpgradeOverStorage* mUi;
    std::unique_ptr<QMovie> mAnimation;
    mega::MegaApi* mMegaApi;
    std::shared_ptr<mega::MegaPricing> mPricing;
    std::shared_ptr<mega::MegaCurrency> mCurrency;
};

#endif // UPGRADEOVERSTORAGE_H
