#ifndef DEVICEMODEL_H
#define DEVICEMODEL_H

#include "DeviceData.h"

#include <QAbstractListModel>

class DeviceModel: public QAbstractListModel
{
    Q_OBJECT

public:
    enum SyncModelRole
    {
        NAME = Qt::UserRole + 1
    };

    explicit DeviceModel(QObject* parent = nullptr);

    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role = Qt::DisplayRole) const override;
    QHash<int, QByteArray> roleNames() const override;

    void reset(const QString& deviceId, const DeviceData& data);

public:
    QString getName(int row) const;

    QList<QPair<QString, DeviceData>> mObjects;
};

#endif // DEVICEMODEL_H
