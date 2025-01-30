#include "NodeSelectorTreeView.h"

#include "CreateRemoveSyncsManager.h"
#include "MegaApplication.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "Platform.h"

#include <QMenu>
#include <QMetaEnum>
#include <QMouseEvent>
#include <QPainter>

QList<mega::MegaHandle> NodeSelectorTreeView::mCopiedHandles = QList<mega::MegaHandle>();

NodeSelectorTreeView::NodeSelectorTreeView(QWidget* parent) :
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
}

NodeSelectorTreeView::~NodeSelectorTreeView()
{
    mCopiedHandles.clear();
}

QModelIndex NodeSelectorTreeView::getIndexFromSourceModel(const QModelIndex& index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    return proxyModel()->getIndexFromSource(index);
}

NodeSelectorProxyModel *NodeSelectorTreeView::proxyModel() const
{
    return static_cast<NodeSelectorProxyModel*>(model());
}

//Only used for single selection mode
MegaHandle NodeSelectorTreeView::getSelectedNodeHandle()
{
    MegaHandle ret = INVALID_HANDLE;
    if(!selectionModel())
    {
        return ret;
    }

    if(selectionModel()->selectedRows().size() == 1)
    {
        if(auto node = proxyModel()->getNode(selectionModel()->selectedRows().first()))
            ret = node->getHandle();
    }
    return ret;
}

QList<MegaHandle> NodeSelectorTreeView::getMultiSelectionNodeHandle() const
{
    QList<MegaHandle> ret;

    if(!selectionModel())
    {
        return ret;
    }

    auto selectedRows = selectionModel()->selectedRows();

    foreach(auto& s_index, selectedRows)
    {
        if (auto node = proxyModel()->getNode(s_index))
            ret.append(node->getHandle());
    }

    return ret;
}

void NodeSelectorTreeView::setModel(QAbstractItemModel *model)
{
    QTreeView::setModel(model);
    connect(proxyModel(), &NodeSelectorProxyModel::navigateReady, this, &NodeSelectorTreeView::onNavigateReady);
}

void NodeSelectorTreeView::drawBranches(QPainter *painter, const QRect &rect, const QModelIndex &index) const
{
    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && (item->isCloudDrive() || item->isVault() || item->isRubbishBin()))
    {
        QStyleOptionViewItem opt = viewOptions();
        opt.rect = rect;
        if(!selectionModel())
        {
            return;
        }
        if(selectionModel()->isSelected(index))
        {
            opt.state |= QStyle::State_Selected;
        }
        style()->drawPrimitive(QStyle::PE_IndicatorBranch, &opt, painter, this);
        return;
    }
    QTreeView::drawBranches(painter, rect, index);
}

void NodeSelectorTreeView::mousePressEvent(QMouseEvent *event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonPress)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if(accept)
    {
        QTreeView::mousePressEvent(event);
    }
}


void NodeSelectorTreeView::mouseReleaseEvent(QMouseEvent *event)
{
    bool accept = true;
    if (style()->styleHint(QStyle::SH_ListViewExpand_SelectMouseType, 0, this) == QEvent::MouseButtonRelease)
    {
        accept = mousePressorReleaseEvent(event);
    }

    if(accept)
    {
        QTreeView::mouseReleaseEvent(event);
    }
}

void NodeSelectorTreeView::mouseDoubleClickEvent(QMouseEvent *event)
{
    if(event->button() != Qt::RightButton)
    {
        QTreeView::mouseDoubleClickEvent(event);
    }
}

