#include "syncs/model/SyncItemModel.h"
#include "syncs/gui/SyncTooltipCreator.h"
#include "syncs/control/SyncInfo.h"

#include "control/Utilities.h"
#include "MegaApplication.h"

#include <QCoreApplication>
#include <QIcon>
#include <QFileInfo>
#include <QDebug>

const int SyncItemModel::ICON_SIZE = 24;
const int SyncItemModel::STATES_ICON_SIZE = 16;

const int SyncItemModel::ErrorTooltipRole = Qt::UserRole + 1;


SyncItemModel::SyncItemModel(QObject *parent)
    : QAbstractItemModel(parent),
    mSyncInfo (SyncInfo::instance())
{
    connect(mSyncInfo, &SyncInfo::syncStateChanged, this, &SyncItemModel::insertSync);
    connect(mSyncInfo, &SyncInfo::syncStatsUpdated, this, &SyncItemModel::updateSyncStats);
    connect(mSyncInfo, &SyncInfo::syncRemoved, this, &SyncItemModel::removeSync);
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
                return tr("Sync Name");
            if(role == Qt::ToolTipRole)
                return tr("Sort by sync name");
            break;
        case Column::STATE:
            if(role == Qt::DisplayRole)
                return tr("State");
            if(role == Qt::ToolTipRole)
                return tr("Sort by sync state");
            break;
        case Column::FILES:
            if(role == Qt::DisplayRole)
                return tr("Files");
            if(role == Qt::ToolTipRole)
                return tr("Sort by file count");
            break;
        case Column::FOLDERS:
            if(role == Qt::DisplayRole)
                return tr("Folders");
            if(role == Qt::ToolTipRole)
                return tr("Sort by folder count");
            break;
        case Column::DOWNLOADS:
            if(role == Qt::DisplayRole)
                return tr("Downloads");
            if(role == Qt::ToolTipRole)
                return tr("Sort by Downloads");
            break;
        case Column::UPLOADS:
            if(role == Qt::DisplayRole)
                return tr("Uploads");
            if(role == Qt::ToolTipRole)
                return tr("Sort by Uploads");
            break;
        }
    }
    return QVariant();
}

