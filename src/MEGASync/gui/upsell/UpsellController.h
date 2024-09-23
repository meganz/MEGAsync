#ifndef UPSELL_CONTROLLER_H
#define UPSELL_CONTROLLER_H

#include "UpsellPlans.h"

#include <QObject>
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

    std::shared_ptr<UpsellPlans> getPlans() const;
    void openPlan(int row);

public slots:
    void onBilledPeriodChanged();

signals:
    void beginInsertRows(int first, int last);
    void endInsertRows();
    void beginRemoveRows(int first, int last);
    void endRemoveRows();
    void dataChanged(int rowStart, int rowFinal, QVector<int> roles);

private:
    std::shared_ptr<mega::QTMegaRequestListener> mDelegateListener;
    std::shared_ptr<UpsellPlans> mPlans;

    void processGetPricingRequest(mega::MegaPricing* pricing, mega::MegaCurrency* currency);
    void process(mega::MegaPricing* pricing);
    void process(mega::MegaCurrency* currency);
    int countNumPlans(mega::MegaPricing* pricin) const;
    bool isProLevelValid(int proLevel) const;
    QUrl getUpsellPlanUrl(int proLevel);
};

#endif // UPSELL_CONTROLLER_H
