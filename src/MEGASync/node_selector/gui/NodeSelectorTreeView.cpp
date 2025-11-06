#include "NodeSelectorTreeView.h"

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
#include "Platform.h"
#include "ThemeManager.h"

#include <QDrag>
#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

QList<mega::MegaHandle> NodeSelectorTreeView::mCopiedHandles = QList<mega::MegaHandle>();

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent):
    LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>(parent),
    mAllowContextMenu(false),
    mAllowNewFolderContextMenuItem(false),
    mRootIndexReadOnly(false),
    mMegaApi(MegaSyncApp->getMegaApi())
{
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

// Only used for single selection mode
MegaHandle NodeSelectorTreeView::getSelectedNodeHandle()
{
    MegaHandle ret = INVALID_HANDLE;

    if (selectedRows().size() == 1)
    {
        if (auto node = proxyModel()->getNode(selectedRows().first()))
        {
            ret = node->getHandle();
        }
    }
    return ret;
}

QList<MegaHandle>
    NodeSelectorTreeView::getMultiSelectionNodeHandle(const QModelIndexList& selectedRows) const
{
    QList<MegaHandle> ret;

    foreach(auto& s_index, selectedRows)
    {
        if (auto node = proxyModel()->getNode(s_index))
        {
            ret.append(node->getHandle());
        }
    }

    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel* model)
{
    QTreeView::setModel(model);
    connect(proxyModel(),
            &NodeSelectorProxyModel::navigateReady,
            this,
            &NodeSelectorTreeView::onNavigateReady);
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
    if (item && (item->isCloudDrive() || item->isVault() || item->isRubbishBin()))
    {
        return;
    }

    QStyleOptionViewItem opt = viewOptions();
    QSize iconSize(16, 16);

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
        if (selectionModel()->isSelected(index))
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
            // These are not magical numbers, they are taken from design
            // Left margin is set on the UI (also 12px)
            rect.setRight(option.rect.right() - 12);
            rect.setTop(option.rect.top() + 3);
            rect.setBottom(option.rect.bottom() - 5);
            path.addRoundedRect(rect, 4, 4);
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
        if (event->key() == Qt::Key_F2)
        {
            renameNode();
        }
        else if (event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if (!indexes.isEmpty())
            {
                if (indexes.first() == rootIndex() || indexes.size() > 1)
                {
                    emit nodeSelected();
                }
                else
                {
                    auto node = std::unique_ptr<MegaNode>(
                        mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
                    if (node)
                    {
                        emit nodeSelected();
                    }
                }
            }
        }
        else if (event->key() == Qt::Key_Delete)
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
                [selectedHandle]()
                {
                    CreateRemoveSyncsManager::addSync(
                        SyncInfo::SyncOrigin::CLOUD_DRIVE_DIALOG_ORIGIN,
                        selectedHandle);
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

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndexList selectionIndexes(selectionModel()->selectedRows());

    if (selectionIndexes.isEmpty())
    {
        auto index(proxyModel->mapFromSource(
            proxyModel->getMegaModel()->rootIndex(proxyModel->mapToSource(rootIndex()))));
        if (index.isValid())
        {
            selectionIndexes.append(index);
        }
    }

    return selectionIndexes;
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent* event)
{
    if (!mAllowContextMenu)
    {
        return;
    }

    QMenu customMenu(this);
    customMenu.setProperty("icon-token", QLatin1String("icon-primary"));
    customMenu.setProperty("class", QLatin1String("MegaMenu"));

    if (!selectionModel())
    {
        return;
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QList<mega::MegaHandle> selectionHandles;
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
        clickedHandle = proxyModel->getHandle(rootIndex());
        indexClicked = rootIndex();
        clickedEmptySpace = true;
        clearSelection();
    }

    // If it is still invalid, don´t show anything
    if (clickedHandle == mega::INVALID_HANDLE)
    {
        return;
    }

    QModelIndexList selectedIndexes = selectedRows();
    auto currentSelectionHandles(getMultiSelectionNodeHandle(selectedIndexes));

    if (currentSelectionHandles.contains(clickedHandle))
    {
        selectionHandles = currentSelectionHandles;
    }
    else
    {
        selectedIndexes = QModelIndexList() << indexClicked;
        selectionHandles.append(clickedHandle);
    }

    if (!proxyModel->hasContextMenuOptions(selectedIndexes))
    {
        return;
    }

    QMap<int, QAction*> actions;

    if (!clickedEmptySpace && areAllEligibleForCopy(selectedIndexes))
    {
        auto copyAction(
            new MegaMenuItemAction(tr("Copy"),
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

    if (!clickedEmptySpace && !selectedIndexes.isEmpty())
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

        addRestoreMenuAction(actions, selectedIndexes, selectionHandles);
        addShareLinkMenuAction(actions, selectedIndexes, selectionHandles);
        addRemoveMenuActions(actions, selectedIndexes, selectionHandles);
    }

    QAction* lastActionAdded(nullptr);

    QMetaEnum e = QMetaEnum::fromType<ActionsOrder>();
    for (int i = 0; i < e.keyCount(); i++)
    {
        QString actionName(QString::fromUtf8(e.key(i)));
        if (actionName.contains(QLatin1String("SEPARATOR")))
        {
            if (lastActionAdded)
            {
                customMenu.addSeparator();
            }
        }
        else
        {
            auto action(actions.value(e.value(i)));
            if (action)
            {
                lastActionAdded = action;
                customMenu.addAction(action);
            }
        }
    }

    if (!customMenu.actions().isEmpty())
    {
        customMenu.exec(mapToGlobal(event->pos()));
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

    // Paint only the first column cell
    QModelIndex index = indexes.first().sibling(indexes.first().row(), 0);
    auto pixmap(delegate->paintForDrag(index, this));

    QDrag* drag = new QDrag(this);
    QMimeData* mimeData = model()->mimeData(indexes);
    drag->setMimeData(mimeData);
    drag->setPixmap(pixmap);
    drag->exec(supportedActions);
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
        QModelIndex dropIndex = indexAt(event->pos());
        if (!dropIndex.isValid())
        {
            dropIndex = rootIndex();
        }

        // clear selection and select only the drop index
        selectionModel()->clearSelection();

        if (!proxyModel()->canDropMimeData(event->mimeData(), Qt::MoveAction, -1, -1, dropIndex))
        {
            event->ignore();
            return;
        }

        selectionModel()->select(indexAt(event->pos()),
                                 QItemSelectionModel::Select | QItemSelectionModel::Rows);

        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dropEvent(QDropEvent* event)
{
    if (proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        QModelIndex dropIndex = indexAt(event->pos());

        // Get the list of URLs
        QList<QUrl> urlList = event->mimeData()->urls();
        if (!urlList.isEmpty())
        {
            auto dialog = DialogOpener::findDialog<NodeSelector>();

            // get the node handle of the drop index from the proxy model
            auto node = getDropNode(dropIndex);
            if (node)
            {
                MegaSyncApp->uploadFilesToNode(urlList, node->getHandle(), dialog->getDialog());
            }
            else
            {
                auto parentIndex(dropIndex.parent());
                auto parentNode = getDropNode(parentIndex);
                if (parentNode)
                {
                    MegaSyncApp->uploadFilesToNode(urlList,
                                                   parentNode->getHandle(),
                                                   dialog->getDialog());
                }
                else
                {
                    event->ignore();
                    return;
                }
            }
        }

        QTreeView::dropEvent(event);
        event->acceptProposedAction();
    }
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
        if (!item || item->isSpecialNode())
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
                      item->isSpecialNode() || item->isInRubbishBin()))
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
        if (!item || item->isInRubbishBin())
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

void NodeSelectorTreeView::onNavigateReady(const QModelIndex& index)
{
    if (index.isValid())
    {
        // Loading finished
        proxyModel()->getMegaModel()->sendBlockUiSignal(false);

        QPoint point = visualRect(index).center();
        QMouseEvent mouseEvent(QEvent::MouseButtonDblClick,
                               point,
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