Qt::ItemFlags SyncItemModel::flags(const QModelIndex &index) const
{
    if (index.isValid() && (index.column() == Column::ENABLED))
    {
        auto sync = mList.at(index.row());
        if(sync)
        {
            return QAbstractItemModel::flags(index) | Qt::ItemIsUserCheckable;
        }
    }

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
        {
            if(role == Qt::CheckStateRole)
            {
                return sync->getRunState() == ::mega::MegaSync::RUNSTATE_RUNNING ? Qt::Checked : Qt::Unchecked;
            }
                break;
        }
        case Column::LNAME:
        {
            if(role == Qt::DecorationRole)
            {
                return getStateIcon(sync);
            }
            else if(role == Qt::DisplayRole)
            {
                return sync->name(false, true);
            }
            else if(role == Qt::ToolTipRole)
            {
                QString toolTip;
                toolTip += SyncTooltipCreator::createForLocal(sync->getLocalFolder());
                toolTip += QChar::LineSeparator;
                toolTip += SyncTooltipCreator::createForRemote(sync->getMegaFolder());
                return toolTip;
            }
            else if(role == ErrorTooltipRole)
            {
                QString toolTip;
                toolTip += QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
                return toolTip;
            }
            break;
        }
        case Column::STATE:
        {
            if(role == Qt::DisplayRole)
            {
                QString s;
                switch (sync->getRunState())
                {
                case ::mega::MegaSync::RUNSTATE_PENDING:
                case ::mega::MegaSync::RUNSTATE_LOADING: s = tr("Loading"); break;
                case ::mega::MegaSync::RUNSTATE_PAUSED: s = tr("Paused"); break;
                case ::mega::MegaSync::RUNSTATE_SUSPENDED:
                    {
                        if(sync->getError())
                        {
                            s = tr("Suspended");
                        }
                        else
                        {
                            s = tr("Paused");
                        }
                        break;
                    }
                case ::mega::MegaSync::RUNSTATE_DISABLED: s = tr("Disabled"); break;
                case ::mega::MegaSync::RUNSTATE_RUNNING:
                    {
                        auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
                        if (it != mSyncInfo->mSyncStatsMap.end())
                        {
                            ::mega::MegaSyncStats& stats = *it->second;
                            if (stats.isScanning())
                            {
                                s = tr("Scanning");
                            }
                            else if (stats.isSyncing())
                            {
                                s = tr("Syncing");
                            }
                            else
                            {
                                s = tr("Monitoring");
                            }
                        }
                    }
                }
                return s;
            }
            break;
        }
        case Column::FILES:
        {
            auto statsGetter = [](const ::mega::MegaSyncStats& stats){
                return stats.getFileCount();
            };
            return getColumnStats(role, sync->backupId(), statsGetter);
        }
        case Column::FOLDERS:
        {
            auto statsGetter = [](const ::mega::MegaSyncStats& stats){
                return stats.getFolderCount();
            };
            return getColumnStats(role, sync->backupId(), statsGetter);
        }
        case Column::DOWNLOADS:
        {
            auto statsGetter = [](const ::mega::MegaSyncStats& stats){
                return stats.getDownloadCount();
            };
            return getColumnStats(role, sync->backupId(), statsGetter);
        }
        case Column::UPLOADS:
        {
            auto statsGetter = [](const ::mega::MegaSyncStats& stats) -> int{
                return stats.getUploadCount();
            };
            return getColumnStats(role, sync->backupId(), statsGetter);
        }
        case Column::MENU:
        {
            if(role == Qt::DecorationRole)
            {
                QIcon dotsMenu;
                dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-rest.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
                dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-press.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Selected);
                dotsMenu.addFile(QLatin1String("://images/icons/options_dots/options-hover.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Active);
                return dotsMenu;
            }
            else if(role == Qt::TextAlignmentRole)
            {
                return QVariant::fromValue<Qt::Alignment>(Qt::AlignHCenter);
            }
            else if(role == Qt::ToolTipRole)
            {
                return tr("Click menu for more Sync actions");
            }
            break;
        }
    }

    return QVariant();
}

QVariant SyncItemModel::getColumnStats(int role, mega::MegaHandle backupId, std::function<int (const ::mega::MegaSyncStats &)> statsGetter) const
{
    if(role == Qt::DisplayRole)
    {
        auto it = mSyncInfo->mSyncStatsMap.find(backupId);
        if (it != mSyncInfo->mSyncStatsMap.end())
        {
            const ::mega::MegaSyncStats& stats = *it->second;
            return statsGetter(stats);
        }
    }

    return QVariant();
}

