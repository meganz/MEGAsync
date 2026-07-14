#include "NodeSelectorTreeView.h"

#include "ArrowTooltip.h"
#include "CreateRemoveSyncsManager.h"
#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaDelegateHoverManager.h"
#include "MegaMenuItemAction.h"
#include "NodeSelector.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorViewStyle.h"
#include "Platform.h"
#include "ServiceUrls.h"
#include "ThemeManager.h"
#include "Utilities.h"

#include <QDrag>
#include <QHelpEvent>
#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>
#include <QUrl>

QList<mega::MegaHandle> NodeSelectorTreeView::mCopiedHandles = QList<mega::MegaHandle>();
const int NodeSelectorTreeView::ROW_SIDE_MARGIN = 0;
const int NodeSelectorTreeView::ROW_RIGHT_MARGIN = 0;
const int NodeSelectorTreeView::ROW_VERTICAL_MARGIN = 4;
const qreal NodeSelectorTreeView::ROW_RADIUS = 4.0;

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent):
    LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>(parent),
    mAllowContextMenu(false),
    mAllowNewFolderContextMenuItem(false),
    mRootIndexReadOnly(false),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    setHeader(new NodeSelectorHeaderView(Qt::Horizontal, this));

    installEventFilter(this);

    // Copy paste actions
    mCopyShortcut = new QShortcut(QKeySequence(QKeySequence::Copy), this);
    connect(mCopyShortcut,
            &QShortcut::activated,
            this,
            &NodeSelectorTreeView::onCopyShortcutActivated);

    mPasteShortcut = new QShortcut(QKeySequence(QKeySequence::Paste), this);
    connect(mPasteShortcut,
            &QShortcut::activated,
            this,
            &NodeSelectorTreeView::onPasteShortcutActivated);

    mHoverManager = std::make_unique<MegaDelegateHoverManager>();
    mHoverManager->setView(this);

    // Pin the icon-to-text gap to a fixed value, independent of the platform style.
    auto* viewStyle = new NodeSelectorViewStyle();
    viewStyle->setParent(this);
    setStyle(viewStyle);
}

NodeSelectorTreeView::~NodeSelectorTreeView()
{
    mCopiedHandles.clear();
}

QModelIndex NodeSelectorTreeView::getIndexFromSourceModel(const QModelIndex& index) const
{
    if (!index.isValid())
    {
        return QModelIndex();
    }
    return proxyModel()->getIndexFromSource(index);
}

NodeSelectorProxyModel* NodeSelectorTreeView::proxyModel() const
{
    return static_cast<NodeSelectorProxyModel*>(model());
}

QList<MegaHandle>
    NodeSelectorTreeView::getMultiSelectionNodeHandle(const QModelIndexList& selectedRows) const
{
    QList<MegaHandle> ret;

    foreach(auto& s_index, selectedRows)
    {
        auto node = proxyModel()->getNode(s_index);
        if (node)
        {
            ret.append(node->getHandle());
        }
    }

    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    // setModel may be called repeatedly (the loading scene detaches/reattaches the
    // model). Guard against a null model and against duplicating the connection.
    if (auto* proxy = proxyModel())
    {
        connect(proxy,
                &NodeSelectorProxyModel::navigateReady,
                this,
                &NodeSelectorTreeView::onNavigateReady,
                Qt::UniqueConnection);
    }
}

void NodeSelectorTreeView::setRootIndexReadOnly(bool state)
{
    mRootIndexReadOnly = state;
}

