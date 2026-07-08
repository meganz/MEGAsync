#include "NodeSelectorModelSpecialised.h"

#include "DuplicatedNodeDialog.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MyBackupsHandle.h"
#include "Utilities.h"

#include <QApplication>
#include <QToolTip>
#include <QVariant>

using namespace mega;

// Coalescing window for the search re-sort on searchPathItemsAdded bursts. Keep in sync
// with VIEW_REFRESH_DEBOUNCE_MS (NodeSelectorTreeViewWidget.cpp): both coalesce the two
// halves of the same node-update storm.
const int SEARCH_PATH_ITEMS_RESORT_DEBOUNCE_MS = 100;

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelCloudDrive::NodeSelectorModelCloudDrive(QObject* parent):
    NodeSelectorModel(parent)
{
    setAcceptDragAndDrop(true);
}

QVariant NodeSelectorModelCloudDrive::getDisplayText(NodeSelectorModelItem* item) const
{
    return item->isCloudDrive() ? MegaNodeNames::getRootNodeName(item->getNode().get()) :
                                  NodeSelectorModel::getDisplayText(item);
}

QVariant NodeSelectorModelCloudDrive::getAddedDateText(NodeSelectorModelItem* item) const
{
    return item->isCloudDrive() ? QVariant() : NodeSelectorModel::getAddedDateText(item);
}

QVariant NodeSelectorModelCloudDrive::getLastModifiedDateText(NodeSelectorModelItem* item) const
{
    return item->isCloudDrive() ? QVariant() : NodeSelectorModel::getLastModifiedDateText(item);
}

void NodeSelectorModelCloudDrive::createRootNodes()
{
    emit requestCloudDriveRootCreation();
}

int NodeSelectorModelCloudDrive::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelCloudDrive::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelCloudDrive::firstLoad()
{
    connect(this,
            &NodeSelectorModelCloudDrive::requestCloudDriveRootCreation,
            mNodeRequesterWorker,
            &NodeRequester::createCloudDriveRootItem);
    connect(mNodeRequesterWorker,
            &NodeRequester::megaCloudDriveRootItemCreated,
            this,
            &NodeSelectorModelCloudDrive::onRootItemCreated,
            Qt::QueuedConnection);

    addRootItems();
}

void NodeSelectorModelCloudDrive::onRootItemCreated()
{
    rootItemsLoaded();

    // Add the item of the Cloud Drive
    auto rootIndex(index(0, 0));
    if (canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
        mIndexesToBeExpanded.append(qMakePair(MegaSyncApp->getRootNode()->getHandle(), rootIndex));
    }
    else
    {
        // In case the root item is empty (CD empty), let the model know that we have finished
        loadLevelFinished();
        sendBlockUiSignal(false);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelIncomingShares::NodeSelectorModelIncomingShares(QObject* parent):
    NodeSelectorModel(parent)
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    mSharedNodeList = std::unique_ptr<MegaNodeList>(megaApi->getInShares());

    setAcceptDragAndDrop(true);
}

void NodeSelectorModelIncomingShares::onItemInfoUpdated(int role)
{
    if (NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(sender()))
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx =
                index(i, NodeSelectorModel::Column::USER); // we only update this column because we
                                                           // retrieve the data in async mode
            if (idx.isValid()) // so it is possible that we doesn´t have the information from the
                               // start
            {
                if (NodeSelectorModelItem* chkItem =
                        static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
                {
                    if (chkItem == item)
                    {
                        QVector<int> roles;
                        roles.append(role);
                        emit dataChanged(idx, idx, roles);
                        break;
                    }
                }
            }
        }
    }
}

void NodeSelectorModelIncomingShares::ignoreDuplicatedNodeOptions(
    std::shared_ptr<mega::MegaNode> targetNode)
{
    auto access = Utilities::getNodeAccess(targetNode.get());
    if (access < mega::MegaShare::ACCESS_FULL)
    {
        // It will be clear as soon as the dialog is closed
        DuplicatedNodeDialog::addIgnoreConflictTypes(NodeItemType::FILE_UPLOAD_AND_REPLACE);
    }
}

