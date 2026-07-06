#include "NodeSelectorSelectionCoordinator.h"

#include "NodeSelectorMergeTargetUtils.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorModelItem.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeView.h"

#include <QScopedValueRollback>

namespace
{
// A queued handle's tree path load can die before the handle becomes mappable (the model was
// still empty, the chain failed synchronously, was aborted, or was clobbered by another load).
// Retry a few times -- transient states resolve within a pass or two -- then drop the handle:
// one that never maps (e.g. filtered out by the proxy) must not be re-queued forever.
constexpr int MAX_TREE_PATH_LOAD_ATTEMPTS = 3;
}

NodeSelectorSelectionCoordinator::NodeSelectorSelectionCoordinator(mega::MegaApi* megaApi,
                                                                   Objects objects,
                                                                   Policy policy):
    QObject(nullptr),
    mMegaApi(megaApi),
    mModel(objects.model),
    mProxyModel(objects.proxyModel),
    mTreeView(objects.treeView),
    mSelectIndex(std::move(policy.selectIndex)),
    mClearSelection(std::move(policy.clearSelection)),
    mOnItemDoubleClick(std::move(policy.onItemDoubleClick)),
    mOnNewFolderAdded(std::move(policy.onNewFolderAdded)),
    mSetRootIndexToTop(std::move(policy.setRootIndexToTop)),
    mWithSelectionSilenced(std::move(policy.withSelectionSilenced))
{}

void NodeSelectorSelectionCoordinator::setParentOfRestoredNodes(
    const QSet<mega::MegaHandle>& parentOfRestoredNodes)
{
    mParentOfRestoredNodes = parentOfRestoredNodes;
}

void NodeSelectorSelectionCoordinator::setMergeFolderHandles(
    const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles)
{
    mMergeTargetFolders = handles;
}

void NodeSelectorSelectionCoordinator::resetMergeFolderHandles(
    const QMultiHash<mega::MegaHandle, mega::MegaHandle>& handles)
{
    for (auto it = handles.keyValueBegin(); it != handles.keyValueEnd(); ++it)
    {
        mMergeTargetFolders.remove(it->first);
    }
}

void NodeSelectorSelectionCoordinator::setNewFolderInfo(const NewFolderInfo& info)
{
    mNewFolderInfo = info;
}

void NodeSelectorSelectionCoordinator::setSelectedNodeHandle(const mega::MegaHandle& selectedHandle)
{
    if (selectedHandle == mega::INVALID_HANDLE || mModel->rowCount() == 0)
    {
        return;
    }

    auto node = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(selectedHandle));
    if (!node)
    {
        return;
    }

    mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << node->getHandle());
    triggerTreePathLoad(node);
}

void NodeSelectorSelectionCoordinator::triggerTreePathLoad(
    const std::shared_ptr<mega::MegaNode>& node)
{
    mProxyModel->setExpandMapped(true);
    mModel->loadTreeFromNode(node);
}

void NodeSelectorSelectionCoordinator::expandPendingIndexes()
{
    auto indexesToBeExpanded = mModel->needsToBeExpanded();
    if (indexesToBeExpanded.isEmpty())
    {
        return;
    }

    for (const auto& item: indexesToBeExpanded)
    {
        auto handle = item.first;
        QModelIndex proxyIndex;

        if (handle != mega::INVALID_HANDLE)
        {
            proxyIndex = mProxyModel->getIndexFromHandle(handle);
        }

        if (proxyIndex.isValid())
        {
            mTreeView->setExpanded(proxyIndex, true);
        }
    }
}

void NodeSelectorSelectionCoordinator::selectPendingIndexes()
{
    // A synchronous re-entry (an unresolved handle below triggers loadTreeFromNode, which emits
    // blockUi(false) synchronously and re-invokes this) must bail out before draining the queue,
    // so the recursion stays bounded and the pending handles survive for the real later pass.
    if (mResolvingPendingIndexes)
    {
        return;
    }

    auto indexesToBeSelected = mModel->needsToBeSelected();
    if (indexesToBeSelected.isEmpty())
    {
        return;
    }

    // Rollback guard so the flag is restored on ANY exit path (early return added in the
    // future, exception from a policy callback); a flag stuck at true would silently disable
    // selection for the widget's lifetime.
    QScopedValueRollback<bool> resolvingGuard(mResolvingPendingIndexes, true);

    // Only a restore jumps to the top root (nodes may have different parents); a move/merge
    // stays in the current folder.
    if (!mSkipTopRootOnSelect)
    {
        mSetRootIndexToTop();
    }
    mSkipTopRootOnSelect = false;

    mWithSelectionSilenced(
        [&]() -> bool
        {
            mClearSelection();
            bool allSelected = true;
            for (const auto& item: indexesToBeSelected)
            {
                auto handle = item.first;
                if (handle == mega::INVALID_HANDLE)
                {
                    continue;
                }

                auto proxyIndex = mProxyModel->getIndexFromHandle(handle);
                if (proxyIndex.isValid())
                {
                    mLoadAttempts.remove(handle);
                    mSelectIndex(proxyIndex, true, false);
                    continue;
                }

                auto node = std::shared_ptr<mega::MegaNode>(mMegaApi->getNodeByHandle(handle));
                if (!node)
                {
                    // The node is gone (deleted, or its share was revoked): it can never be
                    // mapped. Drop it so the pending queue drains; re-queuing it would make
                    // every later pass clear the selection and jump to the top root, forever.
                    mLoadAttempts.remove(handle);
                }
                else if (mModel->isLoadingTreePath())
                {
                    // A tree path load is in flight: its rows arrive through queued signals,
                    // so they are inserted only after this synchronous pass returns. Keep the
                    // handle queued WITHOUT touching the load, and let the pass fired on
                    // insertion resolve it. Triggering a load here would clobber the chain in
                    // flight (loadTreeFromNode restarts mNodesToLoad from scratch).
                    mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << handle);
                    allSelected = false;
                }
                else if (mLoadAttempts.value(handle, 0) < MAX_TREE_PATH_LOAD_ATTEMPTS)
                {
                    // No load in flight: trigger this handle's tree path load, or re-trigger
                    // it when the previous chain died before the handle became mappable (it
                    // failed synchronously, was aborted, or was clobbered by another load).
                    // The attempt cap keeps a handle that can never be mapped (e.g. filtered
                    // out by the proxy) from being retried forever; the re-entrancy guard
                    // above keeps each attempt from recursing. Re-queue explicitly: the load
                    // can fail synchronously without leaving the handle queued.
                    ++mLoadAttempts[handle];
                    mModel->selectIndexesByHandleAsync(QSet<mega::MegaHandle>() << handle);
                    triggerTreePathLoad(node);
                    allSelected = false;
                }
                else
                {
                    // All attempts spent and still unmappable: give up so the queue drains.
                    // Terminal like the selected case, so allSelected stays true and the
                    // deferred selection notification can still fire for this pass.
                    mLoadAttempts.remove(handle);
                }
            }
            return allSelected;
        });
}