void NodeSelectorTreeView::drawBranches(QPainter* painter,
                                        const QRect& rect,
                                        const QModelIndex& index) const
{
    auto item = qvariant_cast<NodeSelectorModelItem*>(
        index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if (item && (item->isCloudDrive() || item->isMyBackupsFolder() || item->isRubbishBin() ||
                 item->isTakenDown()))
    {
        return;
    }

    QSize iconSize(16, 16);

    QStyleOptionViewItem opt = viewOptions();

    opt.rect = rect;
    opt.rect.setHeight(iconSize.width());
    opt.rect.moveCenter(rect.center());
    opt.rect.setLeft(opt.rect.width() - iconSize.width());

    if (model()->hasChildren(index))
    {
        bool expanded(isExpanded(index));

        QString icon;
        QPixmap pixmap;

        // Get the cached pixmap. If not available, create new ones

        if (expanded)
        {
            if (!mDownChevron.isNull())
            {
                pixmap = mDownChevron;
            }
            else
            {
                icon = QLatin1String("chevron_down");
            }
        }
        else
        {
            if (!mRightChevron.isNull())
            {
                pixmap = mRightChevron;
            }
            else
            {
                icon = QLatin1String("chevron-right");
            }
        }

        if (pixmap.isNull())
        {
            pixmap = Utilities::getColoredPixmap(icon,
                                                 Utilities::AttributeType::SMALL |
                                                     Utilities::AttributeType::THIN |
                                                     Utilities::AttributeType::OUTLINE,
                                                 QLatin1String("icon-secondary"),
                                                 iconSize);

            if (expanded)
            {
                mDownChevron = pixmap;
            }
            else
            {
                mRightChevron = pixmap;
            }
        }

        painter->drawPixmap(opt.rect, pixmap);
    }
}

void NodeSelectorTreeView::drawRow(QPainter* painter,
                                   const QStyleOptionViewItem& option,
                                   const QModelIndex& index) const
{
    if (!selectionModel())
    {
        return;
    }

    auto delegate(itemDelegate());

    if (auto nodeSelectorDelegate = qobject_cast<NodeSelectorDelegate*>(delegate))
    {
        QString token;
        // A drop target is highlighted like a selected row, but without touching the
        // selection model, so the multi-selection being dragged stays visible.
        if (selectionModel()->isSelected(index) || index == mDropHoverIndex)
        {
            token = QLatin1String("surface-2");
        }
        else if (nodeSelectorDelegate->isHoverStateSet(index))
        {
            token = QLatin1String("surface-1");
        }

        if (!token.isEmpty())
        {
            painter->save();
            auto pen(painter->pen());
            painter->setPen(pen);
            painter->setRenderHint(QPainter::RenderHint::Antialiasing, true);

            QPainterPath path;
            auto rect(option.rect);
            rect.adjust(ROW_SIDE_MARGIN,
                        ROW_VERTICAL_MARGIN,
                        -ROW_RIGHT_MARGIN,
                        -ROW_VERTICAL_MARGIN);
            path.addRoundedRect(rect, ROW_RADIUS, ROW_RADIUS);
            painter->fillPath(path, TokenParserWidgetManager::instance()->getColor(token));

            painter->restore();
        }
    }

    QStyleOptionViewItem auxOpt(option);
    auxOpt.state.setFlag(QStyle::State_Selected, false);
    auxOpt.palette.setColor(QPalette::ColorGroup::Active,
                            QPalette::ColorRole::Highlight,
                            Qt::transparent);
    auxOpt.palette.setColor(QPalette::ColorGroup::Inactive,
                            QPalette::ColorRole::Highlight,
                            Qt::transparent);

    QTreeView::drawRow(painter, auxOpt, index);
}

void NodeSelectorTreeView::mousePressEvent(QMouseEvent* event)
{
    mTooltip.hide();

#ifndef Q_OS_MACOS
    auto index = indexAt(event->pos());
    auto expanded(isExpanded(index));
#endif

    QTreeView::mousePressEvent(event);

#ifndef Q_OS_MACOS
    if (expanded != isExpanded(index))
    {
        selectFromMouseEvent(index, event->modifiers());
    }
#endif
}

void NodeSelectorTreeView::mouseReleaseEvent(QMouseEvent* event)
{
#ifdef Q_OS_MACOS
    auto index = indexAt(event->pos());
    auto expanded(isExpanded(index));
#endif

    QTreeView::mouseReleaseEvent(event);

#ifdef Q_OS_MACOS
    if (expanded != isExpanded(index))
    {
        selectFromMouseEvent(index, event->modifiers());
    }
#endif
}

bool NodeSelectorTreeView::viewportEvent(QEvent* event)
{
    // Every NodeSelector row tooltip is rendered through the styled tooltip, reading the model's
    // ToolTipRole (the sync proxy returns a whole-row access tooltip there for read-only folders).
    const bool consumed = mTooltip.handleViewportEvent(
        event,
        this,
        mTooltipIndex,
        [this](const QPoint& pos)
        {
            const QModelIndex index = indexAt(pos);
            const QString text = index.data(Qt::ToolTipRole).toString();
            const int anchorBottomGlobalY =
                text.isEmpty() ? 0 : viewport()->mapToGlobal(visualRect(index).bottomLeft()).y();
            return NodeSelectorStyledTooltip::Target<QPersistentModelIndex>{
                text,
                anchorBottomGlobalY,
                QPersistentModelIndex(index)};
        });

    // Call the loading-scene base (not QTreeView directly) so viewport-event blocking during
    // the loading scene is preserved.
    return consumed ||
           LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>::viewportEvent(event);
}

void NodeSelectorStyledTooltip::show(QWidget* parent,
                                     const QString& text,
                                     const QPoint& globalCursorPos,
                                     int anchorBottomGlobalY)
{
    hide();

    mTooltip = new ArrowTooltip(parent);
    mTooltip->setFontSize(QLatin1String("caption"), false); // Caption, Regular
    mTooltip->setText(text);
    mTooltip->setArrow(ArrowTooltip::Arrow::Up);

    // Centered on the cursor, just below the hovered element, arrow pointing up at it.
    const int x = globalCursorPos.x() - mTooltip->width() / 2;
    const int y = anchorBottomGlobalY + V_GAP;
    mTooltip->move(x, y);
    mTooltip->show();
}

void NodeSelectorStyledTooltip::hide()
{
    if (mTooltip)
    {
        mTooltip->close();
        mTooltip = nullptr;
    }
}

void NodeSelectorTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
    {
        QAbstractItemView::mouseDoubleClickEvent(event);
    }
}

void NodeSelectorTreeView::keyPressEvent(QKeyEvent* event)
{
    if (!selectionModel())
    {
        return;
    }

    QModelIndexList indexes = selectedRows();

    if (indexes.isEmpty())
    {
        return;
    }

    static QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                           << Qt::Key_Plus << Qt::Key_Minus;

    if (!bannedFromRootKeyList.contains(event->key()) || !indexes.contains(cdRootIndex))
    {
        if (mAllowNewFolderContextMenuItem && event->key() == Qt::Key_F2)
        {
            // Mimic the context menu: rename is only offered for a single
            // selected item which can be renamed
            if (indexes.size() == 1)
            {
                auto item = proxyModel()->getMegaModel()->getItemByIndex(
                    proxyModel()->mapToSource(indexes.first()));
                if (item && item->canBeRenamed())
                {
                    renameNode();
                }
            }
        }
        else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            emit enterKeyPressed();
        }
        else if (mAllowNewFolderContextMenuItem && event->key() == Qt::Key_Delete)
        {
            // You cannot remove the root index
            if (!indexes.contains(rootIndex()))
            {
                auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
                if (proxyModel->canBeDeleted())
                {
                    auto selectedIndexes = selectedRows();
                    auto selectionHandles(getMultiSelectionNodeHandle(selectedIndexes));

                    auto deletionTypeOpt = areAllEligibleForDeletion(selectedIndexes);
                    if (deletionTypeOpt.has_value())
                    {
                        auto deletionType(deletionTypeOpt.value());
                        if (deletionType == DeletionType::LEAVE_SHARE)
                        {
                            emit leaveShareClicked(selectionHandles);
                        }
                        else
                        {
                            bool showConfirmationDialog(deletionType !=
                                                        DeletionType::MOVE_TO_RUBBISH);
                            deleteNode(selectionHandles,
                                       deletionType == DeletionType::MOVE_TO_RUBBISH ? false : true,
                                       showConfirmationDialog);
                        }
                    }
                }
            }
        }
    }

    QTreeView::keyPressEvent(event);
}