void NodeSelectorTreeView::keyPressEvent(QKeyEvent *event)
{
    if(!selectionModel())
    {
        return;
    }

    QModelIndexList selectedRows = selectionModel()->selectedRows();

    static QModelIndex cdRootIndex = proxyModel()->getIndexFromNode(MegaSyncApp->getRootNode());
    static QList<int> bannedFromRootKeyList = QList<int>() << Qt::Key_Left << Qt::Key_Right
                                                     << Qt::Key_Plus << Qt::Key_Minus;

    if(!bannedFromRootKeyList.contains(event->key()) || !selectedRows.contains(cdRootIndex))
    {
        if(event->key() == Qt::Key_F2)
        {
            renameNode();
        }
        else if(event->key() == Qt::Key_Enter || event->key() == Qt::Key_Return)
        {
            if(!selectedRows.isEmpty())
            {
                if(selectedRows.first() == rootIndex() || selectedRows.size() > 1)
                {
                    emit nodeSelected();
                }
                else
                {
                    auto node = std::unique_ptr<MegaNode>(mMegaApi->getNodeByHandle(getSelectedNodeHandle()));
                    if(node)
                    {
                        if(node->isFolder())
                        {
                            emit doubleClicked(selectedRows.first());
                        }
                        else
                        {
                            emit nodeSelected();
                        }
                    }
                }
            }
        }
        else if(event->key() == Qt::Key_Delete)
        {
            auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
            auto removeType(proxyModel->canBeDeleted());
            if (removeType != NodeSelectorModel::RemoveType::NO_REMOVE)
            {
                auto selectionHandles(getMultiSelectionNodeHandle());

                if (areAllEligibleForDeletion(selectionHandles))
                {
                    deleteNode(selectionHandles,
                               removeType == NodeSelectorModel::RemoveType::MOVE_TO_RUBBISH ?
                                   false :
                                   true);
                }
            }
        }
    }
}

void NodeSelectorTreeView::onCopyShortcutActivated()
{
    auto selectionHandles(getMultiSelectionNodeHandle());

    if (areAllEligibleForCopy(selectionHandles))
    {
        mCopiedHandles = selectionHandles;
    }
}

void NodeSelectorTreeView::onPasteShortcutActivated()
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndex pasteIndex =
        proxyModel->getMegaModel()->rootIndex(proxyModel->mapToSource(rootIndex()));

    if (proxyModel->getMegaModel()->pasteNodes(mCopiedHandles, pasteIndex))
    {
        mCopiedHandles.clear();
    }
}

void NodeSelectorTreeView::onPasteClicked()
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndex pasteIndex = findIndexToMoveItem();

    if (proxyModel->getMegaModel()->pasteNodes(mCopiedHandles, pasteIndex))
    {
        mCopiedHandles.clear();
    }
}

void NodeSelectorTreeView::addPasteMenuAction(QMap<int, QAction*>& actions)
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndex pasteIndex = findIndexToMoveItem();

    if (!mCopiedHandles.isEmpty() &&
        proxyModel->getMegaModel()->canPasteNodes(mCopiedHandles, pasteIndex))
    {
        auto pasteAction(new QAction(tr("Paste")));
        connect(pasteAction,
                &QAction::triggered,
                this,
                [this]()
                {
                    onPasteClicked();
                });
        actions.insert(ActionsOrder::PASTE, pasteAction);
    }
}

void NodeSelectorTreeView::addRestoreMenuAction(QMap<int, QAction*>& actions,
                                                QList<MegaHandle> selectionHandles)
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

void NodeSelectorTreeView::addDeleteMenuAction(QMap<int, QAction*>& actions,
                                               QList<MegaHandle> selectionHandles)
{
    auto deleteAction(new QAction(tr("Delete")));
    connect(deleteAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                deleteNode(selectionHandles, false);
            });
    actions.insert(ActionsOrder::DELETE, deleteAction);
}

void NodeSelectorTreeView::addDeletePermanently(QMap<int, QAction*>& actions,
                                                QList<MegaHandle> selectionHandles)
{
    auto deletePermanentlyAction(new QAction(tr("Delete permanently")));
    connect(deletePermanentlyAction,
            &QAction::triggered,
            this,
            [this, selectionHandles]()
            {
                deleteNode(selectionHandles, true);
            });
    actions.insert(ActionsOrder::DELETE_PERMANENTLY, deletePermanentlyAction);
}

