#ifndef QFILTERALERTSMODEL_H
#define QFILTERALERTSMODEL_H

#include <QSortFilterProxyModel>

class QFilterAlertsModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    enum class FilterType
    {
        ALL = 0,
        CONTACTS,
        SHARES,
        PAYMENTS,
        TAKEDOWNS
    };

    QFilterAlertsModel(QObject *parent = 0);
    virtual ~QFilterAlertsModel() = default;

    FilterType filterAlertType();
    void setFilterAlertType(FilterType filterType);
    bool checkFilterType(int sdkType) const;

protected:
    bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const override;

private:
    FilterType actualFilter;

};

#endif // QFILTERALERTSMODEL_H
