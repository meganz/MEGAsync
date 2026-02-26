#include "UpsellPlans.h"

// ************************************************************************************************
// * UpsellPlans
// ************************************************************************************************
namespace
{
constexpr int MONTH_PERIOD(1);
constexpr int YEAR_PERIOD(12);
}

UpsellPlans::UpsellPlans(QObject* parent):
    QObject(parent),
    mViewMode(ViewMode::NONE),
    mIsMonthly(false),
    mIsBillingCurrency(true),
    mCurrentDiscount(-1),
    mTransferFinishTime(0ll),
    mIsOnlyProFlexiAvailable(false),
    mIsPro(false),
    mIsAnyPlanClicked(false)
{}

void UpsellPlans::addPlans(const QList<std::shared_ptr<Data>>& plans)
{
    if (plans.isEmpty())
    {
        return;
    }

    mPlans.append(plans);
    // Switch to monthly view if there are no discount on the yearly plan and a discount on the
    // monthly plan.
    bool isMonthly = false;
    if (!hasYearlyDiscount() && hasMonthlyDiscount())
    {
        isMonthly = true;
    }
    setMonthly(isMonthly);
    emit sizeChanged();
}

std::shared_ptr<UpsellPlans::Data> UpsellPlans::getPlan(int index) const
{
    if (index < 0 || index >= mPlans.size())
    {
        return nullptr;
    }

    return mPlans.at(index);
}

int UpsellPlans::size() const
{
    return mPlans.size();
}

UpsellPlans::ViewMode UpsellPlans::getViewMode() const
{
    return mViewMode;
}

QString UpsellPlans::getCurrencySymbol() const
{
    return mCurrency.currencySymbol();
}

QString UpsellPlans::getCurrencyName() const
{
    return mCurrency.currencyName();
}

QString UpsellPlans::getTransferRemainingTime() const
{
    return mTransferRemainingTime;
}

void UpsellPlans::setCurrency(const QString& symbol, const QString& name)
{
    mCurrency.setCurrencySymbol(symbol);
    mCurrency.setCurrencyName(name);
    emit currencyChanged();
}

void UpsellPlans::setTransferFinishTime(long long newTime)
{
    mTransferFinishTime = newTime;
}

void UpsellPlans::setOnlyProFlexiAvailable(bool onlyProFlexiAvailable)
{
    if (mIsOnlyProFlexiAvailable != onlyProFlexiAvailable)
    {
        mIsOnlyProFlexiAvailable = onlyProFlexiAvailable;
        emit onlyProFlexiAvailableChanged();
    }
}

void UpsellPlans::setPro(bool isPro)
{
    if (mIsPro != isPro)
    {
        mIsPro = isPro;
        emit isProChanged();
    }
}

void UpsellPlans::setIsAnyPlanClicked(bool isAnyPlanClicked)
{
    mIsAnyPlanClicked = isAnyPlanClicked;
}

bool UpsellPlans::isAnyPlanClicked() const
{
    return mIsAnyPlanClicked;
}

long long UpsellPlans::getTransferFinishTime() const
{
    return mTransferFinishTime;
}

bool UpsellPlans::isOnlyProFlexiAvailable() const
{
    return mIsOnlyProFlexiAvailable;
}

bool UpsellPlans::isPro() const
{
    return mIsPro;
}

bool UpsellPlans::isMonthly() const
{
    return mIsMonthly;
}

std::optional<UpsellPlans::Data::DiscountInfo>
    UpsellPlans::getPlanDiscount(std::shared_ptr<UpsellPlans::Data> plan) const
{
    return mIsMonthly ? plan->monthlyData().discount() : plan->yearlyData().discount();
}

void UpsellPlans::setMonthly(bool monthly)
{
    if (mIsMonthly != monthly)
    {
        mIsMonthly = monthly;
        emit monthlyChanged();
    }
}

void UpsellPlans::setBillingCurrency(bool isCurrencyBilling)
{
    if (mIsBillingCurrency != isCurrencyBilling)
    {
        mIsBillingCurrency = isCurrencyBilling;
        emit isCurrencyBillingChanged();
    }
}

void UpsellPlans::setCurrentDiscount(int discount)
{
    if (mCurrentDiscount != discount)
    {
        mCurrentDiscount = discount;
        emit currentDiscountChanged();
    }
}

