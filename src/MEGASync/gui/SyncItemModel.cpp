#include "SyncItemModel.h"

#include <QCoreApplication>
#include <QIcon>
#include <QFileInfo>
#include <QDebug>

#define ICON_SIZE 24

SyncItemModel::SyncItemModel(QObject *parent)
    : QAbstractItemModel(parent),
    mSyncModel (SyncModel::instance())
{
    connect(mSyncModel, &SyncModel::syncStateChanged, this, &SyncItemModel::insertSync);
    connect(mSyncModel, &SyncModel::syncRemoved, this, &SyncItemModel::removeSync);
}

QVariant SyncItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if(orientation == Qt::Horizontal)
    {
        switch(section)
        {
        case Column::ENABLED:
            if(role == Qt::ToolTipRole)
                return tr("Sort by state");
            break;
        case Column::LNAME:
            if(role == Qt::DisplayRole)
                return tr("Local Folder");
            if(role == Qt::ToolTipRole)
                return tr("Sort by folder name");
            break;
        case Column::RNAME:
            if(role == Qt::DisplayRole)
                return tr("MEGA Folder");
            if(role == Qt::ToolTipRole)
                return tr("Sort by MEGA folder name");
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags SyncItemModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && (index.column() == Column::ENABLED))
        return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;

    return QAbstractItemModel::flags(index);
}

QModelIndex SyncItemModel::index(int row, int column, const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return createIndex(row, column, nullptr);
}

QModelIndex SyncItemModel::parent(const QModelIndex &index) const
{
    Q_UNUSED(index)
    // NOTE: Implement me for the tree view
    return QModelIndex();
}

int SyncItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
        return 0;

    return mList.size();
}

int SyncItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return kColumns;
}

QVariant SyncItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    if(mList.size() <= index.row())
    {
        return QVariant();
    }
    auto sync = mList.at(index.row());
    if(!sync)
    {
        return QVariant();
    }

    if(role == Qt::UserRole)
        return QVariant::fromValue(sync);

    switch(index.column())
    {
    case Column::ENABLED:
        if(role == Qt::CheckStateRole)
            return sync->isEnabled() ? Qt::Checked : Qt::Unchecked;
        else if(role == Qt::ToolTipRole)
            return sync->isEnabled() ? tr("Sync is enabled") : tr("Sync is disabled");
        break;
    case Column::LNAME:
        if(role == Qt::DecorationRole)
        {
            QIcon syncIcon;
            if(sync->getError())
            {
                syncIcon.addFile(QLatin1String(":/images/ic_sync_warning.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
            }
            else
            {
                syncIcon.addFile(QLatin1String(":/images/icons/synced-ico-rest.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
                syncIcon.addFile(QLatin1String(":/images/icons/synced-ico-hover.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Selected);
            }
            return syncIcon;
        }
        else if(role == Qt::DisplayRole)
        {
            return sync->name();
        }
        else if(role == Qt::ToolTipRole)
        {
            if(sync->getError())
                return QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
            else
                return sync->getLocalFolder();
        }
        break;
    case Column::RNAME:
        if(role == Qt::DisplayRole)
            return QFileInfo(sync->getMegaFolder()).fileName();
        else if(role == Qt::ToolTipRole)
            return sync->getMegaFolder();
        break;
    case Column::MENU:

        if(role == Qt::DecorationRole)
        {
            QIcon dotsMenu;
            dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-rest.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
            dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-press.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Selected);
            dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-hover.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Active);
            return dotsMenu;
        }
        else if(role == Qt::ToolTipRole)
            return tr("Click menu for more Sync actions");
        else if(role == Qt::TextAlignmentRole)
            return QVariant::fromValue<Qt::Alignment>(Qt::AlignHCenter);
        break;
    }
    return QVariant();
}

bool SyncItemModel::setData(const QModelIndex &index, const QVariant &value, int role)
{
    if (!index.isValid() || (index.column() != Column::ENABLED))
        return false;

    if ((role == Qt::CheckStateRole) || (role == Qt::EditRole))
    {
        auto sync = mList.at(index.row());
        if(!sync)
        {
            return false;
        }
        if (value.toInt() == Qt::Checked)
            emit enableSync(sync);
        else if (value.toInt() == Qt::Unchecked)
            emit disableSync(sync);
        emit dataChanged(index, index, QVector<int>(role));
        return true;
    }

    return false;
}

void SyncItemModel::fillData()
{
    setMode(mega::MegaSync::SyncType::TYPE_TWOWAY);
}

void SyncItemModel::insertSync(std::shared_ptr<SyncSetting> sync)
{
    if(mList.contains(sync))
    {
        for(auto it = mList.cbegin(); it!=mList.cend();++it)
        {
            if((*it) == sync)
            {
                int pos = it - mList.cbegin();
                emit dataChanged(index(pos, Column::ENABLED, QModelIndex()),
                                 index(pos, Column::ENABLED, QModelIndex()),
                                 QVector<int>(Qt::CheckStateRole));
                break;
            }
        }
    }
    else
    {
        if(sync->getType() == mSyncType)
        {
            beginInsertRows(QModelIndex(), mList.size(), mList.size());
            mList.append(sync);
            endInsertRows();
        }
    }
    emit syncUpdateFinished(sync);
}

void SyncItemModel::removeSync(std::shared_ptr<SyncSetting> sync)
{
    if(mList.contains(sync))
    {
        for(auto it = mList.cbegin(); it!=mList.cend();++it)
        {
            if((*it) == sync)
            {
                int pos = it - mList.cbegin();
                beginRemoveRows(QModelIndex(), pos, pos);
                mList.removeOne((*it));
                endRemoveRows();
                break;
            }
        }
    }
    emit syncUpdateFinished(sync);
}


QList<std::shared_ptr<SyncSetting> > SyncItemModel::getList() const
{
    return mList;
}

void SyncItemModel::setList(QList<std::shared_ptr<SyncSetting> > list)
{
    mList = list;
}

void SyncItemModel::setMode(mega::MegaSync::SyncType syncType)
{
    mSyncType = syncType;
    setList(mSyncModel->getSyncSettingsByType(mSyncType));
}

mega::MegaSync::SyncType SyncItemModel::getMode()
{
    return mSyncType;
}


SyncItemSortModel::SyncItemSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

SyncItemSortModel::~SyncItemSortModel()
{

}

bool SyncItemSortModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if(source_left.flags().testFlag(Qt::ItemIsUserCheckable) && source_right.flags().testFlag(Qt::ItemIsUserCheckable))
    {
       return source_left.data(Qt::CheckStateRole).toInt() > source_right.data(Qt::CheckStateRole).toInt();
    }
    bool leftToIntOK = false;
    bool rightToIntOK = false;
    int leftInt = source_left.data(Qt::DisplayRole).toInt(&leftToIntOK);
    int rightInt = source_right.data(Qt::DisplayRole).toInt(&rightToIntOK);
    if(leftToIntOK && rightToIntOK)
    {
        return leftInt < rightInt;
    }

    return QSortFilterProxyModel::lessThan(source_left, source_right);
}
