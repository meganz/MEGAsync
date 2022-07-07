#include "SyncTableView.h"

#include "platform/Platform.h"
#include "MenuItemAction.h"
#include "SyncController.h"

#include <QHeaderView>
#include <QMenu>
#include <QtConcurrent/QtConcurrent>

SyncTableView::SyncTableView(QWidget *parent)
    : QTableView(parent),
    mSyncController(this),
    mIsFirstTime(true)
{
    setIconSize(QSize(24, 24));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this, &SyncTableView::customContextMenuRequested, this, &SyncTableView::onCustomContextMenuRequested);
    connect(this, &SyncTableView::pressed, this, &SyncTableView::onCellClicked);
}

void SyncTableView::keyPressEvent(QKeyEvent *event)
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

void SyncTableView::showEvent(QShowEvent *event)
{
    if(mIsFirstTime)
    {
        Q_UNUSED(event);
        initTable();
        mIsFirstTime = false;
    }
}

void SyncTableView::initTable()
{
    setItemDelegate(new SelectionIconNoChangeOnDisable(this));
    setItemDelegateForColumn(SyncItemModel::Column::MENU, new MenuItemDelegate(this));

    horizontalHeader()->resizeSection(SyncItemModel::Column::ENABLED, FIXED_COLUMN_WIDTH);
    horizontalHeader()->resizeSection(SyncItemModel::Column::MENU, FIXED_COLUMN_WIDTH);
    horizontalHeader()->resizeSection(SyncItemModel::Column::MENU, FIXED_COLUMN_WIDTH);

    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::ENABLED, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::MENU, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(SyncItemModel::Column::LNAME, (width() - FIXED_COLUMN_WIDTH * 2) / 2);

    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::LNAME, QHeaderView::Interactive);
    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::RNAME, QHeaderView::Stretch);

    setFont(QFont().defaultFamily());

    // Hijack the sorting on the dots MENU column and hide the sort indicator,
    // instead of showing a bogus sort on that column;
    connect(horizontalHeader(), &QHeaderView::sortIndicatorChanged, this, [this](int index, Qt::SortOrder order)
    {
        if (index == SyncItemModel::Column::MENU)
            horizontalHeader()->setSortIndicator(-1, order);
    });

    // Sort by sync name by default
    sortByColumn(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
    setSortingEnabled(true);
    horizontalHeader()->setSortIndicator(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
}

void SyncTableView::onCustomContextMenuRequested(const QPoint &pos)
{
    QModelIndex index = indexAt(pos);
    if(index.isValid() && (index.column() > SyncItemModel::Column::ENABLED))
        showContextMenu(viewport()->mapToGlobal(pos), index);
}

void SyncTableView::onCellClicked(const QModelIndex &index)
{
    if(index.isValid() && (index.column() != SyncItemModel::Column::ENABLED))
       selectionModel()->setCurrentIndex(model()->index(index.row(), SyncItemModel::Column::ENABLED), QItemSelectionModel::NoUpdate);
    if(index.isValid() && (index.column() == SyncItemModel::Column::MENU))
        showContextMenu(QCursor().pos(), index);
}

void SyncTableView::showContextMenu(const QPoint &pos, const QModelIndex index)
{
    QMenu *menu(new QMenu(this));
    Platform::initMenu(menu);
    menu->setAttribute(Qt::WA_DeleteOnClose);

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
    auto delAction (new MenuItemAction(tr("Remove synced folder"),
                                       QIcon(QString::fromUtf8("://images/ico_Delete.png"))));
    delAction->setAccent(true);
    connect(delAction, &MenuItemAction::triggered, this, [this, index]()
    {
        auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSetting>>();
        emit removeSync(sync);
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

MenuItemDelegate::MenuItemDelegate(QObject *parent) : QStyledItemDelegate(parent)
{

}

MenuItemDelegate::~MenuItemDelegate()
{

}

void MenuItemDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionViewItem opt(option);
    opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();
    opt.decorationPosition = QStyleOptionViewItem::Top;
    QStyledItemDelegate::paint(painter, opt, index);
}

SelectionIconNoChangeOnDisable::SelectionIconNoChangeOnDisable(QObject *parent) : QStyledItemDelegate(parent)
{

}

SelectionIconNoChangeOnDisable::~SelectionIconNoChangeOnDisable()
{

}

void SelectionIconNoChangeOnDisable::initStyleOption(QStyleOptionViewItem *option, const QModelIndex &index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    if(!option->state.testFlag(QStyle::State_Enabled) && option->state.testFlag(QStyle::State_Selected))
    {
        option->state.setFlag(QStyle::State_Enabled, true);
    }
}
