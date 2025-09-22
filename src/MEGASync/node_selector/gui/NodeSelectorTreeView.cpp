#include "NodeSelectorTreeView.h"

#include "CreateRemoveSyncsManager.h"
#include "DialogOpener.h"
#include "MegaApplication.h"
#include "MegaDelegateHoverManager.h"
#include "NodeSelector.h"
#include "NodeSelectorDelegates.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "Platform.h"

#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>
#include <QPainterPath>

QList<mega::MegaHandle> NodeSelectorTreeView::mCopiedHandles = QList<mega::MegaHandle>();

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent):
    LoadingSceneView<NodeSelectorLoadingDelegate, QTreeView>(parent),
    mMegaApi(MegaSyncApp->getMegaApi())
{
    installEventFilter(this);
    loadingView().setDelayTimeToShowInMs(150);

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
        QString icon;
        if (isExpanded(index))
        {
            icon = QLatin1String("chevron_down");
        }
        else
        {
            icon = QLatin1String("chevron-right");
        }

        painter->drawPixmap(opt.rect,
                            Utilities::getColoredPixmap(icon,
                                                        Utilities::AttributeType::SMALL |
                                                            Utilities::AttributeType::THIN |
                                                            Utilities::AttributeType::OUTLINE,
                                                        QLatin1String("icon-secondary"),
                                                        iconSize));
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
            rect.setRight(option.rect.right() - 10);
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
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) ==
        QEvent::MouseButtonPress)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if (accept)
    {
        QTreeView::mousePressEvent(event);
    }
}

void NodeSelectorTreeView::mouseReleaseEvent(QMouseEvent* event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) ==
        QEvent::MouseButtonRelease)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if (accept)
    {
        QTreeView::mouseReleaseEvent(event);
    }
}

void NodeSelectorTreeView::mouseDoubleClickEvent(QMouseEvent* event)
{
    if (event->button() != Qt::RightButton)
    {
        QTreeView::mouseDoubleClickEvent(event);
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
                            deleteNode(selectionHandles,
                                       deletionType == DeletionType::MOVE_TO_RUBBISH ? false :
                                                                                       true);
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
        auto megaLinkAction(new QAction(tr("Share link")));
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
            auto pasteAction(new QAction(tr("Paste")));
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

void NodeSelectorTreeView::addRestoreMenuAction(QMap<int, QAction*>& actions,
                                                const QModelIndexList& selectedIndexes,
                                                const QList<MegaHandle>& selectionHandles)
{
    if (areAllEligibleForRestore(selectedIndexes))
    {
        auto restoreAction(new QAction(tr("Restore")));
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

    auto renameAction(new QAction(tr("Rename")));
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
        auto syncAction(new QAction(tr("Sync")));
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
        auto unsyncAction(new QAction(tr("Stop syncing")));
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

void NodeSelectorTreeView::setAllowContextMenu(bool newAllowContextMenu)
{
    mAllowContextMenu = newAllowContextMenu;
}

void NodeSelectorTreeView::addDeleteMenuAction(QMap<int, QAction*>& actions,
                                               QList<MegaHandle> selectionHandles)
{
    auto deleteAction(new QAction(tr("Move to Rubbish bin")));
    connect(deleteAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                deleteNode(selectionHandles, false);
            });
    actions.insert(ActionsOrder::DELETE_RUBBISH, deleteAction);
}

void NodeSelectorTreeView::addDeletePermanently(QMap<int, QAction*>& actions,
                                                QList<MegaHandle> selectionHandles)
{
    auto deletePermanentlyAction(new QAction(tr("Permanently delete")));
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
    auto leaveShareAction(new QAction(tr("Leave folder")));
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

    QMenu customMenu;
    Platform::getInstance()->initMenu(&customMenu, "CustomMenu");

    if (!selectionModel())
    {
        return;
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QList<mega::MegaHandle> selectionHandles;
    mega::MegaHandle clickedHandle;

    auto indexClicked = indexAt(event->pos());
    if (indexClicked.isValid())
    {
        clickedHandle = proxyModel->getHandle(indexClicked);
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

    if (areAllEligibleForCopy(selectedIndexes))
    {
        auto copyAction(new QAction(tr("Copy")));
        connect(copyAction,
                &QAction::triggered,
                [selectionHandles]()
                {
                    mCopiedHandles = selectionHandles;
                });
        actions.insert(ActionsOrder::COPY, copyAction);
    }

    addPasteMenuAction(actions, selectedIndexes);

    if (!selectedIndexes.isEmpty())
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

void NodeSelectorTreeView::deleteNode(const QList<MegaHandle>& handles, bool permanently)
{
    emit deleteNodeClicked(handles, permanently);
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

bool NodeSelectorTreeView::mousePressorReleaseEvent(QMouseEvent* event)
{
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    if (!index.isValid())
    {
        return false;
    }

    auto item = qvariant_cast<NodeSelectorModelItem*>(
        index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if (item && item->isCloudDrive())
    { // this line avoid to cloud drive being collapsed and at same time it allows to select it.
        return handleStandardMouseEvent(event);
    }
    else
    {
        if (!index.data(toInt(NodeRowDelegateRoles::INIT_ROLE)).toBool())
        {
            int position = columnViewportPosition(0);
            QModelIndex idx = index.parent();
            while (rootIndex() != idx)
            {
                position += indentation();
                idx = idx.parent();
            }
            QRect rect(position, event->pos().y(), indentation(), rowHeight(index));

            if (rect.contains(event->pos()))
            {
                if (!isExpanded(index))
                {
                    auto sourceIndexToExpand = proxyModel()->mapToSource(index);
                    if (proxyModel()->sourceModel()->canFetchMore(sourceIndexToExpand))
                    {
                        proxyModel()->setExpandMapped(true);
                        proxyModel()->sourceModel()->fetchMore(sourceIndexToExpand);
                        setExpanded(index, true);
                    }

                    return handleStandardMouseEvent(event);
                }
            }
        }
    }
    return true;
}

bool NodeSelectorTreeView::handleStandardMouseEvent(QMouseEvent* event)
{
    if (event->type() == QMouseEvent::MouseButtonPress)
    {
        QAbstractItemView::mousePressEvent(event);
    }
    else if (event->type() == QMouseEvent::MouseButtonRelease)
    {
        QAbstractItemView::mouseReleaseEvent(event);
    }

    return false;
}

NodeSelectorTreeViewHeaderView::NodeSelectorTreeViewHeaderView(Qt::Orientation orientation,
                                                               QWidget* parent):
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void NodeSelectorTreeViewHeaderView::paintSection(QPainter* painter,
                                                  const QRect& rect,
                                                  int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if (logicalIndex == NodeSelectorModel::USER)
    {
        QRect iconRect(QPoint(rect.topLeft()), QSize(18, 18));
        iconRect.moveCenter(rect.center());
        QIcon icon = model()
                         ->headerData(logicalIndex,
                                      Qt::Orientation::Horizontal,
                                      toInt(HeaderRoles::ICON_ROLE))
                         .value<QIcon>();
        if (!icon.isNull())
        {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter);
        }
    }
}
