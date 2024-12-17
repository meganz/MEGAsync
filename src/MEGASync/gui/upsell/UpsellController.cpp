#include "UpsellController.h"

#include "AccountDetailsManager.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "QmlManager.h"
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
constexpr const char* PERIOD_SUFFIX_URL("?m=");
const std::map<int, const char*> PRO_LEVEL_TO_URL = {
    {Preferences::AccountType::ACCOUNT_TYPE_PROI,      "mega://#propay_1"  },
    {Preferences::AccountType::ACCOUNT_TYPE_PROII,     "mega://#propay_2"  },
    {Preferences::AccountType::ACCOUNT_TYPE_PROIII,    "mega://#propay_3"  },
    {Preferences::AccountType::ACCOUNT_TYPE_LITE,      "mega://#propay_4"  },
    {Preferences::AccountType::ACCOUNT_TYPE_STARTER,   "mega://#propay_11" },
    {Preferences::AccountType::ACCOUNT_TYPE_BASIC,     "mega://#propay_12" },
    {Preferences::AccountType::ACCOUNT_TYPE_ESSENTIAL, "mega://#propay_13" },
    {Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI, "mega://#propay_101"}
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
    mPlans(std::make_shared<UpsellPlans>(nullptr)),
    mTransferFinishTimer(nullptr)
{
    connect(mPlans.get(),
            &UpsellPlans::monthlyChanged,
            this,
            &UpsellController::onBilledPeriodChanged);

    // Subscribe to data updates
    AccountDetailsManager::instance()->attachStorageObserver(*this);
    AccountDetailsManager::instance()->updateUserStats(AccountDetailsManager::Flag::ALL,
                                                       true,
                                                       USERSTATS_STORAGECLICKED);
}

UpsellController::~UpsellController()
{
    AccountDetailsManager::instance()->dettachStorageObserver(*this);
}

void UpsellController::updateStorageElements()
{
    if (mPlans->plans().isEmpty())
    {
        return;
    }

    reviewPlansToCheckProFlexi(mPlans->plans());

    emit dataChanged(0,
                     mPlans->size() - 1,
                     QVector<int>() << UpsellPlans::AVAILABLE_ROLE << UpsellPlans::CURRENT_PLAN_ROLE
                                    << UpsellPlans::SHOW_PRO_FLEXI_MESSAGE
                                    << UpsellPlans::SHOW_ONLY_PRO_FLEXI);
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
                emit dataReady();
            }
            break;
        }
        default:
        {
            break;
        }
    }
}

void UpsellController::registerQmlRootContextProperties()
{
    QmlManager::instance()->setRootContextProperty(mPlans.get());
}

