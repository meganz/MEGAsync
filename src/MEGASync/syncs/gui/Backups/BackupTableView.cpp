#include "BackupTableView.h"

#include "platform/Platform.h"
#include "PlatformStrings.h"
#include "MenuItemAction.h"
#include "BackupItemModel.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

BackupTableView::BackupTableView(QWidget *parent)
    : SyncTableView(parent)
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