QIcon SyncItemModel::getStateIcon(const std::shared_ptr<SyncSettings> &sync) const
{
    QIcon syncIcon;

    if(sync->getRunState() == mega::MegaSync::RUNSTATE_RUNNING
        || (sync->getRunState() == mega::MegaSync::RUNSTATE_LOADING || sync->getRunState() == mega::MegaSync::RUNSTATE_PENDING))
    {
        syncIcon.addFile(QLatin1String(":/images/sync_states/sync-running.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
        syncIcon.addFile(QLatin1String(":/images/sync_states/sync-running-selected.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Selected);
    }
    else if(sync->getRunState() == mega::MegaSync::RUNSTATE_PAUSED)
    {
        syncIcon.addFile(QLatin1String(":/images/sync_states/pause-circle.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
        syncIcon.addFile(QLatin1String(":/images/sync_states/pause-circle-selected.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Selected);
    }
    else if(sync->getRunState() == mega::MegaSync::RUNSTATE_DISABLED)
    {
        if(sync->getError())
        {
            syncIcon.addFile(QLatin1String(":/images/sync_states/x-circle-error.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
        }
        else
        {
            syncIcon.addFile(QLatin1String(":/images/sync_states/x-circle.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
        }

        syncIcon.addFile(QLatin1String(":/images/sync_states/x-circle-selected.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Selected);
    }
    else if(sync->getRunState() == mega::MegaSync::RUNSTATE_SUSPENDED)
    {
        if(sync->getError())
        {
            syncIcon.addFile(QLatin1String(":/images/sync_states/hand-error.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
            syncIcon.addFile(QLatin1String(":/images/sync_states/hand-selected.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Selected);
        }
        else
        {
            syncIcon.addFile(QLatin1String(":/images/sync_states/pause-circle.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Normal);
            syncIcon.addFile(QLatin1String(":/images/sync_states/pause-circle-selected.png"), QSize(STATES_ICON_SIZE, STATES_ICON_SIZE), QIcon::Selected);
        }
    }

    return syncIcon;
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
            emit signalSyncCheckboxOn(sync);
        else if (value.toInt() == Qt::Unchecked)
            emit signalSyncCheckboxOff(sync);
        emit dataChanged(index, index, QVector<int>(role));
        return true;
    }

    return false;
}

void SyncItemModel::fillData()
{
    setMode(mega::MegaSync::SyncType::TYPE_TWOWAY);
}

std::shared_ptr<SyncSettings> SyncItemModel::getSyncSettings(const QModelIndex &index) const
{
    if (!index.isValid() || mList.size() <= index.row())
    {
        return std::shared_ptr<SyncSettings>();
    }

    return mList.at(index.row());
}

void SyncItemModel::insertSync(std::shared_ptr<SyncSettings> sync)
{
    if(mList.contains(sync))
    {
        for(auto it = mList.cbegin(); it!=mList.cend();++it)
        {
            if((*it) == sync)
            {
                int pos = it - mList.cbegin();
                sendDataChanged(pos);
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

void SyncItemModel::updateSyncStats(std::shared_ptr<::mega::MegaSyncStats> stats)
{
    for(auto it = mList.cbegin(); it!=mList.cend();++it)
    {
        if((*it)->backupId() == stats->getBackupId())
        {
            int pos = it - mList.cbegin();
            sendDataChanged(pos);
            break;
        }
    }
}

void SyncItemModel::removeSync(std::shared_ptr<SyncSettings> sync)
{
    if(mList.contains(sync))
    {
        for(auto it = mList.cbegin(); it!=mList.cend();++it)
        {
            if((*it) == sync)
            {
                int pos = std::distance(mList.cbegin(), it);
                beginRemoveRows(QModelIndex(), pos, pos);
                mList.removeOne((*it));
                endRemoveRows();
                break;
            }
        }
    }
    emit syncUpdateFinished(sync);
}

void SyncItemModel::sendDataChanged(int row)
{
    emit dataChanged(index(row, Column::ENABLED, QModelIndex()),
                     index(row, Column::MENU, QModelIndex()),
                     QVector<int>()<< Qt::CheckStateRole << Qt::DecorationRole << Qt::ToolTipRole);
}

QList<std::shared_ptr<SyncSettings> > SyncItemModel::getList() const
{
    return mList;
}

void SyncItemModel::setList(QList<std::shared_ptr<SyncSettings> > list)
{
    mList = list;
}

void SyncItemModel::setMode(mega::MegaSync::SyncType syncType)
{
    mSyncType = syncType;
    setList(mSyncInfo->getSyncSettingsByType(mSyncType));
}

mega::MegaSync::SyncType SyncItemModel::getMode()
{
    return mSyncType;
}

SyncItemSortModel::SyncItemSortModel(QObject *parent) : QSortFilterProxyModel(parent)
{
    mQCollator.setNumericMode(true);
    mQCollator.setCaseSensitivity(Qt::CaseInsensitive);
}

bool SyncItemSortModel::lessThan(const QModelIndex &source_left, const QModelIndex &source_right) const
{
    if(source_left.flags().testFlag(Qt::ItemIsUserCheckable)
            && source_right.flags().testFlag(Qt::ItemIsUserCheckable))
    {
        auto stateLeft (source_left.data(Qt::CheckStateRole).toInt());
        auto stateRight (source_right.data(Qt::CheckStateRole).toInt());
        if (stateLeft != stateRight)
        {
            return stateLeft > stateRight;

        }
    }

    return mQCollator.compare(source_left.data(Qt::DisplayRole).toString(),
                              source_right.data(Qt::DisplayRole).toString()) < 0;
}
