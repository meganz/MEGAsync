#include "BackupTableView.h"

#include "BackupItemModel.h"
#include "MenuItemAction.h"
#include "Platform.h"
#include "PlatformStrings.h"

#include <QtConcurrent/QtConcurrent>

#include <QHeaderView>
#include <QMenu>

BackupTableView::BackupTableView(QWidget* parent):
    SyncTableView(parent)
{
    mType = mega::MegaSync::TYPE_BACKUP;
    mContextMenuName = "BackupContextMenu";
}

void BackupTableView::initTable()
{
    setColumnHidden(BackupItemModel::Column::DOWNLOADS, true);
    SyncTableView::initTable();
}

QString BackupTableView::getRemoveActionString()
{
    return tr("Stop backup");
}
