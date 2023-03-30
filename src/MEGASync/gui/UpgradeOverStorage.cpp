#include "UpgradeOverStorage.h"
#include "ui_UpgradeOverStorage.h"
#include "Utilities.h"
#include "Preferences.h"
#include "gui/PlanWidget.h"

using namespace mega;

UpgradeOverStorage::UpgradeOverStorage(MegaApi* megaApi, std::shared_ptr<mega::MegaPricing> pricing,
                                       std::shared_ptr<MegaCurrency> currency, QWidget* parent) :
    QDialog(parent),
    mUi(new Ui::UpgradeOverStorage),
    mMegaApi (megaApi),
    mPricing (pricing),
    mCurrency (currency)
{
    mUi->setupUi(this);

    updatePlans();  
    configureAnimation();

    //Keep storage details hidden until we receive the account details
    mUi->lAccountUsed->hide();
}

UpgradeOverStorage::~UpgradeOverStorage()
{
    delete mUi;
}

void UpgradeOverStorage::setPricing(std::shared_ptr<mega::MegaPricing> pricing,
                                    std::shared_ptr<MegaCurrency> currency)
{
    if (pricing && currency)
    {
        mPricing = pricing;
        mCurrency = currency;
        updatePlans();
    }
}

void UpgradeOverStorage::refreshStorageDetails()
{
    auto preferences = Preferences::instance();
    auto totalStorage(preferences->totalStorage());
    mUi->lAccountUsed->setText(tr("100% of the %1 available used on your account.")
                               .arg(Utilities::getSizeString(totalStorage)));
    mUi->lAccountUsed->show();
}


void UpgradeOverStorage::updatePlans()
{
    if (mPricing && mCurrency)
    {
        clearPlans();
        bool isBillingCurrency (false);
        QVector<PlanWidget*> cards;
        int minPriceFontSize (std::numeric_limits<int>::max());
        QByteArray bSym (QByteArray::fromBase64(mCurrency->getCurrencySymbol()));
        QString billingCurrencySymbol (QString::fromUtf8(bSym.data()));
        QString billingCurrencyName (QString::fromUtf8(mCurrency->getCurrencyName()));
        QString localCurrencyName;
        QString localCurrencySymbol;
        QByteArray lSym (QByteArray::fromBase64(mCurrency->getLocalCurrencySymbol()));
        if (lSym.isEmpty())
        {
            localCurrencySymbol = billingCurrencySymbol;
            localCurrencyName = billingCurrencyName;
            isBillingCurrency = true;
        }
        else
        {
            localCurrencySymbol = QString::fromUtf8(lSym.data());
            localCurrencyName = QString::fromUtf8(mCurrency->getLocalCurrencyName());
        }

        QString userAgent (QString::fromUtf8(mMegaApi->getUserAgent()));
        int products (mPricing->getNumProducts());
        for (int it = 0; it < products; it++)
        {
            // Skip showing Pro card for flexi pro in UpgradeDialog
            if (mPricing->getProLevel(it) == mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI)
            {
                continue;
            }

            if (mPricing->getMonths(it) == 1)
            {
                PlanInfo data {
                    0, 0, 0,
                    mPricing->getProLevel(it),
                    mPricing->getGBPerStorage(it),
                    mPricing->getGBPerTransfer(it),
                    0, 0, 0, 0, 0, 0,
                    billingCurrencySymbol,
                    billingCurrencyName,
                    localCurrencySymbol,
                    localCurrencyName
                };

                if (!mPricing->isBusinessType(it))
                {
                    data.minUsers = 1;
                    data.gbStorage = mPricing->getGBStorage(it);
                    data.gbTransfer =  mPricing->getGBTransfer(it);
                    data.pricePerUserBilling = static_cast<unsigned int>(mPricing->getAmount(it));
                    data.pricePerUserLocal = isBillingCurrency ?
                                                 data.pricePerUserBilling
                                               : static_cast<unsigned int>(mPricing->getLocalPrice(it));
                }
                else
                {
                    data.minUsers = mPricing->getMinUsers(it);
                    data.gbStorage = mPricing->getGBStoragePerUser(it);
                    data.gbTransfer =  mPricing->getGBTransferPerUser(it);
                    data.pricePerUserBilling = mPricing->getPricePerUser(it);
                    data.pricePerStorageBilling = mPricing->getPricePerStorage(it);
                    data.pricePerTransferBilling = mPricing->getPricePerTransfer(it);
                    if (isBillingCurrency)
                    {
                        data.pricePerUserLocal = data.pricePerUserBilling;
                        data.pricePerStorageLocal = data.pricePerStorageBilling;
                        data.pricePerTransferLocal = data.pricePerTransferBilling;
                    }
                    else
                    {
                        data.pricePerUserLocal = mPricing->getLocalPricePerUser(it);
                        data.pricePerStorageLocal = mPricing->getLocalPricePerStorage(it);
                        data.pricePerTransferLocal = mPricing->getLocalPricePerTransfer(it);
                    }
                }

                PlanWidget* card (new PlanWidget(data, userAgent, this));
                mUi->wPlansLayout->addWidget(card);
                mUi->lPriceEstimation->setVisible(!isBillingCurrency);
                cards.append(card);
                minPriceFontSize = std::min(minPriceFontSize, card->getPriceFontSizePx());
            }
        }

        // Set the price font soze to the minimum found
        for (auto card : cards)
        {
            card->setPriceFontSizePx(minPriceFontSize);
        }
        show();
    }
}

void UpgradeOverStorage::clearPlans()
{
    while (QLayoutItem* item = mUi->wPlansLayout->takeAt(0))
    {
        if (QWidget* widget = item->widget())
        {
            widget->deleteLater();
        }
        delete item;
    }
}

void UpgradeOverStorage::configureAnimation()
{

    auto ratio = Utilities::getDevicePixelRatio();
    mAnimation.reset(new QMovie(ratio < 2 ? QLatin1String(":/animations/full-storage.gif")
                                          : QLatin1String(":/animations/full-storage@2x.gif")));

    mUi->lAnimationOverStorage->setMovie(mAnimation.get());
    mUi->lAnimationOverStorage->setAlignment(Qt::AlignHCenter | Qt::AlignVCenter);

    mAnimation.get()->start();
}

void UpgradeOverStorage::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        refreshStorageDetails();
    }
    QDialog::changeEvent(event);
}
