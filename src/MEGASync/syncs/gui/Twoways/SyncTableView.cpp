#include "SyncTableView.h"

#include "MenuItemAction.h"
#include "Platform.h"
#include "PlatformStrings.h"
#include "SyncItemModel.h"
#include "SyncSettings.h"
#include "TokenParserWidgetManager.h"

#include <QHeaderView>
#include <QKeyEvent>
#include <QMenu>
#include <QPainter>
#include <QPainterPath>
#include <QtConcurrent/QtConcurrent>
#include <QToolTip>

SyncTableView::SyncTableView(QWidget* parent):
    QTableView(parent),
    mContextMenuName("SyncContextMenu"),
    mType(mega::MegaSync::TYPE_TWOWAY),
    mIsFirstTime(true)
{
    setIconSize(QSize(24, 24));

    setContextMenuPolicy(Qt::CustomContextMenu);
    connect(this,
            &SyncTableView::customContextMenuRequested,
            this,
            &SyncTableView::onCustomContextMenuRequested);
    connect(this, &SyncTableView::pressed, this, &SyncTableView::onCellClicked);
}

void SyncTableView::keyPressEvent(QKeyEvent* event)
{
    // implement smarter row based navigation
    switch (event->key())
    {
        case Qt::Key_Left:
        case Qt::Key_Right:
            event->ignore();
            return;
        case Qt::Key_Tab:
            if (currentIndex().row() >= (model()->rowCount() - 1))
                selectRow(0);
            else
                selectRow(currentIndex().row() + 1);
            return;
        case Qt::Key_Backtab:
            if (currentIndex().row() <= 0)
                selectRow(model()->rowCount() - 1);
            else
                selectRow(currentIndex().row() - 1);
            return;
        default:
            QTableView::keyPressEvent(event);
    }
}

void SyncTableView::showEvent(QShowEvent* event)
{
    if (mIsFirstTime)
    {
        Q_UNUSED(event);
        initTable();
        mIsFirstTime = false;
    }
}

void SyncTableView::initTable()
{
    setItemDelegateForColumn(SyncItemModel::Column::ENABLED, new BackgroundColorDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::MENU, new MenuItemDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::LNAME, new IconMiddleDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::STATE, new ElideMiddleDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::FILES, new ElideMiddleDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::FOLDERS, new ElideMiddleDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::DOWNLOADS, new ElideMiddleDelegate(this));
    setItemDelegateForColumn(SyncItemModel::Column::UPLOADS, new ElideMiddleDelegate(this));

    setFont(QFont().defaultFamily());
    auto StateColumnWidth =
        horizontalHeader()->fontMetrics().horizontalAdvance(
            model()->headerData(SyncItemModel::Column::STATE, Qt::Horizontal).toString()) +
        6;
    auto FilesColumnWidth =
        horizontalHeader()->fontMetrics().horizontalAdvance(
            model()->headerData(SyncItemModel::Column::FILES, Qt::Horizontal).toString()) +
        6;
    auto FoldersColumnWidth =
        horizontalHeader()->fontMetrics().horizontalAdvance(
            model()->headerData(SyncItemModel::Column::FOLDERS, Qt::Horizontal).toString()) +
        6;
    auto DownloadsColumnWidth =
        horizontalHeader()->fontMetrics().horizontalAdvance(
            model()->headerData(SyncItemModel::Column::DOWNLOADS, Qt::Horizontal).toString()) +
        6;
    auto UploadsColumnWidth =
        horizontalHeader()->fontMetrics().horizontalAdvance(
            model()->headerData(SyncItemModel::Column::UPLOADS, Qt::Horizontal).toString()) +
        6;

    horizontalHeader()->resizeSection(SyncItemModel::Column::ENABLED, FIXED_COLUMN_WIDTH);
    horizontalHeader()->resizeSection(SyncItemModel::Column::MENU, FIXED_COLUMN_WIDTH);

    // 6 is the padding left of the header (set on RemoteItemUI stylesheet)
    horizontalHeader()->resizeSection(SyncItemModel::Column::STATE, 3 * StateColumnWidth);
    horizontalHeader()->resizeSection(SyncItemModel::Column::FILES, 2 * FilesColumnWidth);
    // 10 is an arbitrary padding for these two categories
    horizontalHeader()->resizeSection(SyncItemModel::Column::FOLDERS, FoldersColumnWidth + 10);
    horizontalHeader()->resizeSection(SyncItemModel::Column::DOWNLOADS, DownloadsColumnWidth + 10);
    horizontalHeader()->resizeSection(SyncItemModel::Column::UPLOADS, UploadsColumnWidth + 10);

    horizontalHeader()->setDefaultAlignment(Qt::AlignLeft);
    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::ENABLED, QHeaderView::Fixed);
    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::MENU, QHeaderView::Fixed);
    horizontalHeader()->resizeSection(SyncItemModel::Column::LNAME,
                                      (width() - FIXED_COLUMN_WIDTH * 11));

    horizontalHeader()->setSectionResizeMode(SyncItemModel::Column::LNAME,
                                             QHeaderView::Stretch); // QHeaderView::Interactive);
    horizontalHeader()->setTextElideMode(Qt::ElideMiddle);

    // Hijack the sorting on the dots MENU column and hide the sort indicator,
    // instead of showing a bogus sort on that column;
    connect(horizontalHeader(),
            &QHeaderView::sortIndicatorChanged,
            this,
            [this](int index, Qt::SortOrder order)
            {
                if (index == SyncItemModel::Column::MENU)
                    horizontalHeader()->setSortIndicator(-1, order);
            });

    // Sort by sync name by default
    sortByColumn(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
    setSortingEnabled(true);
    horizontalHeader()->setSortIndicator(SyncItemModel::Column::LNAME, Qt::AscendingOrder);
}

