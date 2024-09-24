#include "UpsellController.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "RequestListenerManager.h"
#include "UpsellPlans.h"
#include "Utilities.h"

#include <QUrl>

namespace
{
constexpr int MONTH_PERIOD(1);
constexpr int YEAR_PERIOD(12);
const std::map<int, QString> proLevelToUrl = {
    {Preferences::AccountType::ACCOUNT_TYPE_PROI,      QString::fromUtf8("mega://#propay_1") },
    {Preferences::AccountType::ACCOUNT_TYPE_PROII,     QString::fromUtf8("mega://#propay_2") },
    {Preferences::AccountType::ACCOUNT_TYPE_PROIII,    QString::fromUtf8("mega://#propay_3") },
    {Preferences::AccountType::ACCOUNT_TYPE_LITE,      QString::fromUtf8("mega://#propay_4") },
    {Preferences::AccountType::ACCOUNT_TYPE_STARTER,   QString::fromUtf8("mega://#propay_11")},
    {Preferences::AccountType::ACCOUNT_TYPE_BASIC,     QString::fromUtf8("mega://#propay_12")},
    {Preferences::AccountType::ACCOUNT_TYPE_ESSENTIAL, QString::fromUtf8("mega://#propay_13")}
  // BUSINESS and PRO_FLEXI are not supported
};
const QString defaultUrl = QString::fromUtf8("mega://#pro");
}

UpsellController::UpsellController(QObject* parent):
    QObject(parent),
    mPlans(std::make_shared<UpsellPlans>())
{
    connect(mPlans.get(),
            &UpsellPlans::monthlyChanged,
            this,
            &UpsellController::onBilledPeriodChanged);

    mDelegateListener = RequestListenerManager::instance().registerAndGetFinishListener(this);
    MegaSyncApp->getMegaApi()->getPricing(mDelegateListener.get());
}