bool NodeSelectorModelIncomingShares::rootNodeUpdated(mega::MegaNode* node)
{
    if (node->getChanges() & MegaNode::CHANGE_TYPE_INSHARE)
    {
        if (node->isInShare())
        {
            auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
            if (!folderIndex.isValid())
            {
                emit addIncomingSharesRoot(std::shared_ptr<mega::MegaNode>(node->copy()));
            }
            else
            {
                if (mNodeRequesterWorker->isIncomingShareCompatible(node))
                {
                    // A permission change must reach the item: updateItemNode re-primes the
                    // cached access and propagates it to the child subtree (updateRow alone
                    // repaints with the stale cached value).
                    updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
                    emit incomingShareInfoChanged(node->getHandle());
                }
                else
                {
                    emit rootNodeAboutToBeRemoved(folderIndex);
                    beginRemoveRowsAsync(node->getHandle());
                    return true;
                }
            }
        }

        return true;
    }
    else if (node->getParentHandle() == mega::INVALID_HANDLE && node->isFolder())
    {
        if (node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
        {
            auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
            if (index.isValid())
            {
                emit rootNodeAboutToBeRemoved(index);
                beginRemoveRowsAsync(node->getHandle());
                return true;
            }
        }

        auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if (folderIndex.isValid())
        {
            updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
            // updateItemNode refreshes the list row via dataChanged, but the navigation
            // breadcrumb and the incoming-share header resolve the share name from the current
            // root / its parent share and are not driven by dataChanged. Reuse the share-info
            // signal so both refresh when the renamed share is the current root or an ancestor.
            emit incomingShareInfoChanged(node->getHandle());
            return true;
        }
    }

    return false;
}

bool NodeSelectorModelIncomingShares::canDropMimeData(const QMimeData* data,
                                                      Qt::DropAction action,
                                                      int row,
                                                      int column,
                                                      const QModelIndex& parent) const
{
    if (action != Qt::CopyAction && action != Qt::MoveAction)
    {
        return false;
    }

    // The drop target is the hovered folder itself (the view passes its row/col/parent),
    // including top-level shares whose parent is the invalid root.
    auto dropIndex(index(row, column, parent));

    // The paste path (canPasteNodes -> canDropMimeData with row/col = -1) passes the target
    // folder directly as 'parent', so index(row, column, parent) is invalid. Fall back to
    // 'parent', which is the actual drop target in that case.
    if (!dropIndex.isValid())
    {
        dropIndex = parent;
    }

    auto item = getItemByIndex(dropIndex);
    if (!item)
    {
        return false;
    }

    auto node = item->getNode();
    if (!node)
    {
        return false;
    }

    // When the hovered item is a file, the real drop target is its parent folder
    // (the move is resolved to the parent in startProcessingNodes). Resolve to the
    // parent here too so dropping onto a file is accepted, as it was before.
    if (!node->isFolder())
    {
        dropIndex = dropIndex.parent();
        item = getItemByIndex(dropIndex);
        node = item ? item->getNode() : nullptr;
    }

    if (!node || !node->isFolder() ||
        Utilities::getNodeAccess(node->getHandle()) < MegaShare::ACCESS_READWRITE)
    {
        return false;
    }

    return action == Qt::CopyAction ? true : checkDraggedMimeData(data, dropIndex);
}

QModelIndex NodeSelectorModelIncomingShares::getTopRootIndex() const
{
    return QModelIndex();
}

bool NodeSelectorModelIncomingShares::canBeDeleted() const
{
    return true;
}

void NodeSelectorModelIncomingShares::onRootItemsCreated()
{
    rootItemsLoaded();

    if (!mNodesToLoad.isEmpty())
    {
        auto index = getIndexFromNode(mNodesToLoad.last(), QModelIndex());
        if (canFetchMore(index))
        {
            fetchMore(index);
        }
    }
    else
    {
        loadLevelFinished();
    }
}

void NodeSelectorModelIncomingShares::createRootNodes()
{
    emit requestIncomingSharesRootCreation(mSharedNodeList);
}

int NodeSelectorModelIncomingShares::rootItemsCount() const
{
    return mSharedNodeList->size();
}

void NodeSelectorModelIncomingShares::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelIncomingShares::firstLoad()
{
    connect(this,
            &NodeSelectorModelIncomingShares::requestIncomingSharesRootCreation,
            mNodeRequesterWorker,
            &NodeRequester::createIncomingSharesRootItems);
    connect(this,
            &NodeSelectorModelIncomingShares::addIncomingSharesRoot,
            mNodeRequesterWorker,
            &NodeRequester::addIncomingSharesRootItem);
    connect(mNodeRequesterWorker,
            &NodeRequester::megaIncomingSharesRootItemsCreated,
            this,
            &NodeSelectorModelIncomingShares::onRootItemsCreated,
            Qt::QueuedConnection);

    addRootItems();
}

///////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelBackups::NodeSelectorModelBackups(QObject* parent):
    NodeSelectorModel(parent),
    mBackupsHandle(INVALID_HANDLE),
    mBackupDevicesSize(0),
    mDeviceNamesRequest(UserAttributes::DeviceNames::requestDeviceNames())
{}

QVariant NodeSelectorModelBackups::getDisplayText(NodeSelectorModelItem* item) const
{
    if (item->isMyBackupsFolder())
    {
        return MegaNodeNames::getBackupsName();
    }
    if (item->isDeviceFolder())
    {
        const auto deviceId = QString::fromUtf8(item->getNode()->getDeviceId());
        const auto deviceName = mDeviceNamesRequest->getDeviceName(deviceId);
        if (!deviceName.isEmpty())
        {
            return deviceName;
        }
    }
    return NodeSelectorModel::getDisplayText(item);
}

QVariant NodeSelectorModelBackups::getAddedDateText(NodeSelectorModelItem* item) const
{
    return item->isMyBackupsFolder() ? QVariant() : NodeSelectorModel::getAddedDateText(item);
}

QVariant NodeSelectorModelBackups::getLastModifiedDateText(NodeSelectorModelItem* item) const
{
    return item->isMyBackupsFolder() ? QVariant() :
                                       NodeSelectorModel::getLastModifiedDateText(item);
}

void NodeSelectorModelBackups::createRootNodes()
{
    emit requestBackupsRootCreation(mBackupsHandle);
}

int NodeSelectorModelBackups::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelBackups::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelBackups::firstLoad()
{
    connect(this,
            &NodeSelectorModelBackups::requestBackupsRootCreation,
            mNodeRequesterWorker,
            &NodeRequester::createBackupRootItems);
    connect(mNodeRequesterWorker,
            &NodeRequester::megaBackupRootItemsCreated,
            this,
            &NodeSelectorModelBackups::onRootItemCreated,
            Qt::QueuedConnection);

    auto backupsRequest = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mBackupsHandle = backupsRequest->getMyBackupsHandle();
    connect(backupsRequest.get(),
            &UserAttributes::MyBackupsHandle::attributeReady,
            this,
            &NodeSelectorModelBackups::onMyBackupsHandleReceived);

    connect(mDeviceNamesRequest.get(),
            &UserAttributes::DeviceNames::attributeReady,
            this,
            &NodeSelectorModelBackups::onDeviceNamesUpdated);

    addRootItems();
}

bool NodeSelectorModelBackups::canBeDeleted() const
{
    return false;
}

bool NodeSelectorModelBackups::canDropMimeData(const QMimeData*,
                                               Qt::DropAction,
                                               int,
                                               int,
                                               const QModelIndex&) const
{
    return canDropMimeData();
}

bool NodeSelectorModelBackups::canDropMimeData() const
{
    return false;
}

void NodeSelectorModelBackups::onMyBackupsHandleReceived(mega::MegaHandle handle)
{
    if (mBackupsHandle != handle && handle != INVALID_HANDLE)
    {
        mBackupsHandle = handle;
        addRootItems();
    }
}

void NodeSelectorModelBackups::onDeviceNamesUpdated()
{
    QModelIndex rootIndex(index(0, 0));
    auto rowcount = rowCount(rootIndex);
    if (rowcount > 0)
    {
        emit dataChanged(index(0, NodeSelectorModel::Column::NODE, rootIndex),
                         index(rowcount - 1, NodeSelectorModel::Column::NODE, rootIndex),
                         {Qt::DisplayRole});

        QList<mega::MegaHandle> deviceHandles;
        deviceHandles.reserve(rowcount);
        for (int row = 0; row < rowcount; ++row)
        {
            const auto deviceIndex = index(row, NodeSelectorModel::Column::NODE, rootIndex);
            if (auto* item = getItemByIndex(deviceIndex))
            {
                if (auto node = item->getNode())
                {
                    deviceHandles.append(node->getHandle());
                }
            }
        }

        if (!deviceHandles.isEmpty())
        {
            emit nodesRenamed(deviceHandles);
        }
    }
}

bool NodeSelectorModelBackups::addToLoadingList(const std::shared_ptr<MegaNode> node)
{
    return node && node->getType() != mega::MegaNode::TYPE_VAULT;
}

void NodeSelectorModelBackups::loadLevelFinished()
{
    const QModelIndex backupsRootIndex(index(0, 0));
    const bool finishingBackupsRoot =
        mIndexesToBeExpanded.size() == 1 && mIndexesToBeExpanded.at(0).second == backupsRootIndex;

    if (finishingBackupsRoot)
    {
        mBackupDevicesSize = 0;

        const int rowcount = rowCount(backupsRootIndex);
        for (int i = 0; i < rowcount; i++)
        {
            const auto idx = index(i, 0, backupsRootIndex);
            if (canFetchMore(idx) && fetchItemChildren(idx))
            {
                ++mBackupDevicesSize;
            }
        }
    }
    else if (mBackupDevicesSize > 0)
    {
        --mBackupDevicesSize;
    }

    if (mBackupDevicesSize == 0)
    {
        NodeSelectorModel::loadLevelFinished();
    }
}

void NodeSelectorModelBackups::onRootItemCreated()
{
    rootItemsLoaded();

    const QModelIndex rootIndex(index(0, 0));
    // Add the item of the Backups Drive
    if (canFetchMore(rootIndex))
    {
        auto backupItem(getItemByIndex(rootIndex));
        if (backupItem)
        {
            mIndexesToBeExpanded.append(qMakePair(backupItem->getNode()->getHandle(), rootIndex));
        }
        fetchItemChildren(rootIndex);
    }
    else
    {
        loadLevelFinished();
    }
}

NodeSelectorModelSearch::NodeSelectorModelSearch(TabTypes allowedTabTypes,
                                                 bool flattenResults,
                                                 QObject* parent):
    NodeSelectorModel(parent),
    mAllowedTabTypes(allowedTabTypes),
    mFlattenSearchResults(flattenResults),
    mDeviceNamesRequest(UserAttributes::DeviceNames::requestDeviceNames())
{
    qRegisterMetaType<TabTypes>("TabTypes");

    // Deleting/moving nodes visible in the search tab re-adds their paths one node-update at
    // a time: the worker emits searchPathItemsAdded per pass, and re-sorting on every
    // emission runs one full concurrent sort (with its blockUi/detach/reattach cycle) per
    // node. Coalesce each burst into a single re-sort at the end of the window.
    mSearchPathItemsAddedDebounce.setSingleShot(true);
    mSearchPathItemsAddedDebounce.setInterval(SEARCH_PATH_ITEMS_RESORT_DEBOUNCE_MS);
    connect(&mSearchPathItemsAddedDebounce,
            &QTimer::timeout,
            this,
            [this]()
            {
                // A structural change is open (the blocking begin was delivered but the
                // queued end is still pending) or the model is mid-reset: emitting now
                // would launch the concurrent sort against a mapping that is being
                // rebuilt. Retry once the model is idle.
                if (isBeingModified())
                {
                    mSearchPathItemsAddedDebounce.start();
                    return;
                }

                emit levelsAdded({}, false);
            });
}

void NodeSelectorModelSearch::firstLoad()
{
    connect(this,
            &NodeSelectorModelSearch::searchNodes,
            mNodeRequesterWorker,
            &NodeRequester::search);
    connect(this,
            &NodeSelectorModelSearch::requestAddSearchRootItem,
            mNodeRequesterWorker,
            &NodeRequester::addSearchRootItem);
    connect(this,
            &NodeSelectorModelSearch::requestAddSearchPathItems,
            mNodeRequesterWorker,
            &NodeRequester::addSearchPathItems);
    connect(mNodeRequesterWorker,
            &NodeRequester::searchPathItemsAdded,
            this,
            &NodeSelectorModelSearch::onSearchPathItemsAdded,
            Qt::QueuedConnection);
    connect(mNodeRequesterWorker,
            &NodeRequester::searchItemsCreated,
            this,
            &NodeSelectorModelSearch::onRootItemsCreated,
            Qt::QueuedConnection);
}

void NodeSelectorModelSearch::createRootNodes()
{
    // pure virtual function in the base class, in first stage this model is empty so not need to
    // put any code here.
}

void NodeSelectorModelSearch::searchByText(const QString& text)
{
    // A deferred re-sort scheduled by the previous results must never fire against the
    // new search's model generation.
    mSearchPathItemsAddedDebounce.stop();
    mNodeRequesterWorker->restartSearch();
    mLastSearchText = text;
    mSearchInFlight = true;
    addRootItems();
    emit searchNodes(text, mAllowedTabTypes, mFlattenSearchResults);
}

void NodeSelectorModelSearch::stopSearch()
{
    mSearchPathItemsAddedDebounce.stop();
    mNodeRequesterWorker->restartSearch();

    // A canceled search never reports back (the worker skips searchItemsCreated), so the
    // reset opened by addRootItems() and the UI block would stay pending forever. Close
    // both here so the header is re-enabled when the search is dismissed mid-flight.
    if (mSearchInFlight)
    {
        mSearchInFlight = false;
        rootItemsLoaded();
        sendBlockUiSignal(false);
    }
}

void NodeSelectorModelSearch::setAllowedTabTypes(TabTypes allowedTypes)
{
    mAllowedTabTypes = allowedTypes;
}

int NodeSelectorModelSearch::rootItemsCount() const
{
    return 0;
}

QModelIndex NodeSelectorModelSearch::getTopRootIndex() const
{
    return QModelIndex();
}

bool NodeSelectorModelSearch::canFetchMore(const QModelIndex& parent) const
{
    Q_UNUSED(parent)
    return false;
}

QVariant NodeSelectorModelSearch::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }
    return NodeSelectorModel::data(index, role);
}

