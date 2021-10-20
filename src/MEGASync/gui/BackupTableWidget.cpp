#include "BackupTableWidget.h"

#include "platform/Platform.h"
#include "MenuItemAction.h"
#include "SyncController.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

BackupTableWidget::BackupTableWidget(QWidget *parent)
    : QTableView(parent)
{
    setIconSize(QSize(16, 16));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &BackupTableWidget::customContextMenuRequested, this, &BackupTableWidget::onCustomContextMenuRequested);
    connect(this, &BackupTableWidget::pressed, this, &BackupTableWidget::onCellClicked);
}

void BackupTableWidget::keyPressEvent(QKeyEvent *event)
{
    // implement smarter row based navigation
    switch(event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
            event->ignore();
            return;
        case Qt::Key_Tab:
            if(currentIndex().row() >= (model()->rowCount() - 1))
                selectRow(0);
            else
                selectRow(currentIndex().row() + 1);
            return;
        case Qt::Key_Backtab:
            if(currentIndex().row() <= 0)
                selectRow(model()->rowCount() - 1);
            else
                selectRow(currentIndex().row() - 1);
            return;
        default:
            QTableView::keyPressEvent(event);
    }
}

void BackupTableWidget::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    if(index.isValid() && (index.column() > BackupItemColumn::ENABLED))
        showContextMenu(viewport()->mapToGlobal(pos), index);
}

void BackupTableWidget::onCellClicked(const QModelIndex &index)
{
    if(index.isValid() && (index.column() != BackupItemColumn::ENABLED))
       selectionModel()->setCurrentIndex(model()->index(index.row(), BackupItemColumn::ENABLED), QItemSelectionModel::NoUpdate);
    if(index.isValid() && (index.column() == BackupItemColumn::MENU))
        showContextMenu(QCursor().pos(), index);
}

void BackupTableWidget::showContextMenu(const QPoint &pos, const QModelIndex index)
{
    QMenu *menu(new QMenu(this));

    menu->setAttribute(Qt::WA_TranslucentBackground);
#if defined(Q_OS_WINDOWS)
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
#endif

    // Show in file explorer action
    auto showLocalAction (new MenuItemAction(QCoreApplication::translate("Platform", Platform::fileExplorerString),
                                             QIcon(QString::fromUtf8("://images/show_in_folder_ico.png"))));
    connect(showLocalAction, &MenuItemAction::triggered, this, [index]()
    {
        auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>();
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(sync->getLocalFolder()));
    });

    // Show in Mega web action
    auto showRemoteAction (new MenuItemAction(tr("Open in MEGA"),
                                              QIcon(QString::fromUtf8("://images/ico_open_MEGA.png"))));
    connect(showRemoteAction, &MenuItemAction::triggered, this, [index]()
    {
        auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>();
        QString url = QString::fromUtf8("mega://#fm/") + sync->getMegaFolder();
        QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
    });

    // Remove Sync action
    auto delAction (new MenuItemAction(tr("Remove backup"),
                                       QIcon(QString::fromUtf8("://images/ico_Delete.png"))));
    delAction->setAccent(true);
    connect(delAction, &MenuItemAction::triggered, this, [index]()
    {
        auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>();
        SyncController::instance().removeSync(index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>());
    });


    showLocalAction->setParent(menu);
    showRemoteAction->setParent(menu);
    delAction->setParent(menu);

    menu->addAction(showLocalAction);
    menu->addAction(showRemoteAction);
    menu->addSeparator();
    menu->addAction(delAction);

    menu->popup(pos);
}
