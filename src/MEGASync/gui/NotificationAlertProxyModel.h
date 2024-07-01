#ifndef NOTIFICATIONALERTPROXYMODEL_H
#define NOTIFICATIONALERTPROXYMODEL_H

#include "NotificationAlertTypes.h"

#include <QSortFilterProxyModel>

class NotificationAlertProxyModel : public QSortFilterProxyModel
{
    Q_OBJECT

public:
    NotificationAlertProxyModel(QObject* parent = 0);
    virtual ~NotificationAlertProxyModel() = default;

    AlertType filterAlertType();
    void setFilterAlertType(AlertType filterType);

protected:
    bool filterAcceptsRow(int row, const QModelIndex& sourceParent) const override;
    bool lessThan(const QModelIndex& left, const QModelIndex& right) const override;

private:
    AlertType mActualFilter;

    bool checkAlertFilterType(int sdkType) const;

};

#endif // NOTIFICATIONALERTPROXYMODEL_H