QVariant NodeSelectorModelSearch::getDisplayText(NodeSelectorModelItem* item) const
{
    if (item->isMyBackupsFolder())
    {
        return MegaNodeNames::getBackupsName();
    }
    if (item->isDeviceFolder())
    {
        const auto deviceId = QString::fromUtf8(item->getNode()->getDeviceId());
        const auto deviceName = mDeviceNamesRequest->getDeviceName(deviceId);
        if (!deviceName.isEmpty())
        {
            return deviceName;
        }
    }

    return NodeSelectorModel::getDisplayText(item);
}

bool NodeSelectorModelSearch::addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes,
                                       const QModelIndex& parent)
{
    if (mFlattenSearchResults)
    {
        emit requestAddSearchRootItem(nodes, mAllowedTabTypes);
        return true;
    }

    if (parent.isValid())
    {
        return NodeSelectorModel::addNodes(nodes, parent);
    }

    emit requestAddSearchPathItems(nodes, mAllowedTabTypes);
    return true;
}

bool NodeSelectorModelSearch::rootNodeUpdated(mega::MegaNode* node)
{
    if (node->getChanges() & MegaNode::CHANGE_TYPE_INSHARE)
    {
        if (node->isInShare())
        {
            auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
            if (!index.isValid())
            {
                QList<std::shared_ptr<mega::MegaNode>> nodes;
                emit requestAddSearchRootItem(nodes
                                                  << std::shared_ptr<mega::MegaNode>(node->copy()),
                                              mAllowedTabTypes);
            }
            else
            {
                // See NodeSelectorModelIncomingShares::rootNodeUpdated: the item (and its
                // subtree) must receive the permission change, not just repaint.
                updateItemNode(index, std::shared_ptr<mega::MegaNode>(node->copy()));
            }
        }

        return true;
    }
    else if (node->getChanges() & (MegaNode::CHANGE_TYPE_PARENT))
    {
        auto indexToChangeParent = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if (indexToChangeParent.isValid())
        {
            auto item = getItemByIndex(indexToChangeParent);
            if (auto searchItem = dynamic_cast<NodeSelectorModelItemSearch*>(item))
            {
                auto newType(calculateSearchType(node));
                auto oldType(searchItem->getType());
                // The file still exists but in other search type: CD, incoming, rubbish...
                if (newType != oldType)
                {
                    searchItem->setType(newType);
                    updateRow(indexToChangeParent);
                    emit nodeTypeHasChanged();
                    if (isMovingNodes())
                    {
                        // The loading view is set, so reduce by one the processed move
                        moveProcessedByNumber(1);
                    }

                    // Remove parents if needed
                    if (!mFlattenSearchResults)
                    {
                        // Collect search-matching descendants BEFORE removing the subtree
                        QList<std::shared_ptr<mega::MegaNode>> nodesToReadd;
                        collectSearchMatchesInSubtree(indexToChangeParent, nodesToReadd);

                        if (matchesCurrentSearch(node))
                        {
                            nodesToReadd.prepend(std::shared_ptr<mega::MegaNode>(node->copy()));
                        }

                        QList<mega::MegaHandle> handlesToRemove;
                        handlesToRemove.append(node->getHandle());

                        auto ancestorIndex = indexToChangeParent.parent();
                        while (ancestorIndex.isValid())
                        {
                            auto ancestorItem = getItemByIndex(ancestorIndex);
                            if (!ancestorItem || ancestorItem->getNumChildren() != 1 ||
                                matchesCurrentSearch(ancestorItem->getNode().get()))
                            {
                                break;
                            }
                            handlesToRemove.append(ancestorItem->getNode()->getHandle());
                            ancestorIndex = ancestorIndex.parent();
                        }

                        foreach(auto handle, handlesToRemove)
                        {
                            beginRemoveRowsAsync(handle);
                        }

                        if (!nodesToReadd.isEmpty())
                        {
                            emit requestAddSearchPathItems(nodesToReadd, mAllowedTabTypes);
                        }
                    }

                    return true;
                }
            }
        }
    }
    else if (node->getChanges() & MegaNode::CHANGE_TYPE_REMOVED)
    {
        auto index = findIndexByNodeHandle(node->getHandle(), QModelIndex());
        if (index.isValid())
        {
            beginRemoveRowsAsync(node->getHandle());
            return true;
        }
    }

    auto folderIndex = findIndexByNodeHandle(node->getHandle(), QModelIndex());
    if (folderIndex.isValid())
    {
        updateItemNode(folderIndex, std::shared_ptr<mega::MegaNode>(node->copy()));
        return true;
    }

    return false;
}