void UpsellController::requestPricingData()
{
    auto listener(RequestListenerManager::instance().registerAndGetFinishListener(this));
    MegaSyncApp->getMegaApi()->getPricing(listener.get());
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
            case Qt::DisplayRole:
            {
                field = mPlans->isMonthly() ? data->monthlyData().isValid() :
                                              data->yearlyData().isValid();
                break;
            }
            case UpsellPlans::NAME_ROLE:
            {
                field = data->name();
                break;
            }
            case UpsellPlans::BUTTON_NAME_ROLE:
            {
                if (isOnlyProFlexiAvailable(data))
                {
                    // For Pro III, if the storage is full, only the Pro Flexi plan is available.
                    // We check if the storage if the plan offered is enough for the current used
                    // one. Override the last plan name to show the Pro Flexi name instead of Pro
                    // III.
                    field = Utilities::getReadablePlanFromId(
                        mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI,
                        true);
                }
                else
                {
                    field = data->name();
                }
                break;
            }
            case UpsellPlans::RECOMMENDED_ROLE:
            {
                if (isOnlyProFlexiAvailable(data))
                {
                    // For Pro III, only Pro III and/or Pro Flexi are available.
                    // Override recommended to show the border as for recommended plans.
                    field = true;
                }
                else
                {
                    field = data->isRecommended();
                }
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
            case UpsellPlans::TOTAL_PRICE_WITHOUT_DISCOUNT_ROLE:
            {
                field = getLocalePriceString(
                    calculateTotalPriceWithoutDiscount(data->monthlyData().price()));
                break;
            }
            case UpsellPlans::MONTHLY_PRICE_WITH_DISCOUNT_ROLE:
            {
                field = getLocalePriceString(
                    calculateMonthlyPriceWithDiscount(data->yearlyData().price()));
                break;
            }
            case UpsellPlans::CURRENT_PLAN_ROLE:
            {
                field = data->proLevel() == Preferences::instance()->accountType() ||
                        data->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI;
                break;
            }
            case UpsellPlans::AVAILABLE_ROLE:
            {
                field = isAvailable(data);
                break;
            }
            case UpsellPlans::SHOW_PRO_FLEXI_MESSAGE:
            {
                field = data->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI;
                break;
            }
            case UpsellPlans::SHOW_ONLY_PRO_FLEXI:
            {
                field = isOnlyProFlexiAvailable(data);
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

void UpsellController::openPlanUrl(int index)
{
    auto plan = mPlans->getPlan(index);
    if (!plan)
    {
        return;
    }

    Utilities::openUrl(getUpsellPlanUrl(plan->proLevel()));
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
    if (viewMode() != mode)
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

        // Force update of the plans to show the correct ones.
        emit dataChanged(0, mPlans->size() - 1, QVector<int>() << UpsellPlans::AVAILABLE_ROLE);
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

    return Utilities::getReadablePlanFromId(proLevel, true);
}

UpsellPlans::ViewMode UpsellController::viewMode() const
{
    return mPlans->getViewMode();
}

UpsellPlans::ViewMode UpsellController::viewMode() const
{
    return mPlans->getViewMode();
}

void UpsellController::onBilledPeriodChanged()
{
    updatePlans();

    emit dataChanged(0,
                     mPlans->size() - 1,
                     QVector<int>()
                         << Qt::DisplayRole << UpsellPlans::STORAGE_ROLE
                         << UpsellPlans::TRANSFER_ROLE << UpsellPlans::PRICE_ROLE
                         << UpsellPlans::AVAILABLE_ROLE << UpsellPlans::RECOMMENDED_ROLE
                         << UpsellPlans::TOTAL_PRICE_WITHOUT_DISCOUNT_ROLE
                         << UpsellPlans::MONTHLY_PRICE_WITH_DISCOUNT_ROLE
                         << UpsellPlans::CURRENT_PLAN_ROLE << UpsellPlans::SHOW_PRO_FLEXI_MESSAGE
                         << UpsellPlans::SHOW_ONLY_PRO_FLEXI);
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
    QList<std::shared_ptr<UpsellPlans::Data>> plans(getAllowedPlans(pricing));
    if (plans.isEmpty())
    {
        return;
    }

    reviewPlansToCheckProFlexi(plans);

    emit beginInsertRows(0, plans.size() - 1);

    mPlans->addPlans(plans);
    updatePlans();

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

QList<std::shared_ptr<UpsellPlans::Data>>
    UpsellController::getAllowedPlans(mega::MegaPricing* pricing)
{
    QList<std::shared_ptr<UpsellPlans::Data>> plans;
    for (int i = 0; i < pricing->getNumProducts(); ++i)
    {
        auto proLevel(pricing->getProLevel(i));
        if (!isProLevelValid(proLevel))
        {
            continue;
        }

        auto plan(appendPlan(proLevel, plans));
        int price(mPlans->isBillingCurrency() ? pricing->getAmount(i) : pricing->getLocalPrice(i));
        auto planData(createAccountBillingPlanData(pricing->getGBStorage(i),
                                                   pricing->getGBTransfer(i),
                                                   price));
        if (pricing->getMonths(i) == MONTH_PERIOD)
        {
            plan->setMonthlyData(planData);
        }
        else if (pricing->getMonths(i) == YEAR_PERIOD)
        {
            plan->setYearlyData(planData);
        }
    }
    return plans;
}

std::shared_ptr<UpsellPlans::Data>
    UpsellController::appendPlan(int proLevel, QList<std::shared_ptr<UpsellPlans::Data>>& plans)
{
    QString name(Utilities::getReadablePlanFromId(proLevel, true));
    std::shared_ptr<UpsellPlans::Data> plan(nullptr);
    auto it = std::find_if(plans.begin(),
                           plans.end(),
                           [proLevel](const std::shared_ptr<UpsellPlans::Data>& existingPlan)
                           {
                               return existingPlan->proLevel() == proLevel;
                           });
    if (it != plans.end())
    {
        plan = *it;
    }
    else
    {
        plan = std::make_shared<UpsellPlans::Data>(proLevel, name);
        plans.append(plan);
    }

    return plan;
}

bool UpsellController::isProLevelValid(int proLevel) const
{
    // Skip showing plans under the current one, pro flexi, business and feature in the dialog.
    return proLevel != mega::MegaAccountDetails::ACCOUNT_TYPE_FREE &&
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

    QString planUrlString(QString::fromLatin1(planUrlChar) +
                          QString::fromLatin1(PERIOD_SUFFIX_URL));
    planUrlString +=
        (mPlans->isMonthly() ? QString::number(MONTH_PERIOD) : QString::number(YEAR_PERIOD));
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

void UpsellController::updatePlans()
{
    int currentRecommendedRow(getRowForCurrentRecommended());
    int row(getRowForNextRecommendedPlan());
    if (row != -1 && currentRecommendedRow != row)
    {
        resetRecommended();

        auto plan(mPlans->getPlan(row));
        plan->setRecommended(true);
        updatePlansAt(plan, row);
    }
}

void UpsellController::updatePlansAt(const std::shared_ptr<UpsellPlans::Data>& data, int row)
{
    // Calculate discount if both monthly and yearly data are available, otherwise set it to -1
    // to indicate that the discount is not available.
    int discount(-1);
    if (mPlans->getPlan(row)->monthlyData().isValid() &&
        mPlans->getPlan(row)->yearlyData().isValid())
    {
        discount = calculateDiscount(data->monthlyData().price(), data->yearlyData().price());
    }
    mPlans->setCurrentDiscount(discount);
}

int UpsellController::getRowForNextRecommendedPlan() const
{
    int current(Preferences::instance()->accountType());
    auto itCurrent =
        std::find(ACCOUNT_TYPES_IN_ORDER.cbegin(), ACCOUNT_TYPES_IN_ORDER.cend(), current);

    const auto& plans(mPlans->plans());
    auto it = std::find_if(
        plans.cbegin(),
        plans.cend(),
        [itCurrent, this](const auto& plan)
        {
            // Recommended plan is the next one in the list that is not the current one,
            // not a lower tier plan (never recommended) and is available.
            auto itNext(std::find(itCurrent, ACCOUNT_TYPES_IN_ORDER.cend(), plan->proLevel()));
            bool isCurrent(plan->proLevel() == Preferences::instance()->accountType());
            bool isLTP(plan->proLevel() == mega::MegaAccountDetails::ACCOUNT_TYPE_STARTER ||
                       plan->proLevel() == mega::MegaAccountDetails::ACCOUNT_TYPE_BASIC ||
                       plan->proLevel() == mega::MegaAccountDetails::ACCOUNT_TYPE_ESSENTIAL);
            return itNext != ACCOUNT_TYPES_IN_ORDER.cend() && !isCurrent && !isLTP &&
                   isAvailable(plan);
        });

    return (it != plans.cend()) ? static_cast<int>(std::distance(plans.cbegin(), it)) : -1;
}

void UpsellController::resetRecommended()
{
    const auto& plans(mPlans->plans());
    std::for_each(plans.begin(),
                  plans.end(),
                  [](auto& plan)
                  {
                      plan->setRecommended(false);
                  });
}

int UpsellController::getRowForCurrentRecommended()
{
    int currentRecommendedRow(-1);
    const auto& plans(mPlans->plans());
    auto it = std::find_if(plans.cbegin(),
                           plans.cend(),
                           [](const auto& plan)
                           {
                               return plan->isRecommended();
                           });

    if (it != plans.cend())
    {
        currentRecommendedRow = plans.indexOf(*it);
    }
    return currentRecommendedRow;
}

bool UpsellController::isAvailable(const std::shared_ptr<UpsellPlans::Data>& data) const
{
    return (!isPlanUnderCurrentProLevel(data->proLevel())) &&
           storageFitsUnderStorageOQConditions(data) &&
           (mPlans->isMonthly() ? data->monthlyData().isValid() : data->yearlyData().isValid());
}

bool UpsellController::isPlanUnderCurrentProLevel(int proLevel) const
{
    int currentAccountType(Preferences::instance()->accountType());
    auto currentLevelIt(std::find(ACCOUNT_TYPES_IN_ORDER.cbegin(),
                                  ACCOUNT_TYPES_IN_ORDER.cend(),
                                  currentAccountType));
    auto planLevelIt(
        std::find(ACCOUNT_TYPES_IN_ORDER.cbegin(), ACCOUNT_TYPES_IN_ORDER.cend(), proLevel));

    return planLevelIt < currentLevelIt;
}

bool UpsellController::planFitsUnderStorageOQConditions(int64_t planGbStorage) const
{
    bool isFullStorageOQ(viewMode() == UpsellPlans::ViewMode::STORAGE_FULL);
    bool isAlmostFullStorageOQ(viewMode() == UpsellPlans::ViewMode::STORAGE_ALMOST_FULL);

    auto totalStorage(Preferences::instance()->totalStorage());
    auto usedStorage(Preferences::instance()->usedStorage());
    bool isTxExceeded(viewMode() == UpsellPlans::ViewMode::TRANSFER_EXCEEDED &&
                      (usedStorage < totalStorage));
    bool isFullStorageUnderTxExceeded(viewMode() == UpsellPlans::ViewMode::TRANSFER_EXCEEDED &&
                                      (usedStorage >= totalStorage));

    bool isStorageFit(usedStorage < planGbStorage);

    return (isAlmostFullStorageOQ || isTxExceeded ||
            ((isFullStorageOQ || isFullStorageUnderTxExceeded) && isStorageFit));
}

float UpsellController::calculateTotalPriceWithoutDiscount(float monthlyPrice) const
{
    return monthlyPrice * NUM_MONTHS_PER_PLAN;
}

float UpsellController::calculateMonthlyPriceWithDiscount(float yearlyPrice) const
{
    return yearlyPrice / NUM_MONTHS_PER_PLAN;
}

bool UpsellController::isOnlyProFlexiAvailable(const std::shared_ptr<UpsellPlans::Data>& data,
                                               int proLevel) const
{
    int currentAccountType(Preferences::instance()->accountType());
    return currentAccountType == mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII &&
           data->proLevel() == proLevel && !storageFitsUnderStorageOQConditions(data);
}

bool UpsellController::storageFitsUnderStorageOQConditions(
    const std::shared_ptr<UpsellPlans::Data>& data) const
{
    // Check if the used storage fits in the plan.
    int64_t planStorage(mPlans->isMonthly() ? data->monthlyData().gBStorage() :
                                              data->yearlyData().gBStorage());
    return planFitsUnderStorageOQConditions(planStorage);
}

bool UpsellController::isOnlyProFlexiAvailable(
    const QList<std::shared_ptr<UpsellPlans::Data>>& plans) const
{
    auto it = std::find_if(
        plans.cbegin(),
        plans.cend(),
        [this](const auto& plan)
        {
            return isOnlyProFlexiAvailable(plan, mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII);
        });

    return it != plans.cend();
}

void UpsellController::reviewPlansToCheckProFlexi(
    const QList<std::shared_ptr<UpsellPlans::Data>>& plans)
{
    if (isOnlyProFlexiAvailable(plans))
    {
        // For Pro III, if the storage is full, only the Pro Flexi plan is available.
        // We check if the storage if the plan offered is enough for the current used
        // one. Override the pro level name to redirect to Pro Flexi propay link.
        // Pro flexi is only available monthly.
        auto it(std::find_if(plans.cbegin(),
                             plans.cend(),
                             [](const auto& plan)
                             {
                                 return plan->proLevel() ==
                                        mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII;
                             }));
        if (it == plans.cend())
        {
            return;
        }

        auto proIIIPlan(*it);
        proIIIPlan->setProLevel(mega::MegaAccountDetails::ACCOUNT_TYPE_PRO_FLEXI);
        mPlans->setOnlyProFlexiAvailable(true);
        setBilledPeriod(true);
    }
}
