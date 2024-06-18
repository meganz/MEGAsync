#ifndef NOTIFICATIONALERTPROXYMODEL_H
#define NOTIFICATIONALERTPROXYMODEL_H

#include <QSortFilterProxyModel>

class NotificationAlertProxyModel : public QSortFilterProxyModel
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

    NotificationAlertProxyModel(QObject *parent = 0);
    virtual ~NotificationAlertProxyModel() = default;

    FilterType filterAlertType();
    void setFilterAlertType(FilterType filterType);
    bool checkFilterType(int sdkType) const;

protected:
    bool filterAcceptsRow(int row, const QModelIndex &sourceParent) const override;
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;

private:
    FilterType actualFilter;

};

#endif // NOTIFICATIONALERTPROXYMODEL_H
