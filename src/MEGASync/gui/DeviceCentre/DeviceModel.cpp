#include "DeviceModel.h"

#include <QCoreApplication>
#include <QDate>

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
    beginResetModel();
    mDevicesData.clear();
    mDevicesData.append(qMakePair(deviceId, data));
    endResetModel();
}

int DeviceModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return mDevicesData.size();
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
    return mDevicesData[row].second.name;
}

void DeviceModel::addDeviceName(const QString& deviceID, const QString& name)
{
    mDeviceNames[deviceID] = name;
}

bool DeviceModel::deviceNameAlreadyExists(const QString& name) const
{
    return std::find_if(std::cbegin(mDeviceNames),
                        std::cend(mDeviceNames),
                        [&name](const auto& deviceIDNamepair)
                        {
                            return deviceIDNamepair.second == name;
                        }) != std::cend(mDeviceNames);
}
