#ifndef UPSELL_CONTROLLER_H
#define UPSELL_CONTROLLER_H

#include "UpsellPlans.h"

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QVector>

#include <memory>

namespace mega
{
class QTMegaRequestListener;
class MegaRequest;
class MegaError;
class MegaPricing;
class MegaCurrency;
}

class UpsellController: public QObject
{
    Q_OBJECT

public:
    UpsellController(QObject* parent = nullptr);
    virtual ~UpsellController() = default;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

    bool setData(int row, const QVariant& value, int role);
    bool setData(std::shared_ptr<UpsellPlans::Data> data, QVariant value, int role);
    QVariant data(int row, int role) const;
    QVariant data(std::shared_ptr<UpsellPlans::Data> data, int role) const;

    void openSelectedPlanUrl();

    std::shared_ptr<UpsellPlans> getPlans() const;
    QString getMinProPlanNeeded(long long usedStorage) const;

    void setBilledPeriod(bool isMonthly);
    void setViewMode(UpsellPlans::ViewMode mode);
    void setTransferFinishTime(long long finishTime);

public slots:
    void onBilledPeriodChanged();

signals:
    void beginInsertRows(int first, int last);
    void endInsertRows();
    void beginRemoveRows(int first, int last);
    void endRemoveRows();
    void dataChanged(int rowStart, int rowFinal, QVector<int> roles);

private slots:
    void onTransferRemainingTimeElapsed();

private:
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<UpsellPlans> mPlans;
    QTimer* mTransferFinishTimer;

    void processGetPricingRequest(mega::MegaPricing* pricing, mega::MegaCurrency* currency);
    void process(mega::MegaPricing* pricing);
    void process(mega::MegaCurrency* currency);
    int countNumPlans(mega::MegaPricing* pricin) const;
    bool isProLevelValid(int proLevel) const;
    QUrl getUpsellPlanUrl(int proLevel);
    QString getLocalePriceString(float price) const;
    UpsellPlans::Data::AccountBillingPlanData createAccountBillingPlanData(int storage,
                                                                           int transfer,
                                                                           int price) const;
    int calculateDiscount(float monthlyPrice, float yearlyPrice) const;
    void addPlan(mega::MegaPricing* pricing, int index);
    void updatePlans();
    void updatePlansAt(const std::shared_ptr<UpsellPlans::Data>& data, int row);
    int getRowForNextRecommendedPlan() const;
    void resetSelectedAndRecommended();
    int getRowForCurrentRecommended();
};

#endif // UPSELL_CONTROLLER_H