void SyncTableView::onCustomContextMenuRequested(const QPoint& pos)
{
    QModelIndex index = indexAt(pos);
    if (index.isValid() && (index.column() > SyncItemModel::Column::ENABLED))
        showContextMenu(viewport()->mapToGlobal(pos), index);
}

void SyncTableView::onCellClicked(const QModelIndex& index)
{
    if (index.isValid() && (index.column() != SyncItemModel::Column::ENABLED))
        selectionModel()->setCurrentIndex(
            model()->index(index.row(), SyncItemModel::Column::ENABLED),
            QItemSelectionModel::NoUpdate);
    if (index.isValid() && (index.column() == SyncItemModel::Column::MENU))
        showContextMenu(QCursor().pos(), index);
}

mega::MegaSync::SyncType SyncTableView::getType() const
{
    return mType;
}

void SyncTableView::showContextMenu(const QPoint& pos, const QModelIndex index)
{
    QMenu* menu(new QMenu(this));
    Platform::getInstance()->initMenu(menu, mContextMenuName);
    menu->setAttribute(Qt::WA_DeleteOnClose);

    auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>();

    // Show in file explorer action
    MenuItemAction* showLocalAction(nullptr);
    QFileInfo localFolder(sync->getLocalFolder());
    if (localFolder.exists())
    {
        showLocalAction =
            new MenuItemAction(PlatformStrings::fileExplorer(),
                               QLatin1String("://images/sync_context_menu/folder-small.png"),
                               menu);
        connect(showLocalAction,
                &MenuItemAction::triggered,
                this,
                [sync]()
                {
                    Utilities::openUrl(QUrl::fromLocalFile(sync->getLocalFolder()));
                });
    }

    // Show in Mega web action
    auto showRemoteAction(
        new MenuItemAction(QCoreApplication::translate("SyncTableView", "Open in MEGA"),
                           QLatin1String("://images/sync_context_menu/MEGA-small.png"),
                           menu));
    connect(showRemoteAction,
            &MenuItemAction::triggered,
            this,
            [sync]()
            {
                Utilities::openInMega(sync->getMegaHandle());
            });

    // Remove Sync action
    auto delAction(new MenuItemAction(getRemoveActionString(),
                                      QLatin1String("://images/sync_context_menu/minus-circle.png"),
                                      menu));
    connect(delAction,
            &MenuItemAction::triggered,
            this,
            [this, sync]()
            {
                removeActionClicked(sync);
            });

    if (showLocalAction)
    {
        menu->addAction(showLocalAction);
    }
    menu->addAction(showRemoteAction);
    menu->addSeparator();

    createStatesContextActions(menu, sync);
    menu->addSeparator();

    menu->addAction(delAction);

    if (!menu->actions().isEmpty())
    {
        menu->popup(pos);
    }
}

