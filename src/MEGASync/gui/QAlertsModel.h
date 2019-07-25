#ifndef QALERTSMODEL_H
#define QALERTSMODEL_H

#include <QCache>
#include <megaapi.h>
#include <deque>
#include "AlertItem.h"
#include <QAbstractItemModel>

class QAlertsModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QAlertsModel(mega::MegaUserAlertList* alerts, QObject *parent = 0);
    virtual ~QAlertsModel();

    void refreshAlerts();

    QModelIndex index(int row, int column,
                      const QModelIndex &parent = QModelIndex()) const override;
    QModelIndex parent(const QModelIndex &index) const override;

    int columnCount(const QModelIndex &parent = QModelIndex()) const override;
    int rowCount(const QModelIndex &parent) const override;

    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const override;

    QCache<int, AlertItem> alertItems;

private:
    QMap<int, mega::MegaUserAlert*> alertsMap;
    std::deque<int> alertOrder;
};

#endif // QALERTSMODEL_H
