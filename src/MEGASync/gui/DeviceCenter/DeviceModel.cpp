#include "DeviceModel.h"

#include <QCoreApplication>
#include <QDate>
#include <Utilities.h>

DeviceModel::DeviceModel(QObject* parent):
    QAbstractListModel(parent)
{}

QHash<int, QByteArray> DeviceModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {NAME, "name"}
    };
    return roles;
}

void DeviceModel::reset(const QString& deviceId, const DeviceData& data)
{
    mObjects.clear();
    beginInsertRows(QModelIndex(), 0, 0);
    mObjects.append(qMakePair(deviceId, data));
    endInsertRows();
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mObjects.size();
}

int DeviceModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 1;
}

QVariant DeviceModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    const auto row = index.row();
    const auto column = index.column();
    if (row >= rowCount() || column >= columnCount())
    {
        return result;
    }

    switch (role)
    {
        case NAME:
            result = getName(row);
            break;

        default:
            break;
    }
    return result;
}

QString DeviceModel::getName(int row) const
{
    return mObjects[row].second.name;
}
