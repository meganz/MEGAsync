#include "Utilities.h"
#include "syncs/model/BackupItemModel.h"
#include "syncs/gui/SyncTooltipCreator.h"
#include "UserAttributesRequests/DeviceName.h"
#include "UserAttributesRequests/MyBackupsHandle.h"

#include <QCoreApplication>
#include <QIcon>
#include <QFileInfo>


BackupItemModel::BackupItemModel(QObject *parent)
    : SyncItemModel(parent),
      mDeviceNameRequest (UserAttributes::DeviceName::requestDeviceName()),
      mMyBackupsHandleRequest (UserAttributes::MyBackupsHandle::requestMyBackupsHandle())
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
            return sync->getSync()->getRunState() == ::mega::MegaSync::RUNSTATE_RUNNING ? Qt::Checked : Qt::Unchecked;
        else if(role == Qt::ToolTipRole)
            return sync->getSync()->getRunState() == ::mega::MegaSync::RUNSTATE_RUNNING ? tr("Backup is enabled") : tr("Backup is disabled");
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
            return sync->name();
        }
        else if(role == Qt::ToolTipRole)
        {
            QString toolTip;
            if(sync->getError())
            {
                toolTip += QCoreApplication::translate("MegaSyncError", mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
                toolTip += QChar::LineSeparator;
            }
            toolTip += SyncTooltipCreator::createForLocal(sync->getLocalFolder());
            toolTip += QChar::LineSeparator;
            toolTip += SyncTooltipCreator::createForRemote(
                        mMyBackupsHandleRequest->getNodeLocalizedPath(sync->getMegaFolder()));
            return toolTip;
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
        else if(role == Qt::ToolTipRole)
            return tr("Click menu for more Backup actions");
        else if(role == Qt::TextAlignmentRole)
            return QVariant::fromValue<Qt::Alignment>(Qt::AlignHCenter);
        break;
    }
    return QVariant();
}

