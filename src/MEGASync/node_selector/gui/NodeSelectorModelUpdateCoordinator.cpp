#include "NodeSelectorModelUpdateCoordinator.h"

#include "NodeSelectorMergeTargetUtils.h"
#include "NodeSelectorModel.h"
#include "NodeSelectorProxyModel.h"
#include "NodeSelectorTreeView.h"

#include <QAbstractItemView>
#include <QItemSelectionModel>

NodeSelectorModelUpdateCoordinator::NodeSelectorModelUpdateCoordinator(mega::MegaApi* megaApi,
                                                                       Objects objects,
                                                                       SharedState sharedState,
                                                                       Policy policy):
    mMegaApi(megaApi),
    mModel(objects.model),
    mProxyModel(objects.proxyModel),
    mTreeView(objects.treeView),
    mMovedHandlesToSelect(sharedState.movedHandlesToSelect),
    mParentOfRestoredNodes(sharedState.parentOfRestoredNodes),
    mMergeTargetFolders(sharedState.mergeTargetFolders),
    mNodesToBeReplaced(sharedState.nodesToBeReplaced),
    mGetNodeState(std::move(policy.getNodeState)),
    mGetAddedNodeParent(std::move(policy.getAddedNodeParent))
{}

bool NodeSelectorModelUpdateCoordinator::onNodesUpdate(mega::MegaNodeList* nodes)
{
    if (!nodes)
    {
        return false;
    }

    for (int i = 0; i < nodes->size(); i++)
    {
        auto node = nodes->get(i);

        // Some specialised model could take the update with special logic
        if (mModel->rootNodeUpdated(node))
        {
            continue;
        }

        if (node->getParentHandle() != mega::INVALID_HANDLE)
        {
            const auto index = mModel->findIndexByNodeHandle(node->getHandle(), QModelIndex());
            const auto existenceType = static_cast<NodeState>(mGetNodeState(index, node));

            if (existenceType == NodeState::DOESNT_EXIST)
            {
                continue;
            }

            if (node->getChanges() &
                (mega::MegaNode::CHANGE_TYPE_PARENT | mega::MegaNode::CHANGE_TYPE_NEW))
            {
                if (existenceType == NodeState::REMOVE)
                {
                    mRemovedNodes.append(UpdateNodesInfo(node, index));
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> parentNode(
                        mMegaApi->getNodeByHandle(node->getParentHandle()));
                    if (parentNode)
                    {
                        if (existenceType == NodeState::ADD || existenceType == NodeState::MOVED)
                        {
                            if (!node->isFile() || mModel->showFiles())
                            {
                                if (mUpdatedNodesBeforeAdded.contains(node->getHandle()))
                                {
                                    mUpdatedNodesBeforeAdded.remove(node->getHandle());
                                }

                                mAddedNodesByParentHandle.insert(node->getParentHandle(),
                                                                 UpdateNodesInfo(node, index));
                            }

                            if (existenceType == NodeState::MOVED)
                            {
                                mRemoveMovedNodes.append(UpdateNodesInfo(node, index));
                            }
                        }
                        else if (existenceType == NodeState::EXISTS_BUT_OUT_OF_VIEW &&
                                 mParentOfRestoredNodes.contains(node->getParentHandle()))
                        {
                            mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                        }
                        else if (existenceType == NodeState::EXISTS_BUT_PARENT_UNINITIALISED ||
                                 existenceType == NodeState::MOVED_OUT_OF_VIEW)
                        {
                            if (existenceType == NodeState::MOVED_OUT_OF_VIEW)
                            {
                                mRemoveMovedNodes.append(UpdateNodesInfo(node, index));
                            }

                            if (!NodeSelectorMergeTargetUtils::isNodeInsideMergeTargetSubtree(
                                    mMegaApi,
                                    mMergeTargetFolders,
                                    node))
                            {
                                mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                            }
                        }
                    }
                }
            }
            else if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_NAME)
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mRenamedNodesByHandle.append(UpdateNodesInfo(node, index));
                }
            }
            else if (node->getChanges() & mega::MegaNode::CHANGE_TYPE_REMOVED)
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mRemovedNodes.append(UpdateNodesInfo(node, index));
                }
                else if (existenceType == NodeState::EXISTS_BUT_PARENT_UNINITIALISED)
                {
                    mUpdatedButInvisibleNodes.append(UpdateNodesInfo(node, index));
                }
            }
            else if (node->getChanges() &
                     (mega::MegaNode::CHANGE_TYPE_ATTRIBUTES | MegaNode::CHANGE_TYPE_PUBLIC_LINK))
            {
                if (existenceType == NodeState::EXISTS)
                {
                    mUpdatedNodes.append(UpdateNodesInfo(node, index));
                }
                else if (existenceType == NodeState::ADD)
                {
                    mUpdatedNodesBeforeAdded.insert(node->getHandle(),
                                                    UpdateNodesInfo(node, index));
                }
            }
        }
    }

    return hasPendingUpdates();
}