bool NodeSelectorModelSearch::canDropMimeData(const QMimeData*,
                                              Qt::DropAction,
                                              int,
                                              int,
                                              const QModelIndex&) const
{
    return canDropMimeData();
}

bool NodeSelectorModelSearch::canDropMimeData() const
{
    return false;
}

bool NodeSelectorModelSearch::canCopyNodes() const
{
    return false;
}

void NodeSelectorModelSearch::proxyInvalidateFinished()
{
    mNodeRequesterWorker->lockSearchMutex(false);
}

void NodeSelectorModelSearch::onRootItemsCreated()
{
    // Results from a search stopped after the worker had already finished: the pending
    // reset was closed in stopSearch(), so discard them instead of double-ending it.
    if (!mSearchInFlight)
    {
        return;
    }

    if (mNodeRequesterWorker->trySearchLock())
    {
        mSearchInFlight = false;
        rootItemsLoaded();
        emit levelsAdded(mIndexesToBeExpanded, true);
    }
}

void NodeSelectorModelSearch::onSearchPathItemsAdded()
{
    if (!mSearchPathItemsAddedDebounce.isActive())
    {
        mSearchPathItemsAddedDebounce.start();
    }
}

bool NodeSelectorModelSearch::matchesCurrentSearch(mega::MegaNode* node) const
{
    if (!node || mLastSearchText.isEmpty())
    {
        return false;
    }
    return QString::fromUtf8(node->getName()).contains(mLastSearchText, Qt::CaseInsensitive);
}

