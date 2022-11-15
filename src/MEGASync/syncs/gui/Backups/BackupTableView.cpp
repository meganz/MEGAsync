#include "BackupTableView.h"

#include "platform/Platform.h"
#include "MenuItemAction.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

BackupTableView::BackupTableView(QWidget *parent)
    : SyncTableView(parent)
{
}

void BackupTableView::initTable()
{
    setItemDelegateForColumn(BackupItemModel::Column::MENU, new MenuItemDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::LNAME, new IconMiddleDelegate(this));
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    horizontalHeader()->resizeSection(BackupItemModel::Column::ENABLED, FIXED_COLUMN_WIDTH);
    horizontalHeader()->resizeSection(BackupItemModel::Column::MENU, FIXED_COLUMN_WIDTH);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setSectionResizeMode(BackupItemModel::Column::ENABLED, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(BackupItemModel::Column::LNAME,QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(BackupItemModel::Column::MENU, QHeaderView::Fixed);
    setFont(QFont().defaultFamily());

    // Hijack the sorting on the dots MENU column and hide the sort indicator,
    // instead of showing a bogus sort on that column;
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [this](int index, Qt::SortOrder order)
    {
        if (index == BackupItemModel::Column::MENU)
        {
            horizontalHeader()->setSortIndicator(-1, order);
        }
    });

    // Sort by sync name by default
    sortByColumn(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
    setSortingEnabled(true);
    horizontalHeader()->setSortIndicator(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
}

void BackupTableView::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    if (index.isValid() && (index.column() > BackupItemModel::Column::ENABLED))
    {
        showContextMenu(viewport()->mapToGlobal(pos), index);
    }
}

void BackupTableView::onCellClicked(const QModelIndex &index)
{
    if (index.isValid() && (index.column() != BackupItemModel::Column::ENABLED))
    {
       selectionModel()->setCurrentIndex(model()->index(index.row(), BackupItemModel::Column::ENABLED), QItemSelectionModel::NoUpdate);
    }

    if (index.isValid() && (index.column() == BackupItemModel::Column::MENU))
    {
        showContextMenu(QCursor().pos(), index);
    }
}

void BackupTableView::showContextMenu(const QPoint &pos, const QModelIndex index)
{
    auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>();

    QMenu *menu(new QMenu(this));
    Platform::initMenu(menu);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    // Show in system file explorer action
    auto openLocalAction (new MenuItemAction(QCoreApplication::translate("Platform", Platform::fileExplorerString),
                                             QIcon(QString::fromUtf8("://images/show_in_folder_ico.png"))));
    connect(openLocalAction, &MenuItemAction::triggered, this, [sync]()
    {
        Utilities::openUrl(QUrl::fromLocalFile(sync->getLocalFolder()));
    });

    // Show in MEGA Web App action
    auto openRemoteAction (new MenuItemAction(tr("Open in MEGA"),
                                              QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    connect(openRemoteAction, &MenuItemAction::triggered, this, [this, sync]()
    {
        emit openInMEGA(sync->getMegaHandle());
    });

    // Remove Backup action
    auto removeAction (new MenuItemAction(tr("Remove backup"),
                                       QIcon(QString::fromUtf8("://images/ico_Delete.png"))));
    removeAction->setAccent(true);
    connect(removeAction, &MenuItemAction::triggered, this, [this, sync]()
    {
        emit removeBackup(sync);
    });

    auto syncRun (new MenuItemAction(tr("Run"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncPause (new MenuItemAction(tr("Pause"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncSuspend (new MenuItemAction(tr("Suspend"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto syncDisable (new MenuItemAction(tr("Disable"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    auto openMegaignore (new MenuItemAction(tr("Edit .megaignore"), QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));

    connect(syncRun, &MenuItemAction::triggered, this, [this, sync]() { emit signalRunSync(sync); });
    connect(syncPause, &MenuItemAction::triggered, this, [this, sync]() { emit signalPauseSync(sync); });
    connect(syncSuspend, &MenuItemAction::triggered, this, [this, sync]() { emit signalSuspendSync(sync); });
    connect(syncDisable, &MenuItemAction::triggered, this, [this, sync]() { emit signalDisableSync(sync); });
    connect(openMegaignore, &MenuItemAction::triggered, this, [this, sync]() { emit signalOpenMegaignore(sync); });

    syncRun->setEnabled(sync->getSync()->getRunState() !=  mega::MegaSync::RUNSTATE_RUNNING);
    syncPause->setEnabled(sync->getSync()->getRunState() !=  mega::MegaSync::RUNSTATE_PAUSED);
    syncSuspend->setEnabled(sync->getSync()->getRunState() !=  mega::MegaSync::RUNSTATE_SUSPENDED);
    syncDisable->setEnabled(sync->getSync()->getRunState() !=  mega::MegaSync::RUNSTATE_DISABLED);

    syncRun->setParent(menu);
    syncPause->setParent(menu);
    syncSuspend->setParent(menu);
    syncDisable->setParent(menu);
    openMegaignore->setParent(menu);

    openLocalAction->setParent(menu);
    openRemoteAction->setParent(menu);
    removeAction->setParent(menu);

    menu->addAction(openLocalAction);
    menu->addAction(openRemoteAction);
    menu->addSeparator();
    menu->addAction(removeAction);

    menu->addSeparator();
    menu->addAction(syncRun);
    menu->addAction(syncPause);
    menu->addAction(syncSuspend);
    menu->addAction(syncDisable);

    menu->addSeparator();
    menu->addAction(openMegaignore);

    menu->popup(pos);
}
