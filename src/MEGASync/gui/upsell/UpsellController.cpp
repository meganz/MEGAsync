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
constexpr int PRECISION_FOR_DECIMALS(2);
constexpr int NO_DECIMALS(0);
constexpr float CENTS_IN_1_UNIT(100.0f);
constexpr float NUM_MONTHS_PER_PLAN(12.0f);
constexpr float PERCENTAGE(100.0f);
constexpr long long TRANSFER_REMAINING_TIME_INTERVAL_MS(1000ll);
constexpr int64_t NB_B_IN_1GB(1024 * 1024 * 1024);
constexpr QLatin1Char BILLING_CURRENCY_REMARK('*');
constexpr const char* DEFAULT_PRO_URL("mega://#pro");
const std::map<int, const char*> PRO_LEVEL_TO_URL = {
    {Preferences::AccountType::ACCOUNT_TYPE_PROI,      "mega://#propay_1" },
    {Preferences::AccountType::ACCOUNT_TYPE_PROII,     "mega://#propay_2" },
    {Preferences::AccountType::ACCOUNT_TYPE_PROIII,    "mega://#propay_3" },
    {Preferences::AccountType::ACCOUNT_TYPE_LITE,      "mega://#propay_4" },
    {Preferences::AccountType::ACCOUNT_TYPE_STARTER,   "mega://#propay_11"},
    {Preferences::AccountType::ACCOUNT_TYPE_BASIC,     "mega://#propay_12"},
    {Preferences::AccountType::ACCOUNT_TYPE_ESSENTIAL, "mega://#propay_13"}
  // BUSINESS and PRO_FLEXI are not supported
};
const std::vector<int> ACCOUNT_TYPES_IN_ORDER = {Preferences::AccountType::ACCOUNT_TYPE_FREE,
                                                 Preferences::AccountType::ACCOUNT_TYPE_STARTER,
                                                 Preferences::AccountType::ACCOUNT_TYPE_BASIC,
                                                 Preferences::AccountType::ACCOUNT_TYPE_ESSENTIAL,
                                                 Preferences::AccountType::ACCOUNT_TYPE_LITE,
                                                 Preferences::AccountType::ACCOUNT_TYPE_PROI,
                                                 Preferences::AccountType::ACCOUNT_TYPE_PROII,
                                                 Preferences::AccountType::ACCOUNT_TYPE_PROIII,
                                                 Preferences::AccountType::ACCOUNT_TYPE_BUSINESS,
                                                 Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI};
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
                auto currentSelected(mPlans->getCurrentPlanSelected());
                mPlans->getPlan(currentSelected)->setSelected(false);
                emit dataChanged(currentSelected, currentSelected, QVector<int>() << role);
                updatePlansAt(data, row);
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
                field = data->name();
                break;
            }
            case UpsellPlans::RECOMMENDED_ROLE:
            {
                field = data->isRecommended();
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
                field = getLocalePriceString(mPlans->isMonthly() ? data->monthlyData().price() :
                                                                   data->yearlyData().price());
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

void UpsellController::openSelectedPlanUrl()
{
    auto row = mPlans->getCurrentPlanSelected();
    Utilities::openUrl(getUpsellPlanUrl(mPlans->getPlan(row)->proLevel()));
}

void UpsellController::setBilledPeriod(bool isMonthly)
{
    if (mPlans->isMonthly() != isMonthly)
    {
        mPlans->setMonthly(isMonthly);
    }
}

void UpsellController::setViewMode(UpsellPlans::ViewMode mode)
{
    mPlans->setViewMode(mode);
    if (mode == UpsellPlans::ViewMode::TRANSFER_EXCEEDED)
    {
        if (!mTransferFinishTimer)
        {
            mTransferFinishTimer = new QTimer(this);
            mTransferFinishTimer->setSingleShot(false);
            connect(mTransferFinishTimer,
                    &QTimer::timeout,
                    this,
                    &UpsellController::onTransferRemainingTimeElapsed);
        }
    }
}

void UpsellController::setTransferFinishTime(long long finishTime)
{
    mPlans->setTransferFinishTime(finishTime);
    onTransferRemainingTimeElapsed();
    if (mTransferFinishTimer && !mTransferFinishTimer->isActive())
    {
        mTransferFinishTimer->start(TRANSFER_REMAINING_TIME_INTERVAL_MS);
    }
}

QString UpsellController::getMinProPlanNeeded(long long usedStorage) const
{
    if (!mPlans)
    {
        return QString::fromLatin1("Pro");
    }

    int proLevel(-1);
    float amountPlanNeeded(0.0f);
    for (const auto& plan: mPlans->plans())
    {
        if (usedStorage < plan->monthlyData().gBStorage())
        {
            float currentAmountMonth(plan->monthlyData().price());
            if (proLevel == -1 || currentAmountMonth < amountPlanNeeded)
            {
                proLevel = plan->proLevel();
                amountPlanNeeded = currentAmountMonth;
            }
        }
    }

    return Utilities::getReadablePlanFromId(proLevel);
}

void UpsellController::onBilledPeriodChanged()
{
    emit dataChanged(0,
                     mPlans->size() - 1,
                     QVector<int>() << UpsellPlans::STORAGE_ROLE << UpsellPlans::TRANSFER_ROLE
                                    << UpsellPlans::PRICE_ROLE);
}

void UpsellController::onTransferRemainingTimeElapsed()
{
    long long remainingTime(mPlans->getTransferFinishTime() -
                            QDateTime::currentMSecsSinceEpoch() /
                                TRANSFER_REMAINING_TIME_INTERVAL_MS);
    if (remainingTime < 0)
    {
        remainingTime = 0;
    }
    mPlans->setTransferRemainingTime(Utilities::getTimeString(remainingTime, true, false));
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
        if (!isProLevelValid(pricing->getProLevel(i)))
        {
            continue;
        }

        addPlan(pricing, i);
    }

    if (!mPlans->plans().isEmpty())
    {
        setPlanDataForRecommended();
    }

    emit endInsertRows();
}