void NodeSelectorModelSearch::collectSearchMatchesInSubtree(
    const QModelIndex& parent,
    QList<std::shared_ptr<mega::MegaNode>>& matches) const
{
    const auto rows = rowCount(parent);
    for (int i = 0; i < rows; ++i)
    {
        const auto childIndex = index(i, 0, parent);
        if (!childIndex.isValid())
        {
            continue;
        }
        auto childItem = getItemByIndex(childIndex);
        if (!childItem)
        {
            continue;
        }
        auto childNode = childItem->getNode();
        if (childNode && matchesCurrentSearch(childNode.get()))
        {
            matches.append(std::shared_ptr<mega::MegaNode>(childNode->copy()));
        }
        collectSearchMatchesInSubtree(childIndex, matches);
    }
}

const TabTypes& NodeSelectorModelSearch::searchedTypes() const
{
    return mNodeRequesterWorker->searchedTypes();
}

int NodeSelectorModelSearch::searchResultCount() const
{
    return mNodeRequesterWorker->lastSearchResultCount();
}

TabTypes NodeSelectorModelSearch::calculateSearchType(mega::MegaNode* node)
{
    TabTypes type;

    if (MegaSyncApp->getMegaApi()->isInCloud(node))
    {
        type = TabType::CLOUD_DRIVE;
    }
    else if (MegaSyncApp->getMegaApi()->isInVault(node))
    {
        type = TabType::BACKUP;
    }
    else if (MegaSyncApp->getMegaApi()->isInRubbish(node))
    {
        type = TabType::RUBBISH;
    }
    else
    {
        type = TabType::INCOMING_SHARE;
    }

    return type;
}

