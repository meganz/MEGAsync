#include "SyncModel.h"

#include <QCoreApplication>
#include <QDate>

SyncModel::SyncModel(QObject* parent):
    QAbstractListModel(parent)
{}

QHash<int, QByteArray> SyncModel::roleNames() const
{
    static QHash<int, QByteArray> roles{
        {TYPE,          "type"        },
        {NAME,          "name"        },
        {SIZE,          "size"        },
        {DATE_ADDED,    "dateAdded"   },
        {DATE_MODIFIED, "dateModified"}
    };
    return roles;
}

int SyncModel::rowCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

int SyncModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent);
    return 5;
}

QVariant SyncModel::data(const QModelIndex& index, int role) const
{
    QVariant result;
    const auto row = index.row();
    const auto coulumn = index.column();
    if (row >= rowCount() || coulumn >= columnCount())
    {
        return result;
    }

    switch (role)
    {
        case NAME:
            result = getName(row);
            break;
        case TYPE:
            result = getType(row);
            break;
        case SIZE:
            result = getSize(row);
            break;
        case DATE_ADDED:
            result = getDateAdded(row);
            break;
        case DATE_MODIFIED:
            result = getDateModified(row);
            break;
        default:
            break;
    }
    return result;
}

QString SyncModel::getName(int row) const
{
    return QString::fromUtf8("folder");
}

int SyncModel::getSize(int row) const
{
    return 5;
}

QString SyncModel::getType(int row) const
{
    return QString::fromUtf8("Sync");
}

QDate SyncModel::getDateAdded(int row) const
{
    return QDate::currentDate();
}

QDate SyncModel::getDateModified(int row) const
{
    return QDate::currentDate();
}