void UpsellController::process(mega::MegaCurrency* currency)
{
    QString localCurrencySymbol;
    QString localCurrencyName;
    QByteArray localByteSymbol(QByteArray::fromBase64(currency->getLocalCurrencySymbol()));
    if (localByteSymbol.isEmpty())
    {
        QByteArray byteSymbol(QByteArray::fromBase64(currency->getCurrencySymbol()));
        localCurrencySymbol = QString::fromUtf8(byteSymbol.data());
        localCurrencyName = QString::fromUtf8(currency->getCurrencyName());
    }
    else
    {
        localCurrencySymbol = QString::fromUtf8(localByteSymbol.data());
        localCurrencyName = QString::fromUtf8(currency->getLocalCurrencyName());
    }

    mPlans->setCurrency(localCurrencySymbol, localCurrencyName);
    mPlans->setBillingCurrency(localByteSymbol.isEmpty());
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
    const char* planUrlChar;
    auto it(PRO_LEVEL_TO_URL.find(proLevel));
    if (it != PRO_LEVEL_TO_URL.end())
    {
        planUrlChar = it->second;
    }
    else
    {
        planUrlChar = DEFAULT_PRO_URL;
    }

    QString planUrlString(QString::fromLatin1(planUrlChar));
    Utilities::getPROurlWithParameters(planUrlString);

    return QUrl(planUrlString);
}

QString UpsellController::getLocalePriceString(float price) const
{
    static const QLocale locale(QLocale().language(), QLocale().country());
    int precision(std::fmod(price, 1.) > 0. ? PRECISION_FOR_DECIMALS : NO_DECIMALS);
    QString priceStr(locale.toCurrencyString(price, mPlans->getCurrencySymbol(), precision));
    if (!mPlans->isBillingCurrency())
    {
        priceStr += BILLING_CURRENCY_REMARK;
    }
    return priceStr;
}

UpsellPlans::Data::AccountBillingPlanData
    UpsellController::createAccountBillingPlanData(int storage, int transfer, int price) const
{
    UpsellPlans::Data::AccountBillingPlanData planData(static_cast<int64_t>(storage) * NB_B_IN_1GB,
                                                       static_cast<int64_t>(transfer) * NB_B_IN_1GB,
                                                       static_cast<float>(price) / CENTS_IN_1_UNIT);
    return planData;
}

int UpsellController::calculateDiscount(float monthlyPrice, float yearlyPrice) const
{
    return static_cast<int>(PERCENTAGE -
                            (yearlyPrice * PERCENTAGE) / (monthlyPrice * NUM_MONTHS_PER_PLAN));
}

void UpsellController::addPlan(mega::MegaPricing* pricing, int index)
{
    auto proLevel(pricing->getProLevel(index));
    QString name(Utilities::getReadablePlanFromId(proLevel, true));
    int price(mPlans->isBillingCurrency() ? pricing->getAmount(index) :
                                            pricing->getLocalPrice(index));
    auto planData(createAccountBillingPlanData(pricing->getGBStorage(index),
                                               pricing->getGBTransfer(index),
                                               price));
    if (mPlans->addPlan(std::make_shared<UpsellPlans::Data>(proLevel, name)) &&
        pricing->getMonths(index) == MONTH_PERIOD)
    {
        mPlans->getPlanByProLevel(proLevel)->setMonthlyData(planData);
    }
    else if (pricing->getMonths(index) == YEAR_PERIOD)
    {
        mPlans->getPlanByProLevel(proLevel)->setYearlyData(planData);
    }
}

void UpsellController::setPlanDataForRecommended()
{
    int row(0);
    int current(Preferences::instance()->accountType());
    auto itCurrent(
        std::find(ACCOUNT_TYPES_IN_ORDER.cbegin(), ACCOUNT_TYPES_IN_ORDER.cend(), current));
    for (const auto& plan: mPlans->plans())
    {
        auto itNext(std::find(itCurrent, ACCOUNT_TYPES_IN_ORDER.cend(), plan->proLevel()));
        if (itNext != ACCOUNT_TYPES_IN_ORDER.cend())
        {
            row = mPlans->plans().indexOf(plan);
            break;
        }
    }

    auto plan(mPlans->getPlan(row));
    plan->setSelected(true);
    plan->setRecommended(true);
    updatePlansAt(plan, row);
}

void UpsellController::updatePlansAt(const std::shared_ptr<UpsellPlans::Data>& data, int row)
{
    mPlans->setCurrentPlanSelected(row);
    mPlans->setCurrentDiscount(
        calculateDiscount(data->monthlyData().price(), data->yearlyData().price()));
    mPlans->setCurrentPlanName(data->name());
}
