#include "NotificationModel.h"

#include <QDateTime>

namespace
{
constexpr int MAX_COST = 16;
}

NotificationModel::NotificationModel(QObject *parent)
    : QAbstractItemModel(parent)
{
    notificationItems.setMaxCost(MAX_COST);

    beginInsertRows(QModelIndex(), 0, 0);
    NotifTest* notif = new NotifTest();
    notif->id = 0;
    notif->title = "Get 50% off a Pro I plan";
    notif->description = "To celebrate Data Privacy Day, we’re offering 50% off when you upgrade to a 3-year Pro I plan. This deal is valid until 6th February 2023";
    notif->imageName = "PromoImage.png";
    notif->iconName = "";//"PromoIcon.png";
    notif->imagePath = "C:\\Users\\mega\\Pictures\\NotificationCenter\\";
    notif->start = 0;
    notif->end = 0;
    notif->showBanner = false;
    notif->callToAction1 = {
        { "link", "https://mega.nz/" },
        { "text", "Grab deal" }
    };
    notif->callToAction2 = {};
    mNotifMap.insert(notif->id, notif);
    endInsertRows();
}

NotificationModel::~NotificationModel()
{
}

QModelIndex NotificationModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    return createIndex(row, column, mNotifMap.value(row));
}

QModelIndex NotificationModel::parent(const QModelIndex&) const
{
    return QModelIndex();
}

int NotificationModel::columnCount(const QModelIndex&) const
{
    return 1;
}

int NotificationModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return mNotifMap.size();
}

QVariant NotificationModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || index.row() < 0)
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(index.internalId());
    }

    return QVariant();
}
