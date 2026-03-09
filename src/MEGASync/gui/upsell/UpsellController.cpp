#include "UpsellController.h"

#include "AccountDetailsManager.h"
#include "megaapi.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "QmlManager.h"
#include "RequestListenerManager.h"
#include "ServiceUrls.h"
#include "UpsellPlans.h"
#include "Utilities.h"

#include <QUrl>

namespace
{
constexpr int MONTH_PERIOD(1);
constexpr int YEAR_PERIOD(12);
constexpr double NUM_MONTHS_PER_PLAN(12.);
constexpr double PERCENTAGE(100.);
constexpr double CENTS_IN_1_UNIT(100.);
constexpr int TRANSFER_REMAINING_TIME_INTERVAL_MS(1000);
constexpr int64_t NB_B_IN_1GB(1024 * 1024 * 1024);
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
constexpr double DOUBLE_COMPARISON_EPSILON = 1e-5;
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

    mPlans->setPro(Preferences::instance()->accountType() !=
                   Preferences::AccountType::ACCOUNT_TYPE_FREE);
}

UpsellController::UpsellController(bool proFlexiTrick, QObject* parent):
    UpsellController(parent)
{
    mProFlexiTrick = proFlexiTrick;
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

    if (mProFlexiTrick)
    {
        reviewPlansToCheckProFlexi(mPlans->plans());
    }

    emit dataChanged(0, mPlans->size() - 1);
    mPlans->setPro(Preferences::instance()->accountType() !=
                   Preferences::AccountType::ACCOUNT_TYPE_FREE);
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

QVariant UpsellController::data(std::shared_ptr<UpsellPlans::Data> plan, int role) const
{
    QVariant field;

    if (plan)
    {
        switch (role)
        {
            case Qt::DisplayRole:
            {
                field = mPlans->isMonthly() ? plan->monthlyData().isValid() :
                                              plan->yearlyData().isValid();
                break;
            }
            case UpsellPlans::NAME_ROLE:
            {
                field = plan->name();
                break;
            }
            case UpsellPlans::BUTTON_NAME_ROLE:
            {
                if (plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI &&
                    (isOnlyProFlexiAvailable(plan) ||
                     Preferences::instance()->accountType() ==
                         Preferences::AccountType::ACCOUNT_TYPE_PROIII))
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
                    field = plan->name();
                }
                break;
            }
            case UpsellPlans::RECOMMENDED_ROLE:
            {
                if (isOnlyProFlexiAvailable(plan) &&
                    plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI)
                {
                    // For Pro III, only Pro III and/or Pro Flexi are available.
                    // Override recommended to show the border as for recommended plans.
                    field = true;
                }
                else
                {
                    field = plan->isRecommended() && !mPlans->hasDiscounts();
                }
                break;
            }
            case UpsellPlans::STORAGE_ROLE:
            {
                field =
                    Utilities::getSizeString(mPlans->isMonthly() ? plan->monthlyData().gBStorage() :
                                                                   plan->yearlyData().gBStorage());
                break;
            }
            case UpsellPlans::TRANSFER_ROLE:
            {
                field = Utilities::getSizeString(mPlans->isMonthly() ?
                                                     plan->monthlyData().gBTransfer() :
                                                     plan->yearlyData().gBTransfer());
                break;
            }
            case UpsellPlans::PRICE_AFTER_TAX_ROLE:
            {
                auto isMonthly = mPlans->isMonthly();
                auto isYearly = !isMonthly;
                double price = isMonthly ? plan->monthlyData().priceAfterTax() :
                                           plan->yearlyData().priceAfterTax();
                auto discount = mPlans->getPlanDiscount(plan);
                const double months = isMonthly ? MONTH_PERIOD : YEAR_PERIOD;
                if (discount.has_value())
                {
                    if ((isMonthly && plan->hasMonthlyDiscount()) ||
                        ((isYearly && plan->hasYearlyDiscount())))
                    {
                        price *= (100 - discount->percentage) / 100.;
                    }
                }
                field = getLocalePriceString(price / months);
                break;
            }
            case UpsellPlans::PRICE_BEFORE_TAX_ROLE:
            {
                double price = mPlans->isMonthly() ? plan->monthlyData().priceBeforeTax() :
                                                     plan->yearlyData().priceBeforeTax();
                auto discount = mPlans->getPlanDiscount(plan);
                const double months = mPlans->isMonthly() ? MONTH_PERIOD : YEAR_PERIOD;
                if (discount.has_value())
                {
                    price *= (100 - discount->percentage) / 100.;
                }
                field = getLocalePriceString(price / months);
                break;
            }
            case UpsellPlans::TOTAL_PRICE_WITHOUT_DISCOUNT_ROLE:
            {
                double price =
                    mPlans->isMonthly() ?
                        plan->monthlyData().priceBeforeTax() :
                        calculateTotalPriceWithoutDiscount(plan->yearlyData().priceBeforeTax());
                field = getLocalePriceString(price);
                break;
            }
            case UpsellPlans::MONTHLY_PRICE_WITH_DISCOUNT_ROLE:
            {
                double price =
                    mPlans->isMonthly() ?
                        plan->monthlyData().priceAfterTax() :
                        calculateMonthlyPriceWithDiscount(plan->yearlyData().priceAfterTax());
                field = getLocalePriceString(price);
                break;
            }
            case UpsellPlans::MONTHLY_BASE_PRICE_ROLE:
            {
                double price = -1.;
                if (plan->monthlyData().isValid())
                {
                    price = plan->monthlyData().priceBeforeTax();
                }
                else
                {
                    price = plan->yearlyData().priceBeforeTax() / NUM_MONTHS_PER_PLAN;
                }
                field = price < 0. ? QString() : getLocalePriceString(price);
                break;
            }
            case UpsellPlans::HAS_TAX:
            {
                field = plan->monthlyData().isValid() ? plan->monthlyData().hasTax() :
                                                        plan->yearlyData().hasTax();
                break;
            }
            case UpsellPlans::CURRENT_PLAN_ROLE:
            {
                field = plan->proLevel() == Preferences::instance()->accountType() ||
                        (Preferences::instance()->accountType() ==
                             Preferences::AccountType::ACCOUNT_TYPE_PROIII &&
                         plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI);
                break;
            }
            case UpsellPlans::AVAILABLE_ROLE:
            {
                field = isAvailable(plan);
                break;
            }
            case UpsellPlans::SHOW_PRO_FLEXI_MESSAGE:
            {
                field = plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PROIII ||
                        plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI;
                break;
            }
            case UpsellPlans::SHOW_ONLY_PRO_FLEXI:
            {
                field = isOnlyProFlexiAvailable(plan) ||
                        (plan->proLevel() == Preferences::AccountType::ACCOUNT_TYPE_PRO_FLEXI &&
                         Preferences::instance()->accountType() ==
                             Preferences::AccountType::ACCOUNT_TYPE_PROIII);
                break;
            }
            case UpsellPlans::HAS_DISCOUNT:
            {
                auto discount = mPlans->getPlanDiscount(plan);
                field = discount.has_value();
                break;
            }
            case UpsellPlans::DISCOUNT_MONTHS:
            {
                auto discount = mPlans->getPlanDiscount(plan);
                field = discount.has_value() ? discount->months : 0;
                break;
            }
            case UpsellPlans::DISCOUNT_PERCENTAGE:
            {
                auto discount = mPlans->getPlanDiscount(plan);
                if (discount.has_value())
                {
                    auto dp = discount->percentage;
                    // If the pro level has no monthly plan and we are in yearly view, return the
                    // discount percentage without calculation.
                    field = (mPlans->isMonthly() || !plan->monthlyData().isValid()) ?
                                dp :
                                // dp + ((2 * (100 - dp)) / 12); // Exact formula, do not use for
                                // now because the webclient uses the following:
                                // softCeil(1-(1-16%)(1-dp%))
                                Utilities::softCeil(100 - 0.84 * (100 - dp));
                }
                else
                {
                    field = 0;
                }
                break;
            }
            case UpsellPlans::IS_HIGHLIGHTED:
            {
                const bool anyPlanHasDiscount = mPlans->hasDiscounts();
                auto discount = mPlans->getPlanDiscount(plan);
                field = (discount.has_value() || (plan->isRecommended() && !anyPlanHasDiscount));

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

    mPlans->setIsAnyPlanClicked(true);

    auto periodInMonths = mPlans->isMonthly() ? MONTH_PERIOD : YEAR_PERIOD;

    Utilities::openUrl(ServiceUrls::instance()->getUpsellPlanUrl(plan->proLevel(), periodInMonths));
}

void UpsellController::setBilledPeriod(bool isMonthly)
{
    mPlans->setMonthly(isMonthly);
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

        mPlans->setIsAnyPlanClicked(false);

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
    double amountPlanNeeded(0.);
    for (const auto& plan: mPlans->plans())
    {
        if (usedStorage < plan->monthlyData().gBStorage())
        {
            double currentAmountMonth(plan->monthlyData().priceAfterTax());
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

void UpsellController::onBilledPeriodChanged()
{
    updatePlans();

    emit dataChanged(0, mPlans->size() - 1);
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
    mPlans->setTransferRemainingTime(Utilities::getTimeString(remainingTime, true));
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

    if (mProFlexiTrick)
    {
        reviewPlansToCheckProFlexi(plans);
    }

    emit beginInsertRows(0, plans.size() - 1);

    mPlans->addPlans(plans);
    updatePlans();

    emit endInsertRows();
}

void UpsellController::process(mega::MegaCurrency* currency)
{
    QString localCurrencyName;

    // If Local currency symbol is empty, use billing currency as local currency.
    QString localCurrencySymbol = QString::fromLatin1(currency->getLocalCurrencySymbol());
    const auto localCurrencyIsBillingCurrency = localCurrencySymbol.isEmpty();

    if (localCurrencyIsBillingCurrency)
    {
        // Billing currency symbol is utf-8 encoded
        localCurrencySymbol = QString::fromUtf8(currency->getCurrencySymbol());
        localCurrencyName = QString::fromUtf8(currency->getCurrencyName());
    }
    else
    {
        // Local currency symbol is represented as an escaped unicode sequence ("\uXXXX")
        localCurrencySymbol = Utilities::decodeUnicodeEscapes(localCurrencySymbol);
        localCurrencyName = QString::fromUtf8(currency->getLocalCurrencyName());
    }

    mPlans->setCurrency(localCurrencySymbol, localCurrencyName);
    mPlans->setBillingCurrency(localCurrencyIsBillingCurrency);
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
        const double priceAfterTax(mPlans->isBillingCurrency() ?
                                       pricing->getAmountWithDecimals(i) :
                                       pricing->getLocalPriceWithDecimals(i));
        const double priceBeforeTax(mPlans->isBillingCurrency() ?
                                        pricing->getPriceNetWithDecimals(i) :
                                        pricing->getLocalPriceNetWithDecimals(i));
        auto planData(createAccountBillingPlanData(pricing->getGBStorage(i),
                                                   pricing->getGBTransfer(i),
                                                   priceAfterTax,
                                                   priceBeforeTax));
        if (pricing->getMonths(i) == MONTH_PERIOD)
        {
            plan->setMonthlyData(planData);
        }
        else if (pricing->getMonths(i) == YEAR_PERIOD)
        {
            plan->setYearlyData(planData);
        }
        if (pricing->hasDiscount(i))
        {
            UpsellPlans::Data::DiscountInfo discount{QString::fromUtf8(pricing->getDiscountCode(i)),
                                                     pricing->getDiscountMonths(i),
                                                     pricing->getDiscountPercentage(i)};
            discount.months == MONTH_PERIOD ? plan->monthlyData().setDiscount(discount) :
                                              plan->yearlyData().setDiscount(discount);
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

QString UpsellController::getLocalePriceString(double price) const
{
    return Utilities::toPrice(price, mPlans->getCurrencySymbol(), !mPlans->isBillingCurrency());
}

UpsellPlans::Data::AccountBillingPlanData
    UpsellController::createAccountBillingPlanData(int storage,
                                                   int transfer,
                                                   double priceAfterTax,
                                                   double priceBeforeTax) const
{
    // When the before tax is zero, there's no tax and we use the after tax price
    if (std::abs(priceBeforeTax) < DOUBLE_COMPARISON_EPSILON)
    {
        priceBeforeTax = priceAfterTax;
    }
    UpsellPlans::Data::AccountBillingPlanData planData(storage * NB_B_IN_1GB,
                                                       transfer * NB_B_IN_1GB,
                                                       priceAfterTax / CENTS_IN_1_UNIT,
                                                       priceBeforeTax / CENTS_IN_1_UNIT);
    return planData;
}

int UpsellController::calculateDiscount(double monthlyPrice, double yearlyPrice) const
{
    return static_cast<int>(PERCENTAGE -
                            (yearlyPrice * PERCENTAGE) / (monthlyPrice * NUM_MONTHS_PER_PLAN));
}

void UpsellController::updatePlans()
{
    int currentRecommendedRow(getRowForCurrentRecommended());
    int row(getRowForNextRecommendedPlan());
    if (row > -1 && currentRecommendedRow != row)
    {
        resetRecommended();

        auto plan(mPlans->getPlan(row));
        plan->setRecommended(true);
        mPlans->setCurrentDiscount(mPlans->getMaximumYearlyDiscount());
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
        discount = calculateDiscount(data->monthlyData().priceAfterTax(),
                                     data->yearlyData().priceAfterTax());
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
    return (data->proLevel() != Preferences::instance()->accountType() &&
            !isPlanUnderCurrentProLevel(data->proLevel())) &&
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

double UpsellController::calculateTotalPriceWithoutDiscount(double monthlyPrice) const
{
    return monthlyPrice * NUM_MONTHS_PER_PLAN;
}

double UpsellController::calculateMonthlyPriceWithDiscount(double yearlyPrice) const
{
    return yearlyPrice / NUM_MONTHS_PER_PLAN;
}

bool UpsellController::isOnlyProFlexiAvailable(const std::shared_ptr<UpsellPlans::Data>& data) const
{
    return !storageFitsUnderStorageOQConditions(data);
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
    auto it =
        std::find_if(plans.cbegin(),
                     plans.cend(),
                     [this](const auto& plan)
                     {
                         bool isOnlyProFlexi(isOnlyProFlexiAvailable(plan));
                         return plan->proLevel() == mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII &&
                                isOnlyProFlexi;
                     });

    return it != plans.cend();
}

void UpsellController::reviewPlansToCheckProFlexi(
    const QList<std::shared_ptr<UpsellPlans::Data>>& plans)
{
    int currentAccountType(Preferences::instance()->accountType());
    if (currentAccountType == mega::MegaAccountDetails::ACCOUNT_TYPE_PROIII ||
        isOnlyProFlexiAvailable(plans))
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
