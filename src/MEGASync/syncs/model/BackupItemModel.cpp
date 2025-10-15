#include "BackupItemModel.h"

#include "SyncController.h"
#include "SyncTooltipCreator.h"
#include "Utilities.h"

#include <QCoreApplication>
#include <QFileInfo>
#include <QIcon>

BackupItemModel::BackupItemModel(QObject* parent):
    SyncItemModel(parent)
{}

QVariant BackupItemModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Horizontal)
    {
        switch (section)
        {
            case Column::LNAME:
                if (role == Qt::DisplayRole)
                    return tr("Local Folder");
                if (role == Qt::ToolTipRole)
                    return tr("Sort by name");
                break;
            case Column::STATE:
                if (role == Qt::DisplayRole)
                    return tr("State");
                if (role == Qt::ToolTipRole)
                    return tr("Sort by backup state");
                break;
        }
    }
    return SyncItemModel::headerData(section, orientation, role);
}

int BackupItemModel::columnCount(const QModelIndex& parent) const
{
    Q_UNUSED(parent)

    // We don´t have downloads on backups
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
                     QVector<int>() << Qt::CheckStateRole << Qt::DecorationRole << Qt::ToolTipRole);
}

QVariant BackupItemModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
        return QVariant();

    std::shared_ptr<SyncSettings> sync = getList().at(index.row());

    if (role == Qt::UserRole)
        return QVariant::fromValue(sync);

    switch (index.column())
    {
        case Column::ENABLED:
            if (role == Qt::ToolTipRole)
            {
                return sync->isActive() ? tr("Backup is enabled") : tr("Backup is disabled");
            }
            break;
        case Column::LNAME:
            if (role == Qt::DecorationRole)
            {
                if (sync->getRunState() == mega::MegaSync::RUNSTATE_RUNNING ||
                    (sync->getRunState() == mega::MegaSync::RUNSTATE_LOADING ||
                     sync->getRunState() == mega::MegaSync::RUNSTATE_PENDING))
                {
                    QIcon syncIcon;

                    QPixmap backupNormal =
                        Utilities::getColoredPixmap(QLatin1String("settings-backup"),
                                                    Utilities::AttributeType::NONE,
                                                    QLatin1String("icon-primary"),
                                                    QSize(STATES_ICON_SIZE, STATES_ICON_SIZE));

                    syncIcon.addPixmap(backupNormal, QIcon::Normal);

                    QPixmap backupSelected =
                        Utilities::getColoredPixmap(QLatin1String("settings-backup"),
                                                    Utilities::AttributeType::NONE,
                                                    QLatin1String("icon-inverse"),
                                                    QSize(STATES_ICON_SIZE, STATES_ICON_SIZE));

                    syncIcon.addPixmap(backupSelected, QIcon::Selected);

                    return syncIcon;
                }
            }
            else if (role == Qt::DisplayRole)
            {
                return SyncController::instance().getSyncNameFromPath(sync->getLocalFolder(true));
            }
            else if (role == Qt::ToolTipRole)
            {
                QString toolTip;
                toolTip += SyncTooltipCreator::createForLocal(sync->getLocalFolder());
                return toolTip;
            }
            break;
        case Column::DOWNLOADS:
            return QVariant();
            break;
    }
    return SyncItemModel::data(index, role);
}