///////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelRubbish::NodeSelectorModelRubbish(QObject* parent):
    NodeSelectorModel(parent)
{
    setAcceptDragAndDrop(true);
}

void NodeSelectorModelRubbish::onItemInfoUpdated(int role)
{
    if (NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(sender()))
    {
        for (int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx =
                index(i, NodeSelectorModel::Column::USER); // we only update this column because we
                                                           // retrieve the data in async mode
            if (idx.isValid()) // so it is possible that we doesn´t have the information from the
                               // start
            {
                if (NodeSelectorModelItem* chkItem =
                        static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
                {
                    if (chkItem == item)
                    {
                        QVector<int> roles;
                        roles.append(role);
                        emit dataChanged(idx, idx, roles);
                        break;
                    }
                }
            }
        }
    }
}

void NodeSelectorModelRubbish::onRootItemsCreated()
{
    rootItemsLoaded();

    // Add the item of the Cloud Drive
    auto rootIndex(index(0, 0));
    if (canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
        auto rubbishItem(getItemByIndex(rootIndex));
        if (rubbishItem)
        {
            mIndexesToBeExpanded.append(qMakePair(rubbishItem->getNode()->getHandle(), rootIndex));
        }
    }
    else
    {
        // In case the root item is empty (CD empty), let the model know that we have finished
        loadLevelFinished();
        sendBlockUiSignal(false);
    }
}

void NodeSelectorModelRubbish::createRootNodes()
{
    emit requestRubbishRootCreation();
}

int NodeSelectorModelRubbish::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelRubbish::fetchMore(const QModelIndex& parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelRubbish::firstLoad()
{
    connect(this,
            &NodeSelectorModelRubbish::requestRubbishRootCreation,
            mNodeRequesterWorker,
            &NodeRequester::createRubbishRootItems);
    connect(mNodeRequesterWorker,
            &NodeRequester::megaRubbishRootItemsCreated,
            this,
            &NodeSelectorModelRubbish::onRootItemsCreated,
            Qt::QueuedConnection);

    addRootItems();
}

bool NodeSelectorModelRubbish::isNodeAccepted(MegaNode* node)
{
    return MegaSyncApp->getMegaApi()->isInRubbish(node);
}

bool NodeSelectorModelRubbish::canDropMimeData(const QMimeData* data,
                                               Qt::DropAction action,
                                               int,
                                               int,
                                               const QModelIndex& parent) const
{
    return false;
}

bool NodeSelectorModelRubbish::canCopyNodes() const
{
    return false;
}
