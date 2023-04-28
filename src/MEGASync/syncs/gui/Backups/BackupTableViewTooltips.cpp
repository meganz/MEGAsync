#include "BackupTableViewTooltips.h"

#include "syncs/model/BackupItemModel.h"
#include "syncs/gui/SyncTooltipCreator.h"
#include <QCoreApplication>

BackupTableViewTooltips::BackupTableViewTooltips()
    : mMyBackupsHandleRequest(UserAttributes::MyBackupsHandle::requestMyBackupsHandle())
{
}

QString BackupTableViewTooltips::getTooltipText(const QPoint &mousePos, int columnPosX, const QModelIndex &index)
{
    auto sync = mModel->getSyncSettings(index);
    if (sync)
    {
        if (index.column() == BackupItemModel::Column::ENABLED)
        {
            return sync->isEnabled() ? tr("Backup is enabled") : tr("Backup is disabled");
        }
        else if (index.column() == BackupItemModel::Column::LNAME)
        {
            if (sync->getError() && isInIcon(mousePos, columnPosX))
            {
                return QCoreApplication::translate("MegaSyncError",
                                                   mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
            }
            QString toolTip;
            toolTip += SyncTooltipCreator::createForLocal(sync->getLocalFolder(true));
            toolTip += QChar::LineSeparator;

            toolTip += SyncTooltipCreator::createForRemote(
                        mMyBackupsHandleRequest->getNodeLocalizedPath(sync->getMegaFolder()));
            return toolTip;
        }
        else if (index.column() == BackupItemModel::Column::MENU)
        {
            return tr("Click menu for more Backup actions");
        }
    }
    return QString();
}