QString SyncTableView::getRemoveActionString()
{
    return QCoreApplication::translate("SyncTableView", "Remove synced folder");
}

void SyncTableView::removeActionClicked(std::shared_ptr<SyncSettings> settings)
{
    emit signalRemoveSync(settings);
}

void SyncTableView::createStatesContextActions(QMenu* menu, std::shared_ptr<SyncSettings> sync)
{
    auto addRun = [this, sync, menu]()
    {
        auto syncRun(
            new MenuItemAction(QCoreApplication::translate("SyncTableView", "Run"),
                               QLatin1String("://images/sync_context_menu/play-circle.png"),
                               menu));
        connect(syncRun,
                &MenuItemAction::triggered,
                this,
                [this, sync]()
                {
                    emit signalRunSync(sync);
                });
        menu->addAction(syncRun);
    };

    if (sync->getSync()->getError())
    {
        addRun();
    }
    else
    {
        if (sync->getSync()->getRunState() != mega::MegaSync::RUNSTATE_DISABLED &&
            sync->getSync()->getRunState() != mega::MegaSync::RUNSTATE_SUSPENDED)
        {
            auto syncSuspend(
                new MenuItemAction(QCoreApplication::translate("SyncTableView", "Pause"),
                                   QLatin1String("://images/sync_states/pause-circle.png"),
                                   menu));
            connect(syncSuspend,
                    &MenuItemAction::triggered,
                    this,
                    [this, sync]()
                    {
                        emit signalSuspendSync(sync);
                    });
            menu->addAction(syncSuspend);
        }
        else if (sync->getSync()->getRunState() != mega::MegaSync::RUNSTATE_RUNNING)
        {
            addRun();
        }
    }

    QFileInfo syncDir(sync->getLocalFolder());
    if (syncDir.exists())
    {
        auto addExclusions(new MenuItemAction(
            QCoreApplication::translate("ExclusionsStrings", "Manage exclusions"),
            QLatin1String("://images/sync_context_menu/slash-circle.png"),
            menu));
        connect(addExclusions,
                &MenuItemAction::triggered,
                this,
                [this, sync]()
                {
                    emit signaladdExclusions(sync);
                });

        menu->addSeparator();
        menu->addAction(addExclusions);
    }

    if (sync->getSync()->getRunState() == mega::MegaSync::RUNSTATE_RUNNING)
    {
        auto rescanQuick(
            new MenuItemAction(QCoreApplication::translate("SyncTableView", "Quick Rescan"),
                               QLatin1String("://images/sync_context_menu/search-small.png"),
                               menu));
        connect(rescanQuick,
                &MenuItemAction::triggered,
                this,
                [this, sync]()
                {
                    emit signalRescanQuick(sync);
                });

        auto rescanDeep(
            new MenuItemAction(QCoreApplication::translate("SyncTableView", "Deep Rescan"),
                               QLatin1String("://images/sync_context_menu/search-dark-small.png"),
                               menu));
        connect(rescanDeep,
                &MenuItemAction::triggered,
                this,
                [this, sync]()
                {
                    emit signalRescanDeep(sync);
                });

        menu->addSeparator();
        menu->addAction(rescanQuick);
        menu->addAction(rescanDeep);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
BackgroundColorDelegate::BackgroundColorDelegate(QObject* parent):
    QStyledItemDelegate(parent)
{}

void BackgroundColorDelegate::paint(QPainter* painter,
                                    const QStyleOptionViewItem& option,
                                    const QModelIndex& index) const
{
    if (option.state & QStyle::State_Selected)
    {
        paintRowBackground(
            painter,
            option,
            index,
            TokenParserWidgetManager::instance()->getColor(QLatin1String("link-primary"),
                                                           QLatin1String("#FF2C5BEB")));

        painter->setPen(
            TokenParserWidgetManager::instance()->getColor(QLatin1String("text-inverse-accent"),
                                                           QLatin1String("#FFFAFAFB")));
    }
    else
    {
        auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>();
        if (sync->getError())
        {
            paintRowBackground(
                painter,
                option,
                index,
                TokenParserWidgetManager::instance()->getColor(QLatin1String("notification-error"),
                                                               QLatin1String("#FFFFE4E8")));

            painter->setPen(
                TokenParserWidgetManager::instance()->getColor(QLatin1String("text-error"),
                                                               QLatin1String("#FFE31B57")));
        }
        else
        {
            painter->setPen(
                TokenParserWidgetManager::instance()->getColor(QLatin1String("text-primary"),
                                                               QLatin1String("#ff303233")));
        }
    }

    QStyledItemDelegate::paint(painter, option, index);
}

void BackgroundColorDelegate::paintRowBackground(QPainter* painter,
                                                 const QStyleOptionViewItem& option,
                                                 const QModelIndex& index,
                                                 const QColor& color) const
{
    const int ENABLED_COLUMN_INDEX = 0;
    const int MENU_COLUMN_INDEX = index.model()->columnCount() - 1;
    const double RADIUS_SQUARE_PERCENTATGE = 0.20;

    auto optionRect = option.rect;
    auto radius = optionRect.width() * RADIUS_SQUARE_PERCENTATGE;

    if (index.column() == ENABLED_COLUMN_INDEX) // first column will have the left squares rounded.
    {
        QPainterPath roundRectPath;
        roundRectPath.moveTo(optionRect.left() + 2 * radius, optionRect.top());
        roundRectPath
            .arcTo(optionRect.left(), optionRect.top(), radius * 2.0, radius * 2.0, 90.0, 90.0);
        roundRectPath.lineTo(optionRect.left(), optionRect.bottom() - radius);
        roundRectPath.arcTo(optionRect.left(),
                            (optionRect.bottom() + 1) - radius * 2.0,
                            radius * 2.0,
                            radius * 2.0,
                            180.0,
                            90.0);
        roundRectPath.lineTo(optionRect.right() + 1, optionRect.bottom() + 1);
        roundRectPath.lineTo(optionRect.right() + 1, optionRect.top());
        roundRectPath.closeSubpath();

        painter->fillPath(roundRectPath, color);
    }
    else if (index.column() ==
             MENU_COLUMN_INDEX) // last column will have the right squares rounded.
    {
        QPainterPath roundRectPath;
        roundRectPath.moveTo(optionRect.left(), optionRect.top());
        roundRectPath.lineTo(optionRect.left(), optionRect.bottom() + 1);
        roundRectPath.lineTo(optionRect.right() + 1 - radius, optionRect.bottom() + 1);
        roundRectPath.arcTo(optionRect.right() + 1 - radius * 2.0,
                            optionRect.bottom() + 1 - radius * 2.0,
                            radius * 2.0,
                            radius * 2.0,
                            270.0,
                            90.0);
        roundRectPath.lineTo(optionRect.right() + 1, optionRect.top() + radius);
        roundRectPath.arcTo(optionRect.right() + 1 - 2.0 * radius,
                            optionRect.top(),
                            2.0 * radius,
                            2.0 * radius,
                            0.0,
                            90.0);
        roundRectPath.closeSubpath();

        painter->fillPath(roundRectPath, color);
    }
    else
    {
        painter->fillRect(option.rect, color);
    }
}

/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MenuItemDelegate::MenuItemDelegate(QObject* parent):
    BackgroundColorDelegate(parent)
{}

void MenuItemDelegate::paint(QPainter* painter,
                             const QStyleOptionViewItem& option,
                             const QModelIndex& index) const
{
    QStyleOptionViewItem opt(option);
    opt.decorationAlignment = index.data(Qt::TextAlignmentRole).value<Qt::Alignment>();
    opt.decorationPosition = QStyleOptionViewItem::Top;
    if (!opt.state.testFlag(QStyle::State_Enabled) && opt.state.testFlag(QStyle::State_Selected))
    {
        opt.state.setFlag(QStyle::State_Enabled, true);
    }
    BackgroundColorDelegate::paint(painter, opt, index);
}

const int ICON_SPACE_SIZE = 60;

IconMiddleDelegate::IconMiddleDelegate(QObject* parent):
    BackgroundColorDelegate(parent)
{}

void IconMiddleDelegate::paint(QPainter* painter,
                               const QStyleOptionViewItem& option,
                               const QModelIndex& index) const
{
    BackgroundColorDelegate::paint(painter, option, index);

    QStyleOptionViewItem opt(option);
    opt.decorationAlignment = Qt::AlignVCenter | Qt::AlignHCenter;
    opt.decorationPosition = QStyleOptionViewItem::Top;
    QRect rect = option.rect;
    rect.setRight(ICON_SPACE_SIZE);
    QIcon icon = index.data(Qt::DecorationRole).value<QIcon>();

    QIcon::Mode iconMode = QIcon::Normal;
    if (option.state & QStyle::State_Selected)
    {
        iconMode = QIcon::Selected;
    }
    icon.paint(painter, rect, Qt::AlignVCenter | Qt::AlignHCenter, iconMode);
    QString text = index.data(Qt::DisplayRole).toString();
    QRect textRect = option.rect;
    textRect.setLeft(rect.right());
    QTextOption textOption;
    textOption.setWrapMode(QTextOption::WrapMode::NoWrap);
    textOption.setAlignment(Qt::AlignVCenter);

    QString elidedText = option.fontMetrics.elidedText(text, Qt::ElideMiddle, textRect.width());

    painter->drawText(textRect, elidedText, textOption);
}

void IconMiddleDelegate::initStyleOption(QStyleOptionViewItem* option,
                                         const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
    option->icon = QIcon();
    option->text = QString();
}

bool IconMiddleDelegate::helpEvent(QHelpEvent* event,
                                   QAbstractItemView* view,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index)
{
    auto sync = index.data(Qt::UserRole).value<std::shared_ptr<SyncSettings>>();
    if (sync->getError())
    {
        QRect rect = option.rect;
        rect.setRight(ICON_SPACE_SIZE);
        if (rect.contains(event->pos()))
        {
            QToolTip::showText(event->globalPos(),
                               index.data(SyncItemModel::ErrorTooltipRole).toString());
            return true;
        }
    }

    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

///////////////////////////////
ElideMiddleDelegate::ElideMiddleDelegate(QObject* parent):
    BackgroundColorDelegate(parent)
{}

void ElideMiddleDelegate::initStyleOption(QStyleOptionViewItem* option,
                                          const QModelIndex& index) const
{
    QStyledItemDelegate::initStyleOption(option, index);
#ifdef Q_OS_MACOS
    // On the stylesheet, the header is moved 6 pixels to the left, so we use this magical number
    // To align the header with the cell
    option->rect.setLeft(option->rect.left() - 2);
#endif
    option->features = option->features & ~QStyleOptionViewItem::WrapText;
    option->textElideMode = Qt::ElideMiddle;
}
