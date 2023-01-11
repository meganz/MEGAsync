#include "Utilities.h"
#include "syncs/model/BackupItemModel.h"
#include "syncs/control/SyncController.h"
#include "syncs/gui/SyncTooltipCreator.h"

#include <QCoreApplication>
#include <QIcon>
#include <QFileInfo>


BackupItemModel::BackupItemModel(QObject *parent)
    : SyncItemModel(parent)
{
}

QVariant BackupItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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
                return tr("Sort by name");
            break;
        case Column_STATE:
            if(role == Qt::DisplayRole)
                return tr("State");
            if(role == Qt::ToolTipRole)
                return tr("Sort by backup state");
            break;
        case Column_FILES:
            if(role == Qt::DisplayRole)
                return tr("Files");
            if(role == Qt::ToolTipRole)
                return tr("Sort by file count");
            break;
        case Column_FOLDERS:
            if(role == Qt::DisplayRole)
                return tr("Folders");
            if(role == Qt::ToolTipRole)
                return tr("Sort by folder count");
            break;
        case Column_UPLOADS:
            if(role == Qt::DisplayRole)
                return tr("Uploading");
            if(role == Qt::ToolTipRole)
                return tr("Sort by Uploading");
        }
    }
    return QVariant();
}

int BackupItemModel::columnCount(const QModelIndex &parent) const
{
    Q_UNUSED(parent)

    return kColumns;
}

void BackupItemModel::fillData()
{
    setMode(mega::MegaSync::SyncType::TYPE_BACKUP);
}

void BackupItemModel::sendDataChanged(int row)
{
    emit dataChanged(index(row, Column::ENABLED, QModelIndex()),
                     index(row, Column::MENU, QModelIndex()),
                     QVector<int>()<< Qt::CheckStateRole << Qt::DecorationRole << Qt::ToolTipRole);
}

QVariant BackupItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
        return QVariant();

    std::shared_ptr<SyncSettings> sync = getList().at(index.row());

    if(role == Qt::UserRole)
        return QVariant::fromValue(sync);

    switch(index.column())
    {
    case Column::ENABLED:
        if(role == Qt::CheckStateRole)
            return sync->getRunState() == ::mega::MegaSync::RUNSTATE_RUNNING ? Qt::Checked : Qt::Unchecked;
        break;
    case Column::LNAME:
        if(role == Qt::DecorationRole)
        {
            QIcon syncIcon;
            if(sync->getError())
            {
                syncIcon.addFile(QLatin1String(":/images/ic_sync_warning.png"), QSize(WARNING_ICON_SIZE, WARNING_ICON_SIZE), QIcon::Normal);
            }
            else
            {
                syncIcon.addFile(QLatin1String(":/images/icons/folder/folder-rest.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Normal);
                syncIcon.addFile(QLatin1String(":/images/icons/folder/folder-hover.png"), QSize(ICON_SIZE, ICON_SIZE), QIcon::Selected);
            }
            return syncIcon;
        }
        else if(role == Qt::DisplayRole)
        {
            return SyncController::getSyncNameFromPath(sync->getLocalFolder(true));
        }
        else if(role == Qt::ToolTipRole)
        {
            QString toolTip;
            if(sync->getError())
            {
                toolTip += QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
                toolTip += QChar::LineSeparator;
            }
            toolTip += SyncTooltipCreator::createForLocal(sync->getLocalFolder(true));
            toolTip += QChar::LineSeparator;
            toolTip += SyncTooltipCreator::createForRemote(QString::fromUtf8(sync->getSync()->getLastKnownMegaFolder()));
            return toolTip;
        }
        break;
    case Column_STATE:
        if(role == Qt::DisplayRole)
        {
            std::string s;
            switch (sync->getRunState())
            {
            case ::mega::MegaSync::RUNSTATE_PENDING: s = "Pending"; break;
            case ::mega::MegaSync::RUNSTATE_LOADING: s = "Loading"; break;
            case ::mega::MegaSync::RUNSTATE_PAUSED: s = "Paused"; break;
            case ::mega::MegaSync::RUNSTATE_SUSPENDED: s = "Suspended"; break;
            case ::mega::MegaSync::RUNSTATE_DISABLED: s = "Disabled"; break;
            case ::mega::MegaSync::RUNSTATE_RUNNING:
                {
                    auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
                    if (it != mSyncInfo->mSyncStatsMap.end())
                    {
                        ::mega::MegaSyncStats& stats = *it->second;
                        if (stats.isScanning())
                        {
                            s = "Scanning";
                        }
                        else if (stats.isSyncing())
                        {
                            s = "Syncing";
                        }
                        else
                        {
                            s = "Monitoring";
                        }
                    }
                }
            }
            return QString::fromStdString(s);
        }
        break;
    case Column_FILES:
        if(role == Qt::DisplayRole)
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                ::mega::MegaSyncStats& stats = *it->second;
                return QString::fromStdString(std::to_string(stats.getFileCount()));
            }
        }
        break;
    case Column_FOLDERS:
        if(role == Qt::DisplayRole)
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                ::mega::MegaSyncStats& stats = *it->second;
                return QString::fromStdString(std::to_string(stats.getFolderCount()));
            }
        }
        break;
    case Column_UPLOADS:
        if(role == Qt::DisplayRole)
        {
            auto it = mSyncInfo->mSyncStatsMap.find(sync->backupId());
            if (it != mSyncInfo->mSyncStatsMap.end())
            {
                ::mega::MegaSyncStats& stats = *it->second;
                return QString::fromStdString(std::to_string(stats.getUploadCount()));
            }
        }
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
        else if(role == Qt::TextAlignmentRole)
            return QVariant::fromValue<Qt::Alignment>(Qt::AlignHCenter);
        break;
    }
    return QVariant();
}

