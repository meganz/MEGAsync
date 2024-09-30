#include "UpsellPlans.h"

#include "QmlManager.h"

// ************************************************************************************************
// * UpsellPlans
// ************************************************************************************************

UpsellPlans::UpsellPlans(QObject* parent):
    QObject(parent),
    mViewMode(ViewMode::NONE),
    mIsMonthly(true),
    mIsBillingCurrency(true),
    mCurrentPlanSelected(-1),
    mCurrentDiscount(-1),
    mTransferFinishTime(0ll)
{
    QmlManager::instance()->setRootContextProperty(this);
}

bool UpsellPlans::addPlan(std::shared_ptr<Data> plan)
{
    if (!plan || std::any_of(mPlans.begin(),
                             mPlans.end(),
                             [&plan](const std::shared_ptr<Data>& existingPlan)
                             {
                                 return existingPlan->proLevel() == plan->proLevel();
                             }))
    {
        return false;
    }
    mPlans.append(plan);
    return true;
}

std::shared_ptr<UpsellPlans::Data> UpsellPlans::getPlan(int index) const
{
    return mPlans.at(index);
}

std::shared_ptr<UpsellPlans::Data> UpsellPlans::getPlanByProLevel(int proLevel) const
{
    auto it = std::find_if(mPlans.begin(),
                           mPlans.end(),
                           [proLevel](const std::shared_ptr<Data>& plan)
                           {
                               return plan->proLevel() == proLevel;
                           });
    return it != mPlans.end() ? *it : nullptr;
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

QString UpsellPlans::getCurrentPlanName() const
{
    return mCurrentPlanName;
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

long long UpsellPlans::getTransferFinishTime() const
{
    return mTransferFinishTime;
}

bool UpsellPlans::isMonthly() const
{
    return mIsMonthly;
}

void UpsellPlans::setMonthly(bool monthly)
{
    if (mIsMonthly != monthly)
    {
        mIsMonthly = monthly;
        emit monthlyChanged();
    }
}

void UpsellPlans::setCurrentPlanSelected(int row)
{
    if (mCurrentPlanSelected != row)
    {
        mCurrentPlanSelected = row;
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

void UpsellPlans::setCurrentPlanName(const QString& name)
{
    if (mCurrentPlanName != name)
    {
        mCurrentPlanName = name;
        emit currentPlanNameChanged();
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

int UpsellPlans::getCurrentPlanSelected() const
{
    return mCurrentPlanSelected;
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

// ************************************************************************************************
// * UpsellPlans::Data
// ************************************************************************************************

UpsellPlans::Data::Data(int proLevel, const QString& name):
    mProLevel(proLevel),
    mRecommended(false),
    mSelected(false),
    mName(name)
{}

QHash<int, QByteArray> UpsellPlans::Data::roleNames()
{
    static QHash<int, QByteArray> roles{
        {UpsellPlans::NAME_ROLE,        "name"       },
        {UpsellPlans::RECOMMENDED_ROLE, "recommended"},
        {UpsellPlans::STORAGE_ROLE,     "gbStorage"  },
        {UpsellPlans::TRANSFER_ROLE,    "gbTransfer" },
        {UpsellPlans::PRICE_ROLE,       "price"      },
        {UpsellPlans::SELECTED_ROLE,    "selected"   },
        {UpsellPlans::AVAILABLE_ROLE,   "available"  }
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

const UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::monthlyData() const
{
    return mMonthlyData;
}

const UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::yearlyData() const
{
    return mYearlyData;
}

void UpsellPlans::Data::setYearlyData(const AccountBillingPlanData& newYearlyData)
{
    mYearlyData = newYearlyData;
}

bool UpsellPlans::Data::selected() const
{
    return mSelected;
}

void UpsellPlans::Data::setSelected(bool newChecked)
{
    mSelected = newChecked;
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
    mPrice(-1.0f)
{}

UpsellPlans::Data::AccountBillingPlanData::AccountBillingPlanData(int64_t gbStorage,
                                                                  int64_t gbTransfer,
                                                                  float price):
    mGBStorage(gbStorage),
    mGBTransfer(gbTransfer),
    mPrice(price)
{}

bool UpsellPlans::Data::AccountBillingPlanData::isValid() const
{
    return mGBStorage != -1 && mGBTransfer != -1 && mPrice != -1.0f;
}

int64_t UpsellPlans::Data::AccountBillingPlanData::gBStorage() const
{
    return mGBStorage;
}

int64_t UpsellPlans::Data::AccountBillingPlanData::gBTransfer() const
{
    return mGBTransfer;
}

float UpsellPlans::Data::AccountBillingPlanData::price() const
{
    return mPrice;
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