void NodeSelectorModelUpdateCoordinator::processCachedNodesUpdated()
{
    if (!mProxyModel->isWorking() && !mModel->isRequestingNodes() && hasPendingUpdates())
    {
        int moveProcessedCounter = 0;

        if (!mModel->isBeingModified())
        {
            QList<mega::MegaHandle> renamedHandles;
            renamedHandles.reserve(mRenamedNodesByHandle.size());
            for (const auto& info: std::as_const(mRenamedNodesByHandle))
            {
                updateNode(info, true);
                renamedHandles.append(info.handle);
            }
            mRenamedNodesByHandle.clear();

            if (!renamedHandles.isEmpty())
            {
                emit nodesRenamed(renamedHandles);
            }
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mUpdatedNodes))
            {
                updateNode(info, false);
            }
            mUpdatedNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mRemovedNodes))
            {
                removeItemByHandle(info.handle);

                if (!mNodesToBeReplaced.remove(info.handle))
                {
                    moveProcessedCounter++;
                }
            }
            mRemovedNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            for (const auto& info: std::as_const(mRemoveMovedNodes))
            {
                removeItemByHandle(info.handle);
            }
            mRemoveMovedNodes.clear();
        }

        if (!mModel->isBeingModified() && !mUpdatedButInvisibleNodes.isEmpty())
        {
            for (const auto& info: std::as_const(mUpdatedButInvisibleNodes))
            {
                if (info.handle != mega::INVALID_HANDLE)
                {
                    if (info.node->getChanges() == mega::MegaNode::CHANGE_TYPE_REMOVED)
                    {
                        removeItemByHandle(info.handle);
                    }
                    else
                    {
                        mMovedHandlesToSelect.insert(info.handle);
                    }
                    moveProcessedCounter++;
                }
            }

            mUpdatedButInvisibleNodes.clear();
        }

        if (!mModel->isBeingModified())
        {
            foreach(auto& parentHandle, mAddedNodesByParentHandle.uniqueKeys())
            {
                const auto parentIndex = mGetAddedNodeParent(parentHandle);
                const auto infos = mAddedNodesByParentHandle.values(parentHandle);
                QList<std::shared_ptr<mega::MegaNode>> addedNodes;

                for (const auto& info: infos)
                {
                    const auto handle = info.handle;

                    if (mUpdatedNodesBeforeAdded.contains(handle))
                    {
                        addedNodes.append(mUpdatedNodesBeforeAdded.take(handle).node);
                    }
                    else
                    {
                        addedNodes.append(info.node);
                    }
                }

                if (!mModel->addNodes(addedNodes, parentIndex))
                {
                    auto processedNodes = 0;

                    for (const auto& addedNode: std::as_const(addedNodes))
                    {
                        if (!NodeSelectorMergeTargetUtils::isNodeInsideMergeTargetSubtree(
                                mMegaApi,
                                mMergeTargetFolders,
                                addedNode.get()))
                        {
                            processedNodes++;
                        }
                    }

                    mModel->moveProcessedByNumber(processedNodes);
                }

                const auto proxyParentIndex = mProxyModel->mapFromSource(parentIndex);
                if (!proxyParentIndex.parent().isValid())
                {
                    mTreeView->setExpanded(proxyParentIndex, true);
                }
            }

            mAddedNodesByParentHandle.clear();
            mUpdatedNodesBeforeAdded.clear();
        }

        mModel->moveProcessedByNumber(moveProcessedCounter);
    }
}

bool NodeSelectorModelUpdateCoordinator::hasPendingUpdates() const
{
    return !mUpdatedNodes.isEmpty() || !mRemovedNodes.isEmpty() ||
           !mRenamedNodesByHandle.isEmpty() || !mAddedNodesByParentHandle.isEmpty() ||
           !mRemoveMovedNodes.isEmpty() || !mUpdatedButInvisibleNodes.isEmpty();
}

bool NodeSelectorModelUpdateCoordinator::shouldUpdateImmediately(int immediateThreshold) const
{
    auto totalSize = mUpdatedNodes.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    totalSize += mRemovedNodes.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    totalSize += mRemoveMovedNodes.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    totalSize += mRenamedNodesByHandle.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    totalSize += mAddedNodesByParentHandle.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    totalSize += mUpdatedButInvisibleNodes.size();
    if (totalSize > immediateThreshold)
    {
        return true;
    }
    return false;
}

void NodeSelectorModelUpdateCoordinator::updateNode(const UpdateNodesInfo& info, bool scrollTo)
{
    auto index = mModel->findIndexByNodeHandle(info.handle, QModelIndex());
    auto proxyIndex = mProxyModel->mapFromSource(index);

    auto isSelected = false;

    if (scrollTo)
    {
        if (mTreeView->selectionModel())
        {
            if (proxyIndex.isValid())
            {
                isSelected = mTreeView->selectionModel()->isSelected(proxyIndex);
            }
        }
    }

    mModel->updateItemNode(index, info.node);
    proxyIndex = mProxyModel->mapFromSource(index);

    if (info.node)
    {
        if (proxyIndex.isValid() && mTreeView->rootIndex() == proxyIndex)
        {
            emit viewStateChanged();
        }
    }

    if (isSelected)
    {
        proxyIndex = mProxyModel->mapFromSource(index);
        mTreeView->scrollTo(proxyIndex, QAbstractItemView::ScrollHint::PositionAtCenter);
    }
}

void NodeSelectorModelUpdateCoordinator::removeItemByHandle(mega::MegaHandle handle)
{
    const auto index = mModel->findIndexByNodeHandle(handle, QModelIndex());
    if (index.isValid())
    {
        const auto proxyIndex = mProxyModel->mapFromSource(index);
        if (proxyIndex.isValid())
        {
            mMovedHandlesToSelect.remove(handle);
            emit indexRemovedAffectingCurrentPath(proxyIndex);
            mProxyModel->deleteNode(proxyIndex);
        }
    }
}
