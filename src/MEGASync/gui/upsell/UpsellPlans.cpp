#include "UpsellPlans.h"

#include "QmlManager.h"

namespace
{
constexpr int64_t NB_B_IN_1GB(1024 * 1024 * 1024);
constexpr float CENTS_IN_1_UNIT(100.0f);
}

// ************************************************************************************************
// * UpsellPlans
// ************************************************************************************************

UpsellPlans::UpsellPlans(QObject* parent):
    QObject(parent)
{
    QmlManager::instance()->setRootContextProperty(this);
}

void UpsellPlans::addPlan(std::shared_ptr<Data> plan)
{
    mPlans.append(plan);
}

std::shared_ptr<UpsellPlans::Data> UpsellPlans::getPlan(int index) const
{
    return mPlans.at(index);
}

int UpsellPlans::size() const
{
    return mPlans.size();
}

QString UpsellPlans::getCurrencySymbol() const
{
    return mCurrencySymbol;
}

void UpsellPlans::setCurrencySymbol(const QString& symbol)
{
    if (mCurrencySymbol != symbol)
    {
        mCurrencySymbol = symbol;
        emit currencySymbolChanged();
    }
}

bool UpsellPlans::isMonthly() const
{
    return mMonthly;
}

void UpsellPlans::setMonthly(bool monthly)
{
    if (mMonthly != monthly)
    {
        mMonthly = monthly;
        emit monthlyChanged();
    }
}

// ************************************************************************************************
// * UpsellPlans::Data
// ************************************************************************************************

UpsellPlans::Data::Data(int proLevel,
                        bool recommended,
                        const AccountBillingPlanData& monthlyData,
                        const AccountBillingPlanData& yearlyData):
    mProLevel(proLevel),
    mRecommended(recommended),
    mMonthlyData(monthlyData),
    mYearlyData(yearlyData)
{}

QHash<int, QByteArray> UpsellPlans::Data::roleNames()
{
    static QHash<int, QByteArray> roles{
        {UpsellPlans::NAME_ROLE,        "name"       },
        {UpsellPlans::RECOMMENDED_ROLE, "recommended"},
        {UpsellPlans::STORAGE_ROLE,     "gbStorage"  },
        {UpsellPlans::TRANSFER_ROLE,    "gbTransfer" },
        {UpsellPlans::PRICE_ROLE,       "price"      }
    };

    return roles;
}

int UpsellPlans::Data::proLevel() const
{
    return mProLevel;
}

bool UpsellPlans::Data::recommended() const
{
    return mRecommended;
}

const UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::monthlyData() const
{
    return mMonthlyData;
}

const UpsellPlans::Data::AccountBillingPlanData& UpsellPlans::Data::yearlyData() const
{
    return mYearlyData;
}

// ************************************************************************************************
// * UpsellPlans::Data::AccountBillingPlanData
// ************************************************************************************************

UpsellPlans::Data::AccountBillingPlanData::AccountBillingPlanData(int gbStorage,
                                                                  int gbTransfer,
                                                                  int price):
    mGBStorage(static_cast<int64_t>(gbStorage) * NB_B_IN_1GB),
    mGBTransfer(static_cast<int64_t>(gbTransfer) * NB_B_IN_1GB),
    mPrice(static_cast<float>(price) / CENTS_IN_1_UNIT)
{}

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
