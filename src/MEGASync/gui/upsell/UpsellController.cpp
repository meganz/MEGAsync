#include "UpsellController.h"

#include "UpsellPlans.h"
#include "Utilities.h"

UpsellController::UpsellController(QObject* parent):
    QObject(parent),
    mPlans(std::make_shared<UpsellPlans>())
{
    connect(mPlans.get(),
            &UpsellPlans::monthlyChanged,
            this,
            &UpsellController::onBilledPeriodChanged);
}

void UpsellController::init()
{
    emit beginInsertRows(0, 3);

    mPlans->addPlan(std::make_shared<UpsellPlans::Data>(
        1,
        true,
        UpsellPlans::Data::AccountBillingPlanData(2048, 2048, 999),
        UpsellPlans::Data::AccountBillingPlanData(2048, 24576, 9999)));
    mPlans->addPlan(std::make_shared<UpsellPlans::Data>(
        2,
        false,
        UpsellPlans::Data::AccountBillingPlanData(8192, 8192, 1999),
        UpsellPlans::Data::AccountBillingPlanData(8192, 98304, 19999)));
    mPlans->addPlan(std::make_shared<UpsellPlans::Data>(
        3,
        false,
        UpsellPlans::Data::AccountBillingPlanData(16384, 16384, 2999),
        UpsellPlans::Data::AccountBillingPlanData(16384, 196608, 29999)));

    emit endInsertRows();

    mPlans->setCurrencySymbol(QString::fromUtf8("$"));
}

bool UpsellController::setData(int row, const QVariant& value, int role)
{
    return setData(mPlans->getPlan(row), value, role);
}

bool UpsellController::setData(std::shared_ptr<UpsellPlans::Data> data, QVariant value, int role)
{
    return true;
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

void UpsellController::onBilledPeriodChanged()
{
    QVector<int> roles;
    roles << UpsellPlans::STORAGE_ROLE << UpsellPlans::TRANSFER_ROLE << UpsellPlans::PRICE_ROLE;
    emit dataChanged(0, mPlans->size() - 1, roles);
}
