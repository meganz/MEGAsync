#ifndef UPSELL_CONTROLLER_H
#define UPSELL_CONTROLLER_H

#include "UpsellPlans.h"

#include <QObject>
#include <QVariant>
#include <QVector>

#include <memory>

class UpsellController: public QObject
{
    Q_OBJECT

public:
    UpsellController(QObject* parent = nullptr);
    virtual ~UpsellController() = default;

    void init();

    bool setData(int row, const QVariant& value, int role);
    bool setData(std::shared_ptr<UpsellPlans::Data> data, QVariant value, int role);
    QVariant data(int row, int role) const;
    QVariant data(std::shared_ptr<UpsellPlans::Data> data, int role) const;

    std::shared_ptr<UpsellPlans> getPlans() const;

public slots:
    void onBilledPeriodChanged();

signals:
    void beginInsertRows(int first, int last);
    void endInsertRows();
    void beginRemoveRows(int first, int last);
    void endRemoveRows();
    void dataChanged(int rowStart, int rowFinal, QVector<int> roles);

private:
    std::shared_ptr<UpsellPlans> mPlans;
};

#endif // UPSELL_CONTROLLER_H