QModelIndex NodeSelectorTreeView::findIndexToMoveItem() const
{
    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());

    QModelIndexList selectionIndexes(selectionModel()->selectedRows());

    QModelIndex pasteIndex;

    if (selectionIndexes.isEmpty())
    {
        pasteIndex = proxyModel->getMegaModel()->rootIndex(proxyModel->mapToSource(rootIndex()));
    }
    else
    {
        pasteIndex = proxyModel->mapToSource(selectionIndexes.first());
    }

    return pasteIndex;
}

void NodeSelectorTreeView::contextMenuEvent(QContextMenuEvent *event)
{
    QMenu customMenu;
    Platform::getInstance()->initMenu(&customMenu, "CustomMenu");

    if (!selectionModel())
    {
        return;
    }

    if (!indexAt(event->pos()).isValid())
    {
        clearSelection();
    }

    auto proxyModel = static_cast<NodeSelectorProxyModel*>(model());
    auto removeType(proxyModel->canBeDeleted());

    QList<mega::MegaHandle> selectionHandles{getMultiSelectionNodeHandle()};

    QMap<int, QAction*> actions;

    if (!selectionHandles.isEmpty() && areAllEligibleForCopy(selectionHandles))
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

    auto selectedIndexes(selectionModel()->selectedRows());

    if (selectedIndexes.size() <= 1)
    {
        addPasteMenuAction(actions);
    }

    if (!selectionHandles.isEmpty())
    {
        auto selectedIndex = proxyModel->mapToSource(selectedIndexes.first());

        if (selectionHandles.size() == 1)
        {
            if (areAllEligibleForRestore(selectionHandles))
            {
                addRestoreMenuAction(actions, selectionHandles);
            }

            if (removeType == NodeSelectorModel::RemoveType::PERMANENT_REMOVE)
            {
                addDeletePermanently(actions, selectionHandles);
            }
            else
            {
                int access = Utilities::getNodeAccess(selectionHandles.first());

                if (access != MegaShare::ACCESS_UNKNOWN)
                {
                    if (access == MegaShare::ACCESS_OWNER)
                    {
                        auto megaLinkAction(new QAction(tr("Share link")));
                        connect(megaLinkAction,
                                &QAction::triggered,
                                this,
                                [this]()
                                {
                                    getMegaLink();
                                });
                        actions.insert(ActionsOrder::MEGA_LINK, megaLinkAction);
                    }

                    if (removeType != NodeSelectorModel::RemoveType::NO_REMOVE)
                    {
                        if (access >= MegaShare::ACCESS_FULL)
                        {
                            auto item = proxyModel->getMegaModel()->getItemByIndex(selectedIndex);

                            auto renameAction(new QAction(tr("Rename")));
                            connect(renameAction,
                                    &QAction::triggered,
                                    this,
                                    [this]()
                                    {
                                        renameNode();
                                    });
                            actions.insert(ActionsOrder::RENAME, renameAction);

                            if (item)
                            {
                                auto itemStatus = item->getStatus();
                                if (itemStatus == NodeSelectorModelItem::Status::NONE &&
                                    !(item->getNode()->isFile()))
                                {
                                    auto syncAction(new QAction(tr("Sync")));
                                    connect(syncAction,
                                            &QAction::triggered,
                                            this,
                                            [selectionHandles]()
                                            {
                                                CreateRemoveSyncsManager::addSync(
                                                    SyncInfo::SyncOrigin::CLOUD_DRIVE_DIALOG_ORIGIN,
                                                    selectionHandles.first(),
                                                    true);
                                            });
                                    actions.insert(ActionsOrder::SYNC, syncAction);
                                }
                                else if (itemStatus == NodeSelectorModelItem::Status::SYNC)
                                {
                                    auto unsyncAction(new QAction(tr("Stop syncing")));
                                    connect(unsyncAction,
                                            &QAction::triggered,
                                            this,
                                            [this, selectionHandles]()
                                            {
                                                CreateRemoveSyncsManager::removeSync(
                                                    selectionHandles.first(),
                                                    this);
                                            });
                                    actions.insert(ActionsOrder::UNSYNC, unsyncAction);
                                }
                            }

                            addDeleteMenuAction(actions, selectionHandles);
                        }
                    }
                }
            }
        }
        else if (selectionHandles.size() > 1)
        {
            if (areAllEligibleForRestore(selectionHandles))
            {
                addRestoreMenuAction(actions, selectionHandles);
            }

            if (removeType != NodeSelectorModel::RemoveType::NO_REMOVE)
            {
                // All or none
                if (areAllEligibleForDeletion(selectionHandles))
                {
                    if (removeType == NodeSelectorModel::RemoveType::PERMANENT_REMOVE)
                    {
                        addDeletePermanently(actions, selectionHandles);
                    }
                    else
                    {
                        addDeleteMenuAction(actions, selectionHandles);
                    }
                }
            }
        }
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
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dragMoveEvent(QDragMoveEvent* event)
{
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        QModelIndex dropIndex = indexAt(event->pos());
        if (!dropIndex.isValid())
        {
            dropIndex = rootIndex();
        }

        // clear selection and select only the drop index
        selectionModel()->clearSelection();

        if (!proxyModel()->canDropMimeData(event->mimeData(), Qt::CopyAction, -1, -1, dropIndex))
        {
            event->ignore();
            return;
        }

        selectionModel()->select(indexAt(event->pos()), QItemSelectionModel::Select | QItemSelectionModel::Rows);

        event->acceptProposedAction();
        event->accept();
    }
}

void NodeSelectorTreeView::dropEvent(QDropEvent* event)
{
    if(proxyModel()->getMegaModel()->acceptDragAndDrop(event->mimeData()))
    {
        // get drop index
        QModelIndex dropIndex = indexAt(event->pos());

        // Get the list of URLs
        QList<QUrl> urlList = event->mimeData()->urls();
        if(!urlList.isEmpty())
        {
            // get the node handle of the drop index from the proxy model
            auto node = getDropNode(dropIndex);
            if(node)
            {
                MegaSyncApp->uploadFilesToNode(urlList, node->getHandle());
            }
            else
            {
                event->ignore();
                return;
            }
        }

        QTreeView::dropEvent(event);
        event->acceptProposedAction();
    }
}

std::shared_ptr<MegaNode> NodeSelectorTreeView::getDropNode(const QModelIndex& dropIndex)
{
    if(!dropIndex.isValid())
    {
        const auto root = rootIndex();
        NodeSelectorModelItem* item(nullptr);
        if(!root.isValid())
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
    if(!node || node->isFolder())
    {
        return node;
    }
    return std::shared_ptr<MegaNode>(mMegaApi->getParentNode(node.get()));
}

bool NodeSelectorTreeView::areAllEligibleForCopy(const QList<MegaHandle>& handles) const
{
    if (!proxyModel()->getMegaModel()->canCopyNodes())
    {
        return false;
    }

    auto copyItems(handles.size());
    foreach(auto&& nodeHandle, handles)
    {
        std::unique_ptr<mega::MegaNode> node(
            MegaSyncApp->getMegaApi()->getNodeByHandle(nodeHandle));
        if (node)
        {
            copyItems--;
        }
    }

    return copyItems == 0;
}

bool NodeSelectorTreeView::areAllEligibleForDeletion(const QList<MegaHandle> &handles) const
{
    auto removableItems(handles.size());
    for (const auto& handle: handles)
    {
        if (Utilities::getNodeAccess(handle) >= MegaShare::ACCESS_FULL)
        {
            removableItems--;
        }
    }

    return removableItems == 0;
}

bool NodeSelectorTreeView::areAllEligibleForRestore(const QList<MegaHandle> &handles) const
{
    auto restorableItems(handles.size());
    for (const auto& handle: handles)
    {
        std::unique_ptr<mega::MegaNode> node(mMegaApi->getNodeByHandle(handle));
        if (node && mMegaApi->isInRubbish(node.get()))
        {
            std::unique_ptr<mega::MegaNode> parentNode(
                mMegaApi->getNodeByHandle(node->getParentHandle()));
            auto previousParentNode =
                std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(node->getRestoreHandle()));

            if (previousParentNode && !mMegaApi->isInRubbish(previousParentNode.get()))
            {
                restorableItems--;
            }
        }
    }

    return restorableItems == 0;
}

void NodeSelectorTreeView::clearCopiedHandles()
{
    mCopiedHandles.clear();
}

void NodeSelectorTreeView::deleteNode(const QList<MegaHandle>& handles, bool permanently)
{
    emit deleteNodeClicked(handles, permanently);
}

void NodeSelectorTreeView::renameNode()
{
    emit renameNodeClicked();
}

void NodeSelectorTreeView::getMegaLink()
{
    emit getMegaLinkClicked();
}

void NodeSelectorTreeView::restore(const QList<mega::MegaHandle>& handles)
{
    emit restoreClicked(handles);
}

void NodeSelectorTreeView::onNavigateReady(const QModelIndex &index)
{
    if(index.isValid())
    {
        //Loading finished
        proxyModel()->getMegaModel()->sendBlockUiSignal(false);

        QPoint point = visualRect(index).center();
        QMouseEvent mouseEvent(QEvent::MouseButtonDblClick, point, Qt::LeftButton, Qt::LeftButton, Qt::NoModifier);
        mouseDoubleClickEvent(&mouseEvent);
    }
}

bool NodeSelectorTreeView::mousePressorReleaseEvent(QMouseEvent *event)
{
    QPoint pos = event->pos();
    QModelIndex index = indexAt(pos);
    if(!index.isValid())
    {
        return false;
    }

    auto item = qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
    if(item && item->isCloudDrive())
    {   //this line avoid to cloud drive being collapsed and at same time it allows to select it.
       return handleStandardMouseEvent(event);
    }
    else
    {
        if(!index.data(toInt(NodeRowDelegateRoles::INIT_ROLE)).toBool())
        {
            int position = columnViewportPosition(0);
            QModelIndex idx = index.parent();
            while(rootIndex() != idx)
            {
                position += indentation();
                idx = idx.parent();
            }
            QRect rect(position, event->pos().y(), indentation(), rowHeight(index));

            if(rect.contains(event->pos()))
            {
                if(!isExpanded(index))
                {
                    auto sourceIndexToExpand = proxyModel()->mapToSource(index);
                    if(proxyModel()->sourceModel()->canFetchMore(sourceIndexToExpand))
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
    if(event->type() == QMouseEvent::MouseButtonPress)
    {
        QAbstractItemView::mousePressEvent(event);
    }
    else if(event->type() == QMouseEvent::MouseButtonRelease)
    {
        QAbstractItemView::mouseReleaseEvent(event);
    }

    return false;
}

NodeSelectorTreeViewHeaderView::NodeSelectorTreeViewHeaderView(Qt::Orientation orientation, QWidget *parent) :
    QHeaderView(orientation, parent)
{
    setDefaultAlignment(Qt::AlignLeft);
    setStretchLastSection(true);
    setDefaultSectionSize(35);
}

void NodeSelectorTreeViewHeaderView::paintSection(QPainter *painter, const QRect &rect, int logicalIndex) const
{
    painter->save();
    QHeaderView::paintSection(painter, rect, logicalIndex);
    painter->restore();
    if(logicalIndex == NodeSelectorModel::USER || logicalIndex == NodeSelectorModel::STATUS)
    {  
        QRect iconRect(QPoint(rect.topLeft()), QSize(18, 18));
        iconRect.moveCenter(rect.center());
        QIcon icon = model()->headerData(logicalIndex, Qt::Orientation::Horizontal, toInt(HeaderRoles::ICON_ROLE)).value<QIcon>();
        if(!icon.isNull())
        {
            painter->setRenderHint(QPainter::Antialiasing, true);
            painter->setRenderHint(QPainter::SmoothPixmapTransform, true);
            icon.paint(painter, iconRect, Qt::AlignVCenter | Qt::AlignHCenter);
        }
    }
}