void NodeSelectorTreeView::onCopyShortcutActivated()
{
    mCopiedHandles.clear();

    auto selectedIndexes(selectedRows());
    if (areAllEligibleForCopy(selectedIndexes))
    {
        mCopiedHandles = getMultiSelectionNodeHandle(selectedIndexes);
    }
}

void NodeSelectorTreeView::onPasteShortcutActivated()
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndex pasteIndex =
        proxyModel->getMegaModel()->rootIndex(proxyModel->mapToSource(rootIndex()));

    if (proxyModel->getMegaModel()->canPasteNodes(mCopiedHandles, pasteIndex))
    {
        proxyModel->getMegaModel()->pasteNodes(mCopiedHandles, pasteIndex);
    }
}

void NodeSelectorTreeView::onPasteClicked(const QModelIndex& selectedIndex)
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
    QModelIndex targetIndex(selectedIndex);

    if (!targetIndex.isValid())
    {
        targetIndex = rootIndex();
    }

    // Double check in case the scenario has changed
    if (proxyModel->getMegaModel()->canPasteNodes(mCopiedHandles, targetIndex))
    {
        proxyModel->getMegaModel()->pasteNodes(mCopiedHandles,
                                               proxyModel->mapToSource(targetIndex));
    }
}

void NodeSelectorTreeView::addShareLinkMenuAction(QMap<int, QAction*>& actions,
                                                  const QModelIndexList& selectedIndexes,
                                                  const QList<mega::MegaHandle>& selectionHandles)
{
    if (areAllEligibleForLinkShare(selectedIndexes))
    {
        auto megaLinkAction(
            new MegaMenuItemAction(tr("Share link"),
                                   Utilities::getPixmapName(QLatin1String("link-01"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));
        connect(megaLinkAction,
                &QAction::triggered,
                this,
                [this, selectionHandles]()
                {
                    emit getMegaLinkClicked(selectionHandles);
                });
        actions.insert(ActionsOrder::MEGA_LINK, megaLinkAction);
    }
}

void NodeSelectorTreeView::addPasteMenuAction(QMap<int, QAction*>& actions,
                                              const QModelIndexList& selectedIndexes)
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    if (selectedIndexes.size() == 1)
    {
        if (!mCopiedHandles.isEmpty() && proxyModel->getMegaModel()->canPasteNodes(
                                             mCopiedHandles,
                                             proxyModel->mapToSource(selectedIndexes.first())))
        {
            auto pasteAction(new MegaMenuItemAction(
                tr("Paste"),
                Utilities::getPixmapName(QLatin1String("clipboard"),
                                         Utilities::AttributeType::SMALL |
                                             Utilities::AttributeType::THIN |
                                             Utilities::AttributeType::OUTLINE,
                                         false)));
            connect(pasteAction,
                    &QAction::triggered,
                    this,
                    [this, selectedIndexes]()
                    {
                        onPasteClicked(selectedIndexes.first());
                    });
            actions.insert(ActionsOrder::PASTE, pasteAction);
        }
    }
}

void NodeSelectorTreeView::addNewFolderMenuAction(QMap<int, QAction*>& actions)
{
    if (!mRootIndexReadOnly)
    {
        auto newfolderAction(
            new MegaMenuItemAction(tr("New folder"),
                                   Utilities::getPixmapName(QLatin1String("folder-plus-01"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));

        connect(newfolderAction,
                &QAction::triggered,
                this,
                &NodeSelectorTreeView::newFolderClicked);

        actions.insert(ActionsOrder::NEW_FOLDER, newfolderAction);
    }
}

void NodeSelectorTreeView::addDownloadMenuAction(QMap<int, QAction*>& actions,
                                                 const QModelIndexList& selectedIndexes,
                                                 const QList<MegaHandle>& selectionHandles)
{
    if (areAllEligibleForDownload(selectedIndexes))
    {
        auto downloadAction(
            new MegaMenuItemAction(tr("Download"),
                                   Utilities::getPixmapName(QLatin1String("arrow-down-circle"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));
        connect(downloadAction,
                &QAction::triggered,
                this,
                [selectionHandles]()
                {
                    MegaSyncApp->downloadACtionClickedWithHandles(selectionHandles);
                });
        actions.insert(ActionsOrder::DOWNLOAD, downloadAction);
    }
}

void NodeSelectorTreeView::addUploadMenuAction(QMap<int, QAction*>& actions)
{
    auto uploadAction(new MegaMenuItemAction(
        tr("Upload"),
        Utilities::getPixmapName(QLatin1String("arrow-up-circle"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(uploadAction, &QAction::triggered, this, &NodeSelectorTreeView::uploadClicked);
    actions.insert(ActionsOrder::UPLOAD, uploadAction);
}

void NodeSelectorTreeView::addRestoreMenuAction(QMap<int, QAction*>& actions,
                                                const QModelIndexList& selectedIndexes,
                                                const QList<MegaHandle>& selectionHandles)
{
    if (areAllEligibleForRestore(selectedIndexes))
    {
        auto restoreAction(
            new MegaMenuItemAction(tr("Restore"),
                                   Utilities::getPixmapName(QLatin1String("clock-rotate"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));
        connect(restoreAction,
                &QAction::triggered,
                this,
                [this, selectionHandles]()
                {
                    restore(selectionHandles);
                });
        actions.insert(ActionsOrder::RESTORE, restoreAction);
    }
}

void NodeSelectorTreeView::addRenameMenuAction(QMap<int, QAction*>& actions,
                                               const QModelIndex& index)
{
    auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
    if (!item || !item->canBeRenamed())
    {
        return;
    }

    auto renameAction(new MegaMenuItemAction(
        tr("Rename"),
        Utilities::getPixmapName(QLatin1String("pen-2"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(renameAction,
            &QAction::triggered,
            this,
            [this]()
            {
                renameNode();
            });
    actions.insert(ActionsOrder::RENAME, renameAction);
}

void NodeSelectorTreeView::addSyncMenuActions(QMap<int, QAction*>& actions,
                                              const QModelIndex& index,
                                              MegaHandle selectedHandle)
{
    auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
    if (!item || item->getNode()->isFile())
    {
        return;
    }

    auto itemStatus = item->getStatus();

    if (itemStatus == NodeSelectorModelItem::Status::NONE && item->isSyncable())
    {
        auto syncAction(
            new MegaMenuItemAction(tr("Sync"),
                                   Utilities::getPixmapName(QLatin1String("sync-01"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));
        connect(syncAction,
                &QAction::triggered,
                this,
                [this, selectedHandle]()
                {
                    CreateRemoveSyncsManager::addSync(
                        SyncInfo::SyncOrigin::CLOUD_DRIVE_DIALOG_ORIGIN,
                        selectedHandle,
                        QString(),
                        Utilities::getTopParent<NodeSelector>(this));
                });
        actions.insert(ActionsOrder::SYNC, syncAction);
    }
    else if (itemStatus == NodeSelectorModelItem::Status::SYNC)
    {
        auto unsyncAction(
            new MegaMenuItemAction(tr("Stop syncing"),
                                   Utilities::getPixmapName(QLatin1String("x"),
                                                            Utilities::AttributeType::SMALL |
                                                                Utilities::AttributeType::THIN |
                                                                Utilities::AttributeType::OUTLINE,
                                                            false)));
        connect(unsyncAction,
                &QAction::triggered,
                this,
                [this, selectedHandle]()
                {
                    CreateRemoveSyncsManager::removeSync(selectedHandle, this);
                });
        actions.insert(ActionsOrder::UNSYNC, unsyncAction);
    }
}

void NodeSelectorTreeView::setAllowNewFolderContextMenuItem(bool newAllowNewFolderContextMenuItem)
{
    mAllowNewFolderContextMenuItem = newAllowNewFolderContextMenuItem;
}

void NodeSelectorTreeView::setAllowContextMenu(bool newAllowContextMenu)
{
    mAllowContextMenu = newAllowContextMenu;
}

void NodeSelectorTreeView::addDisputeTakedownMenuAction(QMap<int, QAction*>& actions)
{
    auto disputeAction(new MegaMenuItemAction(
        tr("Dispute takedown"),
        Utilities::getPixmapName(QLatin1String("message-alert"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(disputeAction,
            &QAction::triggered,
            []()
            {
                Utilities::openUrl(ServiceUrls::getDisputeTakenDownLink());
            });
    actions.insert(ActionsOrder::DISPUTE_TAKEDOWN, disputeAction);
}

void NodeSelectorTreeView::addDeleteMenuAction(QMap<int, QAction*>& actions,
                                               QList<MegaHandle> selectionHandles)
{
    auto deleteAction(new MegaMenuItemAction(
        tr("Move to Rubbish bin"),
        Utilities::getPixmapName(QLatin1String("trash"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(deleteAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                deleteNode(selectionHandles, false, false);
            });
    actions.insert(ActionsOrder::DELETE_RUBBISH, deleteAction);
}

void NodeSelectorTreeView::addDeletePermanently(QMap<int, QAction*>& actions,
                                                QList<MegaHandle> selectionHandles)
{
    auto deletePermanentlyAction(new MegaMenuItemAction(
        tr("Permanently delete"),
        Utilities::getPixmapName(QLatin1String("trash"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(deletePermanentlyAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                deleteNode(selectionHandles, true);
            });
    actions.insert(ActionsOrder::DELETE_PERMANENTLY, deletePermanentlyAction);
}

void NodeSelectorTreeView::addLeaveInshare(QMap<int, QAction*>& actions,
                                           const QList<MegaHandle>& selectionHandles)
{
    auto leaveShareAction(new MegaMenuItemAction(
        tr("Leave folder"),
        Utilities::getPixmapName(QLatin1String("log-out-02"),
                                 Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                                     Utilities::AttributeType::OUTLINE,
                                 false)));
    connect(leaveShareAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                emit leaveShareClicked(selectionHandles);
            });
    actions.insert(ActionsOrder::LEAVE_SHARE, leaveShareAction);
}

void NodeSelectorTreeView::addRemoveMenuActions(QMap<int, QAction*>& actions,
                                                const QModelIndexList& selectedIndexes,
                                                const QList<mega::MegaHandle>& selectionHandles)
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    if (proxyModel->canBeDeleted())
    {
        auto deletionTypeOpt = areAllEligibleForDeletion(selectedIndexes);
        if (deletionTypeOpt.has_value())
        {
            auto deletionType(deletionTypeOpt.value());
            if (deletionType == DeletionType::LEAVE_SHARE)
            {
                addLeaveInshare(actions, selectionHandles);
            }
            else if (deletionType == DeletionType::MOVE_TO_RUBBISH)
            {
                addDeleteMenuAction(actions, selectionHandles);
            }
            else
            {
                addDeletePermanently(actions, selectionHandles);
            }
        }
    }
}

QHash<mega::MegaHandle, int>
    NodeSelectorTreeView::getNodesAccess(const QList<MegaHandle>& handles) const
{
    QHash<mega::MegaHandle, int> accessByHandle;

    for (const auto& handle: handles)
    {
        auto node = std::unique_ptr<MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if (node)
        {
            accessByHandle.insert(handle, Utilities::getNodeAccess(node.get()));
        }
    }

    return accessByHandle;
}

QModelIndexList NodeSelectorTreeView::selectedRows() const
{
    if (!selectionModel())
    {
        return QModelIndexList();
    }

    return selectionModel()->selectedRows();
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!mAllowContextMenu)
    {
        return;
    }

    if (!selectionModel())
    {
        return;
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    mega::MegaHandle clickedHandle(mega::INVALID_HANDLE);
    bool clickedEmptySpace(false);

    auto indexClicked = indexAt(event->pos());
    if (indexClicked.isValid())
    {
        clickedHandle = proxyModel->getHandle(indexClicked);
    }

    // You just may click the extra row or an empty folder
    if (clickedHandle == mega::INVALID_HANDLE)
    {
        indexClicked = rootIndex();
        clickedEmptySpace = true;
        clearSelection();
    }

    showContextMenu(selectedRows(), indexClicked, clickedEmptySpace, mapToGlobal(event->pos()));
}

void NodeSelectorTreeView::showContextMenu(const QModelIndexList& selectedRowsList,
                                           const QModelIndex& clickedIndex,
                                           bool clickedEmptySpace,
                                           const QPoint& globalPos,
                                           bool ignoreEmptySpaceBlock)
{
    if (!selectionModel())
    {
        return;
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
    if (!proxyModel)
    {
        return;
    }

    QMenu customMenu(this);
    customMenu.setProperty("icon-token", QLatin1String("icon-primary"));
    customMenu.setProperty("class", QLatin1String("MegaMenu"));

    QList<mega::MegaHandle> selectionHandles;
    mega::MegaHandle clickedHandle = proxyModel->getHandle(clickedIndex);

    // If it is invalid, don´t show anything
    if (clickedHandle == mega::INVALID_HANDLE)
    {
        return;
    }

    QModelIndexList selectedIndexes = selectedRowsList;
    auto currentSelectionHandles(getMultiSelectionNodeHandle(selectedIndexes));

    if (currentSelectionHandles.contains(clickedHandle))
    {
        selectionHandles = currentSelectionHandles;
    }
    else
    {
        selectedIndexes = QModelIndexList() << clickedIndex;
        selectionHandles.append(clickedHandle);
    }

    if (!proxyModel->hasContextMenuOptions(selectedIndexes))
    {
        return;
    }

    QMap<int, QAction*> actions;
    const auto takenDownSelection = containsTakenDownItem(selectedIndexes);

    const bool offerNodeActions = !clickedEmptySpace || ignoreEmptySpaceBlock;

    if (offerNodeActions && takenDownSelection)
    {
        addDisputeTakedownMenuAction(actions);
        addRemoveMenuActions(actions, selectedIndexes, selectionHandles);
        addRestoreMenuAction(actions, selectedIndexes, selectionHandles);
    }
    else
    {
        if (offerNodeActions && areAllEligibleForCopy(selectedIndexes))
        {
            auto copyAction(new MegaMenuItemAction(
                tr("Copy"),
                Utilities::getPixmapName(QLatin1String("copy-02"),
                                         Utilities::AttributeType::SMALL |
                                             Utilities::AttributeType::THIN |
                                             Utilities::AttributeType::OUTLINE,
                                         false)));
            connect(copyAction,
                    &QAction::triggered,
                    [selectionHandles]()
                    {
                        mCopiedHandles = selectionHandles;
                    });
            actions.insert(ActionsOrder::COPY, copyAction);
        }

        // Don´t offer "Download" if the folder is empty
        if (model()->index(0, 0, rootIndex()).isValid())
        {
            addDownloadMenuAction(actions, selectedIndexes, selectionHandles);
        }

        if (clickedEmptySpace && !mRootIndexReadOnly)
        {
            addUploadMenuAction(actions);
        }

        addPasteMenuAction(actions, selectedIndexes);

        if (clickedEmptySpace)
        {
            addNewFolderMenuAction(actions);
        }

        if (offerNodeActions && !selectedIndexes.isEmpty())
        {
            if (selectedIndexes.size() == 1)
            {
                auto selectedIndex = proxyModel->mapToSource(selectedIndexes.first());

                addRenameMenuAction(actions, selectedIndex);

                if (!selectionHandles.isEmpty())
                {
                    addSyncMenuActions(actions, selectedIndex, selectionHandles.first());
                }
            }

            addRemoveMenuActions(actions, selectedIndexes, selectionHandles);
            addRestoreMenuAction(actions, selectedIndexes, selectionHandles);
            addShareLinkMenuAction(actions, selectedIndexes, selectionHandles);
        }
    }

    bool separatorPending(false);

    QMetaEnum e = QMetaEnum::fromType<ActionsOrder>();
    for (int i = 0; i < e.keyCount(); i++)
    {
        QString actionName(QString::fromUtf8(e.key(i)));
        if (actionName.contains(QLatin1String("SEPARATOR")))
        {
            separatorPending = !customMenu.actions().isEmpty();
        }
        else
        {
            auto action(actions.value(e.value(i)));
            if (action)
            {
                if (separatorPending)
                {
                    customMenu.addSeparator();
                    separatorPending = false;
                }
                customMenu.addAction(action);
            }
        }
    }

    if (!customMenu.actions().isEmpty())
    {
        customMenu.exec(globalPos);
    }
}

bool NodeSelectorTreeView::event(QEvent* event)
{
    if (event->type() == ThemeManager::ThemeChanged)
    {
        mDownChevron = QPixmap();
        mRightChevron = QPixmap();
    }

    return LoadingSceneView::event(event);
}

void NodeSelectorTreeView::startDrag(Qt::DropActions supportedActions)
{
    QModelIndexList indexes = selectedIndexes();
    if (indexes.isEmpty())
    {
        return;
    }

    auto delegate(dynamic_cast<NodeRowDelegate*>(itemDelegate()));
    if (!delegate)
    {
        return;
    }

    // Stack every selected row so the drag pixmap reflects all the items being moved.
    auto pixmap(delegate->paintForDrag(selectedRows(), this));

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = model()->mimeData(indexes);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->exec(supportedActions);
    setDropHoverIndex(QModelIndex());
}

void NodeSelectorTreeView::dragEnterEvent(QDragEnterEvent* event)
{
    if (proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dragMoveEvent(QDragMoveEvent* event)
{
    if (proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        const QModelIndex posIndex = indexAt(event->pos());

        // When dropping over a row, the target is resolved from that item; when dropping over
        // the empty area of the view, the target is the current folder (the view's root index).
        int dropRow(posIndex.row());
        int dropColumn(posIndex.column());
        QModelIndex dropParent(posIndex.parent());

        // Empty space, invalid index, take the current root index (open folder)
        if (!posIndex.isValid())
        {
            dropRow = -1;
            dropColumn = -1;
            dropParent = rootIndex();
        }

        if (!proxyModel()->canDropMimeData(event->mimeData(),
                                           Qt::MoveAction,
                                           dropRow,
                                           dropColumn,
                                           dropParent))
        {
            // Not a valid target: drop the highlight without touching the user's selection.
            setDropHoverIndex(QModelIndex());
            event->ignore();
            return;
        }

        // Highlight the drop target without mutating the selection model, so the
        // multi-selection being dragged stays visible during the drag.
        setDropHoverIndex(posIndex);

        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dragLeaveEvent(QDragLeaveEvent* event)
{
    setDropHoverIndex(QModelIndex());
    LoadingSceneView::dragLeaveEvent(event);
}

void NodeSelectorTreeView::setDropHoverIndex(const QModelIndex& index)
{
    // indexAt() returns the index of the cell under the cursor, but drawRow()
    // compares against the row's column-0 index. Normalize here so the highlight
    // is column-independent, like the Select|Rows selection it replaced.
    const QModelIndex rowIndex = index.isValid() ? index.sibling(index.row(), 0) : index;

    if (mDropHoverIndex == rowIndex)
    {
        return;
    }

    mDropHoverIndex = rowIndex;
    viewport()->update();
}

void NodeSelectorTreeView::dropEvent(QDropEvent* event)
{
    setDropHoverIndex(QModelIndex());

    if (proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // Get the list of URLs
        QList<QUrl> urlList = event->mimeData()->urls();
        if (!urlList.isEmpty())
        {
            // get drop index
            QModelIndex dropIndex = indexAt(event->pos());

            auto dialog = DialogOpener::findDialog<NodeSelector>();

            if (dialog)
            {
                // get the node handle of the drop index from the proxy model
                auto node = getDropNode(dropIndex);

                // If no node, try to get the parent node
                if (!node)
                {
                    auto parentIndex(dropIndex.parent());
                    node = getDropNode(parentIndex);
                }

                if (node && !node->isTakenDown())
                {
                    MegaSyncApp->uploadFilesToNode(urlList,
                                                   node->getHandle(),
                                                   mega::MegaApi::PITAG_TRIGGER_DRAG_AND_DROP,
                                                   dialog->getDialog());

                    // Accept the drop
                    QTreeView::dropEvent(event);
                    event->acceptProposedAction();
                    return;
                }
            }
        }
        else
        {
            // If there is no urlList, it may be a internal move
            LoadingSceneView::dropEvent(event);
            return;
        }
    }

    // By default, don´t accept the drop
    event->ignore();
}

std::shared_ptr<MegaNode> NodeSelectorTreeView::getDropNode(const QModelIndex& dropIndex)
{
    if (!dropIndex.isValid())
    {
        const auto root = rootIndex();
        NodeSelectorModelItem* item(nullptr);
        if (!root.isValid())
        {
            QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
            item = NodeSelectorModel::getItemByIndex(cdRootIndex);
        }
        else
        {
            item = NodeSelectorModel::getItemByIndex(root);
        }
        return item ? item->getNode() : nullptr;
    }
    auto node = proxyModel()->getNode(dropIndex);
    if (!node || node->isFolder())
    {
        return node;
    }
    return std::shared_ptr<MegaNode>(mMegaApi->getParentNode(node.get()));
}

bool NodeSelectorTreeView::areAllEligibleForCopy(const QModelIndexList& selectedIndexes) const
{
    if (!proxyModel()->getMegaModel()->canCopyNodes() || selectedIndexes.isEmpty())
    {
        return false;
    }

    foreach(const auto& index, selectedIndexes)
    {
        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (!item || item->isSpecialNode() || item->isTakenDown())
        {
            return false;
        }
    }

    return true;
}

std::optional<NodeSelectorTreeView::DeletionType>
    NodeSelectorTreeView::areAllEligibleForDeletion(const QModelIndexList& selectedIndexes) const
{
    auto removableItems(selectedIndexes.size());
    std::optional<NodeSelectorTreeView::DeletionType> type;

    for (const auto& index: selectedIndexes)
    {
        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (item)
        {
            auto node = item->getNode();
            if (node && !item->isSpecialNode())
            {
                std::optional<DeletionType> currentNodeDeletionType;
                if (item->isInRubbishBin())
                {
                    currentNodeDeletionType = DeletionType::PERMANENT_REMOVE;
                }
                else if (node->isInShare())
                {
                    currentNodeDeletionType = DeletionType::LEAVE_SHARE;
                }
                else
                {
                    if (item->getNodeAccess() >= mega::MegaShare::ACCESS_FULL)
                    {
                        currentNodeDeletionType = DeletionType::MOVE_TO_RUBBISH;
                    }
                }

                if (!currentNodeDeletionType.has_value())
                {
                    break;
                }

                if (!type.has_value())
                {
                    type = currentNodeDeletionType.value();
                }
                // We cannot remove two items of different type
                else if (type.value() != currentNodeDeletionType.value())
                {
                    return std::nullopt;
                }

                removableItems--;
            }
        }
    }

    return removableItems == 0 ? type : std::nullopt;
}

bool NodeSelectorTreeView::areAllEligibleForLinkShare(const QModelIndexList& selectedIndexes) const
{
    auto result(true);

    for (const auto& index: selectedIndexes)
    {
        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (!item || ((item->getNodeAccess() != mega::MegaShare::ACCESS_OWNER) ||
                      item->isSpecialNode() || item->isInRubbishBin() || item->isTakenDown()))
        {
            result = false;
            break;
        }
    }

    return result;
}

bool NodeSelectorTreeView::areAllEligibleForRestore(const QModelIndexList& selectedIndexes) const
{
    if (selectedIndexes.isEmpty())
    {
        return false;
    }

    for (const auto& index: selectedIndexes)
    {
        if (!index.isValid())
        {
            return false;
        }

        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (!item)
        {
            return false;
        }

        auto node(item->getNode());

        if (node && item->isInRubbishBin())
        {
            std::unique_ptr<mega::MegaNode> parentNode(
                mMegaApi->getNodeByHandle(node->getParentHandle()));
            auto previousParentNode =
                std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(node->getRestoreHandle()));

            if (!previousParentNode || mMegaApi->isInRubbish(previousParentNode.get()))
            {
                return false;
            }
        }
        else
        {
            return false;
        }
    }

    return true;
}

bool NodeSelectorTreeView::areAllEligibleForDownload(const QModelIndexList& selectedIndexes) const
{
    if (selectedIndexes.isEmpty())
    {
        return false;
    }

    for (const auto& index: selectedIndexes)
    {
        if (!index.isValid())
        {
            return false;
        }

        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (!item || item->isInRubbishBin() || item->isTakenDown())
        {
            return false;
        }
    }

    return true;
}

void NodeSelectorTreeView::deleteNode(const QList<MegaHandle>& handles,
                                      bool permanently,
                                      bool showConfirmationMessageBox)
{
    emit deleteNodeClicked(handles, permanently, showConfirmationMessageBox);
}

void NodeSelectorTreeView::renameNode()
{
    emit renameNodeClicked();
}

void NodeSelectorTreeView::restore(const QList<mega::MegaHandle>& handles)
{
    emit restoreClicked(handles);
}

bool NodeSelectorTreeView::containsTakenDownItem(const QModelIndexList& selectedIndexes) const
{
    for (const auto& index: selectedIndexes)
    {
        auto item = proxyModel()->getMegaModel()->getItemByIndex(index);
        if (item && item->isTakenDown())
        {
            return true;
        }
    }

    return false;
}

void NodeSelectorTreeView::onNavigateReady(const QModelIndex& index)
{
    if (index.isValid())
    {
        // Loading finished
        proxyModel()->getMegaModel()->sendBlockUiSignal(false);

        QPoint point = visualRect(index).center();
        QMouseEvent mouseEvent(QEvent::MouseButtonDblClick,
                               point,
                               QPoint(),
                               Qt::LeftButton,
                               Qt::LeftButton,
                               Qt::NoModifier);
        mouseDoubleClickEvent(&mouseEvent);
    }
}

void NodeSelectorTreeView::selectFromMouseEvent(const QModelIndex& index,
                                                Qt::KeyboardModifiers modifiers)
{
    QItemSelectionModel* sel = selectionModel();

    if (modifiers & Qt::ControlModifier)
    {
        sel->select(index, QItemSelectionModel::Toggle | QItemSelectionModel::Rows);
    }
    else if (modifiers & Qt::ShiftModifier)
    {
        QModelIndex current = sel->currentIndex();
        QItemSelection selection(current, index);
        sel->select(selection, QItemSelectionModel::Select | QItemSelectionModel::Rows);
    }
    else
    {
        sel->select(index, QItemSelectionModel::ClearAndSelect | QItemSelectionModel::Rows);
    }

    sel->setCurrentIndex(index, QItemSelectionModel::NoUpdate);
}

///////////////////////////////////////////////////////////////////////////////
// NodeSelectorHeaderView
///////////////////////////////////////////////////////////////////////////////

NodeSelectorHeaderView::NodeSelectorHeaderView(Qt::Orientation orientation, QWidget* parent):
    QHeaderView(orientation, parent)
{}

QPixmap NodeSelectorHeaderView::sortArrowPixmap(Qt::SortOrder order) const
{
    auto& cachedPixmap = order == Qt::AscendingOrder ? mAscendingSortArrow : mDescendingSortArrow;

    if (cachedPixmap.isNull())
    {
        cachedPixmap = Utilities::getColoredPixmap(
            order == Qt::AscendingOrder ? QLatin1String("arrow-up") : QLatin1String("arrow-down"),
            Utilities::AttributeType::SMALL | Utilities::AttributeType::THIN |
                Utilities::AttributeType::OUTLINE,
            QLatin1String("icon-secondary"),
            QSize(16, 16));
    }

    return cachedPixmap;
}

void NodeSelectorHeaderView::setNonInteractiveSections(const QSet<int>& sections)
{
    if (mNonInteractiveSections != sections)
    {
        mNonInteractiveSections = sections;
        viewport()->update();
    }
}

void NodeSelectorHeaderView::paintSection(QPainter* painter,
                                          const QRect& rect,
                                          int logicalIndex) const
{
    if (!rect.isValid())
    {
        return;
    }

    const bool sectionIsSorted = isSortIndicatorShown() && sortIndicatorSection() == logicalIndex;

    if (!mNonInteractiveSections.contains(logicalIndex))
    {
        QStyleOptionHeader opt;
        initStyleOption(&opt);
        opt.rect = rect;
        opt.section = logicalIndex;
        opt.orientation = orientation();
        if (isEnabled())
        {
            opt.state |= QStyle::State_Enabled;
        }
        opt.state |= QStyle::State_Horizontal;
        opt.position = sectionPosition(logicalIndex);

        if (auto* m = model())
        {
            opt.text = m->headerData(logicalIndex, orientation(), Qt::DisplayRole).toString();
            const QVariant alignVar =
                m->headerData(logicalIndex, orientation(), Qt::TextAlignmentRole);
            opt.textAlignment =
                alignVar.isValid() ? Qt::Alignment(alignVar.toInt()) : defaultAlignment();
        }
        else
        {
            opt.textAlignment = defaultAlignment();
        }

        if (!sectionIsSorted)
        {
            style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

            QStyleOptionHeader labelOpt(opt);
            // Margin to align header to delegate label
            labelOpt.rect.adjust(3, 0, 0, 0);
            // Explicitly force vertical centering: Linux platform proxies (GTK/KDE) may
            // ignore the flag carried by initStyleOption and draw text at the section top.
            labelOpt.textAlignment =
                Qt::AlignVCenter |
                (layoutDirection() == Qt::RightToLeft ? Qt::AlignRight : Qt::AlignLeft);

            style()->drawControl(QStyle::CE_HeaderLabel, &labelOpt, painter, this);
            return;
        }

        // The QSS-driven sort indicator with `subcontrol-position: center left`
        // does not push the text rect to the right (Qt only reserves space when
        // the indicator sits on the default right side). Draw the arrow manually
        // so it stays tokenized and fixed at 16x16, then shift the label rect.
        opt.sortIndicator = QStyleOptionHeader::None;
        style()->drawControl(QStyle::CE_HeaderSection, &opt, painter, this);

        constexpr int sortArrowSize = 16;
        constexpr int sortArrowMargin = 0;
        constexpr int sortArrowSpacing = 4;
        const bool isRtl = layoutDirection() == Qt::RightToLeft;

        QRect arrowRect(0,
                        rect.top() + (rect.height() - sortArrowSize) / 2,
                        sortArrowSize,
                        sortArrowSize);
        if (isRtl)
        {
            arrowRect.moveRight(rect.right() - sortArrowMargin);
        }
        else
        {
            arrowRect.moveLeft(rect.left() + sortArrowMargin);
        }

        painter->drawPixmap(arrowRect, sortArrowPixmap(sortIndicatorOrder()));

        QStyleOptionHeader labelOpt(opt);
        // Force vertical centering explicitly: on Linux, platform proxies (GTK/KDE) can
        // override CE_HeaderLabel and ignore the alignment set by initStyleOption.
        labelOpt.textAlignment = Qt::AlignVCenter | (isRtl ? Qt::AlignRight : Qt::AlignLeft);
        if (isRtl)
        {
            labelOpt.rect.adjust(0, 0, -(sortArrowSize + sortArrowMargin + sortArrowSpacing), 0);
        }
        else
        {
            labelOpt.rect.adjust(sortArrowSize + sortArrowMargin + sortArrowSpacing, 0, 0, 0);
        }

        style()->drawControl(QStyle::CE_HeaderLabel, &labelOpt, painter, this);
        return;
    }

    QStyleOptionHeader opt;
    initStyleOption(&opt);

    opt.rect = rect;
    opt.section = logicalIndex;
    if (isEnabled())
    {
        opt.state |= QStyle::State_Enabled;
    }
    opt.state |= QStyle::State_Horizontal;
    opt.text = QString();
    opt.icon = QIcon();
    opt.sortIndicator = QStyleOptionHeader::None;
    opt.position = sectionPosition(logicalIndex);

    style()->drawControl(QStyle::CE_Header, &opt, painter, this);
}

void NodeSelectorHeaderView::mousePressEvent(QMouseEvent* event)
{
    mTooltip.hide();

    if (mNonInteractiveSections.contains(logicalIndexAt(event->pos())))
    {
        event->ignore();
        return;
    }
    QHeaderView::mousePressEvent(event);
}

void NodeSelectorHeaderView::mouseReleaseEvent(QMouseEvent* event)
{
    if (mNonInteractiveSections.contains(logicalIndexAt(event->pos())))
    {
        event->ignore();
        return;
    }
    QHeaderView::mouseReleaseEvent(event);
}

bool NodeSelectorHeaderView::event(QEvent* e)
{
    // Refresh sort icons
    if (e->type() == ThemeManager::ThemeChanged)
    {
        mAscendingSortArrow = QPixmap();
        mDescendingSortArrow = QPixmap();
    }

    return QHeaderView::event(e);
}

bool NodeSelectorHeaderView::viewportEvent(QEvent* e)
{
    // Header section tooltips ("Sort by ...") share the tree's styled tooltip.
    const bool consumed = mTooltip.handleViewportEvent(
        e,
        this,
        mTooltipSection,
        [this](const QPoint& pos)
        {
            const int section = logicalIndexAt(pos);
            const QString text =
                (model() && section >= 0) ?
                    model()->headerData(section, orientation(), Qt::ToolTipRole).toString() :
                    QString();
            const int anchorBottomGlobalY = mapToGlobal(QPoint(0, height())).y();
            return NodeSelectorStyledTooltip::Target<int>{text, anchorBottomGlobalY, section};
        });

    return consumed || QHeaderView::viewportEvent(e);
}

QStyleOptionHeader::SectionPosition NodeSelectorHeaderView::sectionPosition(int logicalIndex) const
{
    if (count() == 1)
    {
        return QStyleOptionHeader::OnlyOneSection;
    }
    if (logicalIndex == 0)
    {
        return QStyleOptionHeader::Beginning;
    }
    if (logicalIndex == count() - 1)
    {
        return QStyleOptionHeader::End;
    }
    return QStyleOptionHeader::Middle;
}
