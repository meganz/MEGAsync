#ifndef UPSELL_CONTROLLER_H
#define UPSELL_CONTROLLER_H

#include "UpsellPlans.h"
#include "Utilities.h"

#include <QObject>
#include <QTimer>
#include <QVariant>
#include <QVector>

#include <memory>

namespace mega
{
class MegaRequest;
class MegaError;
class MegaPricing;
class MegaCurrency;
}

class UpsellController: public QObject, public IStorageObserver
{
    Q_OBJECT

public:
    UpsellController(QObject* parent = nullptr);
    virtual ~UpsellController();

    void updateStorageElements() override;

    void onRequestFinish(mega::MegaRequest* request, mega::MegaError* error);

    void registerQmlRootContextProperties();

    void requestPricingData();

    QVariant data(int row, int role) const;
    QVariant data(std::shared_ptr<UpsellPlans::Data> data, int role) const;

    void openPlanUrl(int index);

    std::shared_ptr<UpsellPlans> getPlans() const;
    QString getMinProPlanNeeded(long long usedStorage) const;
    UpsellPlans::ViewMode viewMode() const;

    void setBilledPeriod(bool isMonthly);
    void setViewMode(UpsellPlans::ViewMode mode);
    void setTransferFinishTime(long long finishTime);

    QString getMinProPlanNeeded(long long usedStorage) const;

public slots:
    void onBilledPeriodChanged();

signals:
    void dataReady();
    void beginInsertRows(int first, int last);
    void endInsertRows();
    void dataChanged(int rowStart, int rowFinal, QVector<int> roles);

private slots:
    void onTransferRemainingTimeElapsed();

private:
    std::shared_ptr<UpsellPlans> mPlans;
    QTimer* mTransferFinishTimer;

    void processGetPricingRequest(mega::MegaPricing* pricing, mega::MegaCurrency* currency);
    void process(mega::MegaPricing* pricing);
    void process(mega::MegaCurrency* currency);
    QList<std::shared_ptr<UpsellPlans::Data>> getAllowedPlans(mega::MegaPricing* pricing);
    std::shared_ptr<UpsellPlans::Data> appendPlan(int proLevel,
                                                  QList<std::shared_ptr<UpsellPlans::Data>>& plans);
    bool isProLevelValid(int proLevel) const;
    QUrl getUpsellPlanUrl(int proLevel);
    QString getLocalePriceString(float price) const;
    UpsellPlans::Data::AccountBillingPlanData createAccountBillingPlanData(int storage,
                                                                           int transfer,
                                                                           int price) const;
    int calculateDiscount(float monthlyPrice, float yearlyPrice) const;
    int getRowForNextRecommendedPlan() const;
    int getRowForCurrentRecommended();
    void updatePlans();
    void updatePlansAt(const std::shared_ptr<UpsellPlans::Data>& data, int row);
    void resetRecommended();
    bool isAvailable(const std::shared_ptr<UpsellPlans::Data>& data) const;
    bool isPlanUnderCurrentProLevel(int proLevel) const;
    bool planFitsUnderStorageOQConditions(int64_t planGbStorage) const;
    float calculateTotalPriceWithoutDiscount(float monthlyPrice) const;
    float calculateMonthlyPriceWithDiscount(float yearlyPrice) const;
    bool isOnlyProFlexiIsAvailable(const std::shared_ptr<UpsellPlans::Data>& data) const;
    bool storageFitsUnderStorageOQConditions(const std::shared_ptr<UpsellPlans::Data>& data) const;
};

#endif // UPSELL_CONTROLLER_H