void UpsellController::onRequestFinish(mega::MegaRequest* request, mega::MegaError* error)
{
    switch (request->getType())
    {
        case mega::MegaRequest::TYPE_GET_PRICING:
        {
            if (error->getErrorCode() == mega::MegaError::API_OK)
            {
                processGetPricingRequest(request->getPricing(), request->getCurrency());
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

bool UpsellController::setData(int row, const QVariant& value, int role)
{
    return setData(mPlans->getPlan(row), value, role);
}

bool UpsellController::setData(std::shared_ptr<UpsellPlans::Data> data, QVariant value, int role)
{
    auto result(true);

    switch (role)
    {
        case UpsellPlans::NAME_ROLE:
        {
            break;
        }
        case UpsellPlans::RECOMMENDED_ROLE:
        {
            break;
        }
        case UpsellPlans::STORAGE_ROLE:
        {
            break;
        }
        case UpsellPlans::TRANSFER_ROLE:
        {
            break;
        }
        case UpsellPlans::PRICE_ROLE:
        {
            break;
        }
        case UpsellPlans::SELECTED_ROLE:
        {
            data->setSelected(value.toBool());
            auto row = mPlans->plans().indexOf(data);
            emit dataChanged(row, row, QVector<int>() << role);

            if (value.toBool())
            {
                auto currentSelected(mPlans->currentPlanSelected());
                mPlans->deselectCurrentPlanSelected();
                emit dataChanged(currentSelected, currentSelected, QVector<int>() << role);
                mPlans->setCurrentPlanSelected(row);
            }
            break;
        }
        default:
        {
            result = false;
            break;
        }
    }

    return result;
}

QVariant UpsellController::data(int row, int role) const
{
    return data(mPlans->getPlan(row), role);
}

QVariant UpsellController::data(std::shared_ptr<UpsellPlans::Data> data, int role) const
{
    QVariant field;

    if (data)
    {
        switch (role)
        {
            case UpsellPlans::NAME_ROLE:
            {
                field = Utilities::getReadablePlanFromId(data->proLevel(), true);
                break;
            }
            case UpsellPlans::RECOMMENDED_ROLE:
            {
                field = data->recommended();
                break;
            }
            case UpsellPlans::STORAGE_ROLE:
            {
                field =
                    Utilities::getSizeString(mPlans->isMonthly() ? data->monthlyData().gBStorage() :
                                                                   data->yearlyData().gBStorage());
                break;
            }
            case UpsellPlans::TRANSFER_ROLE:
            {
                field = Utilities::getSizeString(mPlans->isMonthly() ?
                                                     data->monthlyData().gBTransfer() :
                                                     data->yearlyData().gBTransfer());
                break;
            }
            case UpsellPlans::PRICE_ROLE:
            {
                field =
                    mPlans->isMonthly() ? data->monthlyData().price() : data->yearlyData().price();
                break;
            }
            case UpsellPlans::SELECTED_ROLE:
            {
                field = data->selected();
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return field;
}

std::shared_ptr<UpsellPlans> UpsellController::getPlans() const
{
    return mPlans;
}

void UpsellController::openSelectedPlan()
{
    auto row = mPlans->currentPlanSelected();
    Utilities::openUrl(getUpsellPlanUrl(mPlans->getPlan(row)->proLevel()));
}

void UpsellController::onBilledPeriodChanged()
{
    QVector<int> roles;
    roles << UpsellPlans::STORAGE_ROLE << UpsellPlans::TRANSFER_ROLE << UpsellPlans::PRICE_ROLE;
    emit dataChanged(0, mPlans->size() - 1, roles);
}

void UpsellController::processGetPricingRequest(mega::MegaPricing* pricing,
                                                mega::MegaCurrency* currency)
{
    if (!pricing || !currency)
    {
        return;
    }

    process(currency);
    process(pricing);
}

void UpsellController::process(mega::MegaPricing* pricing)
{
    auto numPlans(countNumPlans(pricing));

    emit beginInsertRows(0, numPlans - 1);

    for (int i = 0; i < pricing->getNumProducts(); ++i)
    {
        auto proLevel(pricing->getProLevel(i));
        if (!isProLevelValid(proLevel))
        {
            continue;
        }

        if (mPlans->addPlan(std::make_shared<UpsellPlans::Data>(proLevel, true)) &&
            pricing->getMonths(i) == MONTH_PERIOD)
        {
            mPlans->getPlanByProLevel(proLevel)->setMonthlyData(
                UpsellPlans::Data::AccountBillingPlanData(pricing->getGBStorage(i),
                                                          pricing->getGBTransfer(i),
                                                          pricing->getAmount(i)));
        }
        else if (pricing->getMonths(i) == YEAR_PERIOD)
        {
            mPlans->getPlanByProLevel(proLevel)->setYearlyData(
                UpsellPlans::Data::AccountBillingPlanData(pricing->getGBStorage(i),
                                                          pricing->getGBTransfer(i),
                                                          pricing->getAmount(i)));
        }
    }

    // Fist time, select the first plan and set it as current selected.
    mPlans->plans().first()->setSelected(true);
    mPlans->setCurrentPlanSelected(0);

    emit endInsertRows();
}

void UpsellController::process(mega::MegaCurrency* currency)
{
    QString localCurrencySymbol;
    QByteArray localByteSymbol(QByteArray::fromBase64(currency->getLocalCurrencySymbol()));
    if (localByteSymbol.isEmpty())
    {
        QByteArray byteSymbol(QByteArray::fromBase64(currency->getCurrencySymbol()));
        localCurrencySymbol = QString::fromUtf8(byteSymbol.data());
    }
    else
    {
        localCurrencySymbol = QString::fromUtf8(localByteSymbol.data());
    }

    mPlans->setCurrencySymbol(localCurrencySymbol);
}

int UpsellController::countNumPlans(mega::MegaPricing* pricing) const
{
    auto numProducts(0);
    QSet<int> processedProLevels;
    for (int i = 0; i < pricing->getNumProducts(); ++i)
    {
        auto proLevel = pricing->getProLevel(i);

        if (!isProLevelValid(proLevel) || processedProLevels.contains(proLevel))
        {
            continue;
        }

        processedProLevels.insert(proLevel);
        numProducts++;
    }

    return numProducts;
}

bool UpsellController::isProLevelValid(int proLevel) const
{
    // Skip showing current plan, pro flexi, business and feature plans in the dialog.
    return proLevel != Preferences::instance()->accountType() &&
           proLevel != mega::MegaAccountDetails::ACCOUNT_TYPE_FREE &&
           proLevel != mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI &&
           proLevel != mega::MegaAccountDetails::ACCOUNT_TYPE_BUSINESS &&
           proLevel != mega::MegaAccountDetails::ACCOUNT_TYPE_FEATURE;
}

QUrl UpsellController::getUpsellPlanUrl(int proLevel)
{
    QString planUrlString;
    auto it = proLevelToUrl.find(proLevel);
    if (it != proLevelToUrl.end())
    {
        planUrlString = it->second;
    }
    else
    {
        planUrlString = defaultUrl;
    }

    Utilities::getPROurlWithParameters(planUrlString);

    return QUrl(planUrlString);
}
