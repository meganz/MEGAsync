#include "BackupTableView.h"

#include "platform/Platform.h"
#include "PlatformStrings.h"
#include "MenuItemAction.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

BackupTableView::BackupTableView(QWidget *parent)
    : SyncTableView(parent)
{
    mContextMenuName = "BackupContextMenu";
}

void BackupTableView::initTable()
{
    setColumnHidden(BackupItemModel::Column::Column_DOWNLOADS, true);
    setItemDelegate(new BackgroundColorDelegate(this));
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

QString BackupTableView::getRemoveActionString()
{
    return tr("Stop backup");
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

void BackupTableView::removeActionClicked(std::shared_ptr<SyncSettings> settings)
{
    emit removeBackup(settings);
}