void UpsellPlans::setTransferRemainingTime(const QString& time)
{
    if (mTransferRemainingTime != time)
    {
        mTransferRemainingTime = time;
        emit remainingTimeChanged();
    }
}

void UpsellPlans::setViewMode(ViewMode viewMode)
{
    if (mViewMode != viewMode)
    {
        mViewMode = viewMode;
        emit viewModeChanged();
    }
}

bool UpsellPlans::isBillingCurrency() const
{
    return mIsBillingCurrency;
}

int UpsellPlans::getCurrentDiscount() const
{
    return mCurrentDiscount;
}

QList<std::shared_ptr<UpsellPlans::Data>> UpsellPlans::plans() const
{
    return mPlans;
}

bool UpsellPlans::hasDiscounts() const
{
    return std::any_of(mPlans.begin(),
                       mPlans.end(),
                       [this](std::shared_ptr<Data> plan)
                       {
                           return mIsMonthly ? plan->hasMonthlyDiscount() :
                                               plan->hasYearlyDiscount();
                       });
}

bool UpsellPlans::hasMonthlyDiscount() const
{
    return std::any_of(mPlans.begin(),
                       mPlans.end(),
                       [](std::shared_ptr<Data> plan)
                       {
                           return plan->hasMonthlyDiscount();
                       });
}

bool UpsellPlans::hasYearlyDiscount() const
{
    return std::any_of(mPlans.begin(),
                       mPlans.end(),
                       [](std::shared_ptr<Data> plan)
                       {
                           return plan->hasYearlyDiscount();
                       });
}

int UpsellPlans::getMaximumYearlyDiscount() const
{
    if (mPlans.empty())
        return 0; // or throw, depending on your design

    auto it =
        std::max_element(mPlans.begin(),
                         mPlans.end(),
                         [](const std::shared_ptr<Data>& a, const std::shared_ptr<Data>& b)
                         {
                             return a->calculateYearlyDiscount() < b->calculateYearlyDiscount();
                         });

    return (*it)->calculateYearlyDiscount();
}

// ************************************************************************************************
// * UpsellPlans::Data
// ************************************************************************************************

UpsellPlans::Data::Data(int proLevel, const QString& name):
    mProLevel(proLevel),
    mRecommended(false),
    mName(name)
{}

QHash<int, QByteArray> UpsellPlans::Data::roleNames()
{
    static QHash<int, QByteArray> roles{
        {Qt::DisplayRole, "display"},
        {UpsellPlans::NAME_ROLE, "name"},
        {UpsellPlans::BUTTON_NAME_ROLE, "buttonName"},
        {UpsellPlans::RECOMMENDED_ROLE, "recommended"},
        {UpsellPlans::STORAGE_ROLE, "gbStorage"},
        {UpsellPlans::TRANSFER_ROLE, "gbTransfer"},
        {UpsellPlans::PRICE_AFTER_TAX_ROLE, "priceAfterTax"},
        {UpsellPlans::TOTAL_PRICE_WITHOUT_DISCOUNT_ROLE, "totalPriceWithoutDiscount"},
        {UpsellPlans::MONTHLY_PRICE_WITH_DISCOUNT_ROLE, "monthlyPriceWithDiscount"},
        {UpsellPlans::CURRENT_PLAN_ROLE, "currentPlan"},
        {UpsellPlans::AVAILABLE_ROLE, "available"},
        {UpsellPlans::SHOW_PRO_FLEXI_MESSAGE, "showProFlexiMessage"},
        {UpsellPlans::SHOW_ONLY_PRO_FLEXI, "showOnlyProFlexi"},
        {UpsellPlans::HAS_DISCOUNT, "hasDiscount"},
        {UpsellPlans::DISCOUNT_MONTHS, "discountMonths"},
        {UpsellPlans::DISCOUNT_PERCENTAGE, "discountPercentage"},
        {UpsellPlans::IS_HIGHLIGHTED, "isHighlighted"},
        {UpsellPlans::PRICE_BEFORE_TAX_ROLE, "priceBeforeTax"},
        {UpsellPlans::MONTHLY_BASE_PRICE_ROLE, "monthlyBasePrice"}

    };

    return roles;
}

int UpsellPlans::Data::proLevel() const
{
    return mProLevel;
}

bool UpsellPlans::Data::isRecommended() const
{
    return mRecommended;
}

