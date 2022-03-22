#include "BackupTableWidget.h"

#include "platform/Platform.h"
#include "MenuItemAction.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

BackupTableWidget::BackupTableWidget(QWidget *parent)
    : QTableView(parent)
{
    setIconSize(QSize(24, 24));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &BackupTableWidget::customContextMenuRequested, this, &BackupTableWidget::onCustomContextMenuRequested);
    connect(this, &BackupTableWidget::pressed, this, &BackupTableWidget::onCellClicked);
}

void BackupTableWidget::customize()
{
    horizontalHeader()->setSortIndicator(-1, Qt::AscendingOrder);
    horizontalHeader()->resizeSection(BackupItemColumn::ENABLED, 32);
    horizontalHeader()->resizeSection(BackupItemColumn::MENU, 32);
    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setSectionResizeMode(BackupItemColumn::ENABLED, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(BackupItemColumn::LNAME,QHeaderView::Stretch);
    horizontalHeader()->setSectionResizeMode(BackupItemColumn::MENU, QHeaderView::Fixed);
    setFont(QFont().defaultFamily());

    // Hijack the sorting on the dots MENU column and hide the sort indicator,
    // instead of showing a bogus sort on that column;
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [this](int index, Qt::SortOrder order)
    {
        if (index == BackupItemColumn::MENU)
            horizontalHeader()->setSortIndicator(-1, order);
    });
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
    auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>();

    QMenu *menu(new QMenu(this));
    menu->setAttribute(Qt::WA_TranslucentBackground);
#if defined(Q_OS_WINDOWS)
    menu->setWindowFlags(menu->windowFlags() | Qt::FramelessWindowHint | Qt::NoDropShadowWindowHint);
#endif

    // Show in system file explorer action
    auto openLocalAction (new MenuItemAction(QCoreApplication::translate("Platform", Platform::fileExplorerString),
                                             QIcon(QString::fromUtf8("://images/show_in_folder_ico.png"))));
    connect(openLocalAction, &MenuItemAction::triggered, this, [sync]()
    {
        QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(sync->getLocalFolder()));
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


    openLocalAction->setParent(menu);
    openRemoteAction->setParent(menu);
    removeAction->setParent(menu);

    menu->addAction(openLocalAction);
    menu->addAction(openRemoteAction);
    menu->addSeparator();
    menu->addAction(removeAction);

    menu->popup(pos);
}
