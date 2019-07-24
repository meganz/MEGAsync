#ifndef QFILTERALERTSMODEL_H
#define QFILTERALERTSMODEL_H

#include <QSortFilterProxyModel>
#include "QAlertsModel.h"
#include <megaapi.h>

class QFilterAlertsModel : public QSortFilterProxyModel, public MegaAlertsModel
{
    Q_OBJECT

public:
    using  QSortFilterProxyModel::d_ptr;

    enum {
        FILTER_CONTACTS = 0,
        FILTER_SHARES,
        FILTER_PAYMENT,
        FILTER_TAKEDOWNS,
        NO_FILTER,
    };

    QFilterAlertsModel(QObject *parent = 0);
    virtual ~QFilterAlertsModel();

    bool isProxyModel() const override {return true;}

    int filterAlertType();
    void setFilterAlertType(int filterType);
    bool checkFilterType(int typeToCheck) const;

protected:
    bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const override;

private:
    int actualFilter;
};

#endif // QFILTERALERTSMODEL_H