const QString& UpsellPlans::Data::name() const
{
    return mName;
}

UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::monthlyData()
{
    return mMonthlyData;
}

UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::yearlyData()
{
    return mYearlyData;
}

bool UpsellPlans::Data::hasMonthlyDiscount() const
{
    return mMonthlyData.hasDiscount();
}

bool UpsellPlans::Data::hasYearlyDiscount() const
{
    return mYearlyData.hasDiscount();
}

int UpsellPlans::Data::calculateYearlyDiscount() const
{
    if (mYearlyData.hasDiscount())
    {
        int dp = mYearlyData.discount()->percentage;
        return mMonthlyData.isValid() ? dp + ((2 * (100 - dp)) / 12) : dp;
    }
    else if (mMonthlyData.isValid())
    {
        constexpr float NUM_MONTHS_PER_PLAN(12.0f);
        constexpr float PERCENTAGE(100.0f);

        return static_cast<int>(PERCENTAGE -
                                (mYearlyData.priceAfterTax() * PERCENTAGE) /
                                    (mMonthlyData.priceAfterTax() * NUM_MONTHS_PER_PLAN));
    }
    return 0;
}

void UpsellPlans::Data::setProLevel(int newProLevel)
{
    mProLevel = newProLevel;
}

void UpsellPlans::Data::setYearlyData(const AccountBillingPlanData& newYearlyData)
{
    mYearlyData = newYearlyData;
}

void UpsellPlans::Data::setName(const QString& name)
{
    mName = name;
}

void UpsellPlans::Data::setRecommended(bool newRecommended)
{
    mRecommended = newRecommended;
}

void UpsellPlans::Data::setMonthlyData(const AccountBillingPlanData& newMonthlyData)
{
    mMonthlyData = newMonthlyData;
}

// ************************************************************************************************
// * UpsellPlans::Data::AccountBillingPlanData
// ************************************************************************************************

UpsellPlans::Data::AccountBillingPlanData::AccountBillingPlanData():
    mGBStorage(-1),
    mGBTransfer(-1),
    mPriceAfterTax(-1.0f),
    mPriceBeforeTax(-1.0)
{}

UpsellPlans::Data::AccountBillingPlanData::AccountBillingPlanData(int64_t gbStorage,
                                                                  int64_t gbTransfer,
                                                                  float priceAfterTax,
                                                                  double priceBeforeTax):
    mGBStorage(gbStorage),
    mGBTransfer(gbTransfer),
    mPriceAfterTax(priceAfterTax),
    mPriceBeforeTax(priceBeforeTax)
{}

bool UpsellPlans::Data::AccountBillingPlanData::isValid() const
{
    return mGBStorage != -1 && mGBTransfer != -1 && mPriceAfterTax != -1.0f;
}

int64_t UpsellPlans::Data::AccountBillingPlanData::gBStorage() const
{
    return mGBStorage;
}

int64_t UpsellPlans::Data::AccountBillingPlanData::gBTransfer() const
{
    return mGBTransfer;
}

float UpsellPlans::Data::AccountBillingPlanData::priceAfterTax() const
{
    return mPriceAfterTax;
}

double UpsellPlans::Data::AccountBillingPlanData::priceBeforeTax() const
{
    return mPriceBeforeTax;
}

std::optional<UpsellPlans::Data::DiscountInfo>
    UpsellPlans::Data::AccountBillingPlanData::discount() const
{
    return mDiscount;
}

bool UpsellPlans::Data::AccountBillingPlanData::hasDiscount() const
{
    return mDiscount.has_value();
}

void UpsellPlans::Data::AccountBillingPlanData::setDiscount(
    UpsellPlans::Data::DiscountInfo discount)
{
    mDiscount = discount;
}

// ************************************************************************************************
// * UpsellPlans::CurrencyData
// ************************************************************************************************

QString UpsellPlans::CurrencyData::currencySymbol() const
{
    return mCurrencySymbol;
}

QString UpsellPlans::CurrencyData::currencyName() const
{
    return mCurrencyName;
}

void UpsellPlans::CurrencyData::setCurrencyName(const QString& newCurrencyName)
{
    mCurrencyName = newCurrencyName;
}

void UpsellPlans::CurrencyData::setCurrencySymbol(const QString& newCurrencySymbol)
{
    mCurrencySymbol = newCurrencySymbol;
}