void NodeSelectorSelectionCoordinator::resetMoveNodesToSelect()
{
    if (!mModel->isMovingNodes())
    {
        mMovedHandlesToSelect.clear();
    }
}

void NodeSelectorSelectionCoordinator::onItemsMoved()
{
    // A move/merge stays in the current folder; only a restore jumps to the top root.
    mSkipTopRootOnSelect = mParentOfRestoredNodes.isEmpty();

    if (!mMovedHandlesToSelect.isEmpty() || !mMergeTargetFolders.isEmpty())
    {
        mClearSelection();
    }

    if (!mMovedHandlesToSelect.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMovedHandlesToSelect);
    }

    if (!mMergeTargetFolders.isEmpty())
    {
        mModel->selectIndexesByHandleAsync(mMergeTargetFolders.values());
    }

    // Keep mMovedHandlesToSelect populated: the moved node's model item is recreated
    // (remove + re-insert) while the move settles, which drops the selection of its row.
    // reapplyMovedSelection() re-selects it by handle whenever its row reappears.
    mParentOfRestoredNodes.clear();
    mMergeTargetFolders.clear();
}

void NodeSelectorSelectionCoordinator::reapplyMovedSelection()
{
    if (mMovedHandlesToSelect.isEmpty())
    {
        return;
    }

    mWithSelectionSilenced(
        [&]() -> bool
        {
            // Clear first so only the moved nodes remain selected: the restored selection
            // model keeps the pre-move selection (e.g. the destination folder the user had
            // open), which must not stay selected after the move.
            mClearSelection();
            for (const auto& handle: std::as_const(mMovedHandlesToSelect))
            {
                auto proxyIndex = mProxyModel->getIndexFromHandle(handle);
                if (proxyIndex.isValid())
                {
                    mSelectIndex(proxyIndex, true, false);
                }
            }
            return false;
        });
}

void NodeSelectorSelectionCoordinator::clearMovedSelection()
{
    mMovedHandlesToSelect.clear();
}

void NodeSelectorSelectionCoordinator::onNodesAdded(
    const QList<QPointer<NodeSelectorModelItem>>& itemsAdded)
{
    if (mModel->isMovingNodes())
    {
        auto moveProcessCounter(0);

        for (const auto& item: itemsAdded)
        {
            if (!item)
            {
                continue;
            }

            auto node = item->getNode();
            if (!node)
            {
                continue;
            }

            if (!NodeSelectorMergeTargetUtils::isNodeInsideMergeTargetSubtree(mMegaApi,
                                                                              mMergeTargetFolders,
                                                                              node.get()))
            {
                mMovedHandlesToSelect.insert(node->getHandle());
                moveProcessCounter++;
            }
        }

        mModel->moveProcessedByNumber(moveProcessCounter);
    }
    else
    {
        for (const auto& item: itemsAdded)
        {
            checkNewFolderAdded(item);
        }
    }
}

void NodeSelectorSelectionCoordinator::checkNewFolderAdded(QPointer<NodeSelectorModelItem> item)
{
    if (!mNewFolderInfo.recentlyAdded)
    {
        return;
    }

    if (!item || !item->getNode())
    {
        return;
    }

    if (item->getNode()->getHandle() == mNewFolderInfo.handle)
    {
        auto newFolderIndex(mProxyModel->getIndexFromHandle(mNewFolderInfo.handle));
        mOnNewFolderAdded(newFolderIndex);

        mNewFolderInfo.handle = mega::INVALID_HANDLE;
        mNewFolderInfo.recentlyAdded = false;
    }
}

QSet<mega::MegaHandle>& NodeSelectorSelectionCoordinator::movedHandlesToSelect()
{
    return mMovedHandlesToSelect;
}

QSet<mega::MegaHandle>& NodeSelectorSelectionCoordinator::parentOfRestoredNodes()
{
    return mParentOfRestoredNodes;
}

QMultiHash<mega::MegaHandle, mega::MegaHandle>&
    NodeSelectorSelectionCoordinator::mergeTargetFolders()
{
    return mMergeTargetFolders;
}
