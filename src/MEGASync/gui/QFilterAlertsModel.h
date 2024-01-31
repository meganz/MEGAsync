#ifndef QFILTERALERTSMODEL_H
#define QFILTERALERTSMODEL_H

#include <QSortFilterProxyModel>

#include <megaapi.h>

class QFilterAlertsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:

    enum {
        FILTER_CONTACTS = 0,
        FILTER_SHARES,
        FILTER_PAYMENT,
        FILTER_TAKEDOWNS,
        NO_FILTER,
    };

    QFilterAlertsModel(QObject *parent = 0);
    virtual ~QFilterAlertsModel();

    int filterAlertType();
    void setFilterAlertType(int filterType);
    bool checkFilterType(int typeToCheck) const;

protected:
    bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const override;

private:
    int actualFilter;
};

#endif // QFILTERALERTSMODEL_H
