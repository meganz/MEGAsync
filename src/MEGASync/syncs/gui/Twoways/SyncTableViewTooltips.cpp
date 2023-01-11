#include "SyncTableViewTooltips.h"

#include "syncs/gui/SyncTooltipCreator.h"
#include <QCoreApplication>
#include <QHeaderView>
#include <QHelpEvent>
#include <QSortFilterProxyModel>
#include <QTableView>
#include <QToolTip>

bool SyncTableViewTooltips::eventFilter(QObject *watched, QEvent *event)
{
    if (event->type() == QEvent::ToolTip)
    {
        auto viewDelegate = dynamic_cast<QTableView*>(watched);
        if (viewDelegate && mModel)
        {
            auto proxyModel = dynamic_cast<QSortFilterProxyModel*>(viewDelegate->model());
            if (proxyModel)
            {
                auto helpEvent = static_cast<QHelpEvent*>(event);
                QPoint mousePos = correctedMousePosition(viewDelegate, helpEvent->pos());
                QModelIndex index = proxyModel->mapToSource(viewDelegate->indexAt(mousePos));
                int columnPosX = viewDelegate->columnViewportPosition(index.column());

                QString tooltipText = getTooltipText(mousePos, columnPosX, index);
                if (!tooltipText.isEmpty())
                {
                    QToolTip::showText(helpEvent->globalPos(), tooltipText);
                }
            }
        }
        return true;
    }
    return false;
}

void SyncTableViewTooltips::setSourceModel(SyncItemModel *model)
{
    mModel = model;
}

QString SyncTableViewTooltips::getTooltipText(const QPoint &mousePos, int columnPosX,
                                            const QModelIndex &index)
{
    auto sync = mModel->getSyncSettings(index);
    if (sync)
    {
        if (index.column() == SyncItemModel::Column::ENABLED)
        {
            return QString();//sync->isEnabled() ? tr("Sync is enabled") : tr("Sync is disabled");
        }
        else if (index.column() == SyncItemModel::Column::LNAME)
        {
            if (sync->getError() && isInIcon(mousePos, columnPosX))
            {
                return QCoreApplication::translate("MegaSyncError",
                                                   mega::MegaSync::getMegaSyncErrorCode(sync->getError()));
            }
            return SyncTooltipCreator::createForLocal(sync->getLocalFolder(true));
        }
        //else if (index.column() == SyncItemModel::Column::RNAME)
        //{
        //    return SyncTooltipCreator::createForRemote(sync->getMegaFolder());
        //}
        else if (index.column() == SyncItemModel::Column::MENU)
        {
            return tr("Click menu for more Sync actions");
        }
    }
    return QString();
}

QPoint SyncTableViewTooltips::correctedMousePosition(QTableView *viewDelegate,
                                                     const QPoint &mousePos)
{
    QPoint correctedPosition = mousePos;
    if (auto headerView = viewDelegate->horizontalHeader())
    {
        correctedPosition.setY(mousePos.y() - headerView->height());
    }
    return correctedPosition;
}

bool SyncTableViewTooltips::isInIcon(const QPoint &mousePos, int columnPosX)
{
    const int margin = 5;
    return (mousePos.x() - columnPosX) < (SyncItemModel::WARNING_ICON_SIZE + margin);
}
