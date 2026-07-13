#include "NodeSelectorModel.h"

#include "CameraUploadFolder.h"
#include "DuplicatedNodeConflictAutoResolution.h"
#include "IconTokenizer.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MergeMEGAFolders.h"
#include "MyBackupsHandle.h"
#include "MyChatFilesFolder.h"
#include "NodeSelectorLabelColors.h"
#include "NodeSelectorModelSpecialised.h"
#include "RequestListenerManager.h"
#include "TokenParserWidgetManager.h"
#include "Utilities.h"

#include <QApplication>
#include <QCoreApplication>
#include <QEventLoop>
#include <QFont>
#include <QPainter>
#include <QToolTip>
#include <QUrl>

const char* INDEX_PROPERTY = "INDEX";

NodeRequester::NodeRequester(NodeSelectorModel* model):
    QObject(nullptr),
    mModel(model),
    mCancelToken(mega::MegaCancelToken::createInstance())
{}

NodeRequester::~NodeRequester()
{
    qDeleteAll(mRootItems);
}

void NodeRequester::lockDataMutex(bool state) const
{
    state ? mDataMutex.lock() : mDataMutex.unlock();
}

bool NodeRequester::isRequestingNodes() const
{
    return mNodesRequested.load();
}

bool NodeRequester::trySearchLock() const
{
    return mSearchMutex.tryLock();
}

void NodeRequester::lockSearchMutex(bool state) const
{
    state ? mSearchMutex.lock() : mSearchMutex.unlock();
}

void NodeRequester::requestNodeAndCreateChildren(NodeSelectorModelItem* item,
                                                 const QModelIndex& parentIndex)
{
    if (item)
    {
        auto node = item->getNode();
        item->setProperty(INDEX_PROPERTY, parentIndex);
        if (!item->requestingChildren() && !item->areChildrenInitialized())
        {
            item->setRequestingChildren(true);
            mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

            mNodesRequested = true;

            std::unique_ptr<mega::MegaSearchFilter> searchFilter(
                mega::MegaSearchFilter::createInstance());
            searchFilter->byNodeType(mShowFiles ? mega::MegaNode::TYPE_UNKNOWN :
                                                  mega::MegaNode::TYPE_FOLDER);
            searchFilter->byLocationHandle(node->getHandle());

            std::unique_ptr<mega::MegaNodeList> childNodesFiltered(
                megaApi->getChildren(searchFilter.get(),
                                     mega::MegaApi::ORDER_NONE,
                                     mCancelToken.get()));
            mNodesRequested = false;
            if (!isAborted())
            {
                lockDataMutex(true);
                auto childItems = item->createChildItems(std::move(childNodesFiltered));
                lockDataMutex(false);
                const auto childCount = childItems.size();
                if (childCount > 0)
                {
                    QMetaObject::invokeMethod(mModel,
                                              "beginChildRowsInsertion",
                                              Qt::BlockingQueuedConnection,
                                              Q_ARG(QModelIndex, parentIndex),
                                              Q_ARG(int, 0),
                                              Q_ARG(int, childCount - 1));
                }

                lockDataMutex(true);
                item->initializeChildItems(childItems);
                lockDataMutex(false);
                emit nodesReady(item, static_cast<int>(childCount));
            }
        }
    }
}

void NodeRequester::search(const QString& text, TabTypes typesAllowed, bool flatten)
{
    if (text.isEmpty())
    {
        return;
    }

    {
        QMutexLocker a(&mSearchMutex);
        QMutexLocker d(&mDataMutex);
        qDeleteAll(mRootItems);
        mRootItems.clear();
    }
    mSearchCanceled = false;

    std::unique_ptr<mega::MegaSearchFilter> searchFilter(mega::MegaSearchFilter::createInstance());
    searchFilter->byName(text.toUtf8().constData());

    auto nodeList = std::unique_ptr<mega::MegaNodeList>(
        MegaSyncApp->getMegaApi()->search(searchFilter.get(),
                                          mega::MegaApi::ORDER_NONE,
                                          mCancelToken.get()));
    QList<NodeSelectorModelItem*> items;
    mSearchedTypes = TabType::NONE;
    int validMatches = 0;

    // Items already placed in the result tree, keyed by node handle (each node appears at most
    // once in the tree, so the handle is a unique key). Turns the per-path lookups from linear
    // scans -- O(N^2) with tens of thousands of results under the same parents -- into O(1)
    // hash lookups.
    QHash<mega::MegaHandle, NodeSelectorModelItem*> alreadyProcessedItemsByHandle;
    alreadyProcessedItemsByHandle.reserve(nodeList->size());

    for (int i = 0; i < nodeList->size(); i++)
    {
        auto type = NodeSelectorModelSearch::calculateSearchType(nodeList->get(i));
        if ((typesAllowed & type) && canCreateSearchItem(nodeList->get(i)))
        {
            ++validMatches;
            if (flatten)
            {
                mSearchedTypes |= type;
                if (auto item = createSearchTreeItem(nodeList->get(i), type))
                {
                    items.append(item);
                }
            }
            else
            {
                auto path = createSearchPath(nodeList->get(i), type);
                addSearchPath(items, path, type, {}, &alreadyProcessedItemsByHandle);
            }
        }
    }

    if (isAborted() || mSearchCanceled)
    {
        qDeleteAll(items);
    }
    else
    {
        mLastSearchResultCount.store(validMatches);
        QMutexLocker d(&mDataMutex);
        mRootItems.append(items);
        emit searchItemsCreated();
    }
}

int NodeRequester::lastSearchResultCount() const
{
    return mLastSearchResultCount.load();
}

void NodeRequester::addSearchRootItem(QList<std::shared_ptr<mega::MegaNode>> nodes,
                                      TabTypes typesAllowed)
{
    QList<NodeSelectorModelItem*> items;
    foreach(auto node, nodes)
    {
        auto item = createSearchItem(node.get(), typesAllowed);
        if (item)
        {
            items.append(item);
        }
    }

    if (isAborted())
    {
        qDeleteAll(items);
    }
    else if (!items.isEmpty())
    {
        appendRootItems(items);
    }
}

void NodeRequester::appendRootItems(const QList<NodeSelectorModelItem*>& items)
{
    if (items.isEmpty())
    {
        return;
    }

    const int firstRow = rootIndexSize();
    QMetaObject::invokeMethod(mModel,
                              "beginRootItemsInsertion",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(int, firstRow),
                              Q_ARG(int, firstRow + items.size() - 1));

    {
        QMutexLocker d(&mDataMutex);
        mRootItems.append(items);
    }

    emit rootItemsAdded();
}

NodeSelectorModelItem* NodeRequester::createSearchItem(mega::MegaNode* node, TabTypes typesAllowed)
{
    TabTypes type = NodeSelectorModelSearch::calculateSearchType(node);

    if ((typesAllowed & type) && canCreateSearchItem(node))
    {
        mSearchedTypes |= type;
        return createSearchTreeItem(node, type);
    }

    return nullptr;
}

NodeSelectorModelItemSearch* NodeRequester::createSearchTreeItem(mega::MegaNode* node,
                                                                 TabTypes type)
{
    auto nodeUptr = std::unique_ptr<mega::MegaNode>(node->copy());
    auto item = new NodeSelectorModelItemSearch(std::move(nodeUptr), type);
    if (item->isValid())
    {
        connect(item,
                &NodeSelectorModelItemSearch::tabTypeChanged,
                this,
                &NodeRequester::onSearchItemTypeChanged);
        return item;
    }

    item->deleteLater();
    return nullptr;
}

bool NodeRequester::canCreateSearchItem(mega::MegaNode* node)
{
    if (isAborted() || mSearchCanceled)
    {
        return false;
    }
    if ((node->isFile() && !mShowFiles))
    {
        return false;
    }
    else if (!mShowReadOnlyFolders)
    {
        if (MegaSyncApp->getMegaApi()->getAccess(node) == mega::MegaShare::ACCESS_READ ||
            !node->isNodeKeyDecrypted())
        {
            return false;
        }
    }

    return true;
}

QList<std::shared_ptr<mega::MegaNode>> NodeRequester::createSearchPath(mega::MegaNode* node,
                                                                       TabTypes type) const
{
    QList<std::shared_ptr<mega::MegaNode>> path;

    auto megaApi = MegaSyncApp->getMegaApi();
    auto currentNode = std::shared_ptr<mega::MegaNode>(node->copy());
    while (currentNode && !isSearchRootNode(currentNode.get(), type))
    {
        path.prepend(currentNode);
        currentNode.reset(megaApi->getParentNode(currentNode.get()));
    }

    return path;
}

void NodeRequester::addSearchPathItems(QList<std::shared_ptr<mega::MegaNode>> nodes,
                                       TabTypes typesAllowed)
{
    // Adds a children, so we need to call beginChildRowsInsertion
    auto appendChildren = [this](NodeSelectorModelItem* parentItem,
                                 const QList<std::shared_ptr<mega::MegaNode>>& children)
        -> QList<QPointer<NodeSelectorModelItem>>
    {
        if (!parentItem)
        {
            return {};
        }
        auto parentIndex =
            mModel->findIndexByNodeHandle(parentItem->getNode()->getHandle(), QModelIndex());
        if (!parentIndex.isValid())
        {
            return {};
        }
        return onAddNodesRequested(children, parentIndex, parentItem);
    };

    bool anyAdded = false;
    foreach(auto node, nodes)
    {
        if (!canCreateSearchItem(node.get()))
        {
            continue;
        }

        TabTypes type = NodeSelectorModelSearch::calculateSearchType(node.get());
        if (!(typesAllowed & type))
        {
            continue;
        }

        auto path = createSearchPath(node.get(), type);
        if (path.isEmpty())
        {
            continue;
        }

        addSearchPath(mRootItems, path, type, appendChildren);
        anyAdded = true;
    }

    if (anyAdded)
    {
        emit searchPathItemsAdded();
    }
}

void NodeRequester::addSearchPath(QList<NodeSelectorModelItem*>& items,
                                  const QList<std::shared_ptr<mega::MegaNode>>& path,
                                  TabTypes type,
                                  AppendChildrenFn appendChildren,
                                  QHash<mega::MegaHandle, NodeSelectorModelItem*>* handleIndex)
{
    if (path.isEmpty())
    {
        return;
    }

    mSearchedTypes |= type;

    NodeSelectorModelItem* parentItem(nullptr);
    for (const auto& node: path)
    {
        const auto handle = node->getHandle();

        auto existingItem = handleIndex ? handleIndex->value(handle, nullptr) :
                                          (parentItem ? findSearchChild(parentItem, handle) :
                                                        findSearchItem(items, handle));
        if (!existingItem)
        {
            if (parentItem)
            {
                const auto newItems = appendChildren ? appendChildren(parentItem, {node}) :
                                                       parentItem->addNodes({node});
                existingItem = newItems.isEmpty() ? nullptr : newItems.first().data();
                if (auto searchItem = dynamic_cast<NodeSelectorModelItemSearch*>(existingItem))
                {
                    connect(searchItem,
                            &NodeSelectorModelItemSearch::tabTypeChanged,
                            this,
                            &NodeRequester::onSearchItemTypeChanged);
                }
            }
            else
            {
                existingItem = createSearchTreeItem(node.get(), type);
                if (existingItem)
                {
                    if (appendChildren)
                    {
                        appendRootItems({existingItem});
                    }
                    else
                    {
                        items.append(existingItem);
                    }
                }
            }
        }

        if (!existingItem)
        {
            return;
        }

        if (handleIndex)
        {
            handleIndex->insert(handle, existingItem);
        }

        parentItem = existingItem;
    }
}

NodeSelectorModelItem* NodeRequester::findSearchItem(const QList<NodeSelectorModelItem*>& items,
                                                     mega::MegaHandle handle) const
{
    for (auto item: items)
    {
        if (item && item->getNode() && item->getNode()->getHandle() == handle)
        {
            return item;
        }
    }

    return nullptr;
}

NodeSelectorModelItem* NodeRequester::findSearchChild(NodeSelectorModelItem* parent,
                                                      mega::MegaHandle handle) const
{
    if (!parent)
    {
        return nullptr;
    }

    for (int i = 0; i < parent->getNumChildren(); ++i)
    {
        auto child = parent->getChild(i);
        if (child && child->getNode() && child->getNode()->getHandle() == handle)
        {
            return child;
        }
    }

    return nullptr;
}

bool NodeRequester::isSearchRootNode(mega::MegaNode* node, TabTypes type) const
{
    auto isNode = [node](const std::shared_ptr<mega::MegaNode>& rootNode)
    {
        return rootNode && node && rootNode->getHandle() == node->getHandle();
    };

    if (type.testFlag(TabType::CLOUD_DRIVE))
    {
        return isNode(MegaSyncApp->getRootNode());
    }
    if (type.testFlag(TabType::BACKUP))
    {
        // The root item displayed in the Backups tab is the "My Backups" folder, not the real
        // vault node (which is its parent). Stop the path here so "My Backups" is not added.
        auto backupsHandle =
            UserAttributes::MyBackupsHandle::requestMyBackupsHandle()->getMyBackupsHandle();
        return node && node->getHandle() == backupsHandle;
    }
    if (type.testFlag(TabType::RUBBISH))
    {
        return isNode(MegaSyncApp->getRubbishNode());
    }

    return false;
}

void NodeRequester::createCloudDriveRootItem()
{
    auto root = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getRootNode());
    if (!isAborted())
    {
        auto item = new NodeSelectorModelItemCloudDrive(std::move(root), mShowFiles);
        if (item->isValid())
        {
            mRootItems.append(item);
        }
        else
        {
            mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR,
                               "Root Cloud Drive node unavailable.");
            item->deleteLater();
        }

        emit megaCloudDriveRootItemCreated();
    }
}

bool NodeRequester::isIncomingShareCompatible(mega::MegaNode* node)
{
    if (!mShowReadOnlyFolders)
    {
        if (MegaSyncApp->getMegaApi()->getAccess(node) == mega::MegaShare::ACCESS_READ ||
            !node->isNodeKeyDecrypted())
        {
            return false;
        }
    }

    return true;
}

void NodeRequester::createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList)
{
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    QList<NodeSelectorModelItem*> items;
    for (int i = 0; i < nodeList->size(); i++)
    {
        if (isAborted())
        {
            break;
        }

        if (!isIncomingShareCompatible(nodeList->get(i)))
        {
            continue;
        }

        auto node = std::unique_ptr<mega::MegaNode>(nodeList->get(i)->copy());
        auto user = std::unique_ptr<mega::MegaUser>(megaApi->getUserFromInShare(node.get()));
        NodeSelectorModelItem* item =
            new NodeSelectorModelItemIncomingShare(std::move(node), mShowFiles);

        if (item->isValid())
        {
            items.append(item);

            auto incomingSharesModel = dynamic_cast<NodeSelectorModelIncomingShares*>(mModel);
            if (incomingSharesModel)
            {
                item->setProperty(INDEX_PROPERTY, incomingSharesModel->index(i, 0));
                connect(item,
                        &NodeSelectorModelItem::infoUpdated,
                        incomingSharesModel,
                        &NodeSelectorModelIncomingShares::onItemInfoUpdated);
                item->setOwner(std::move(user));
            }
        }
        else
        {
            item->deleteLater();
        }
    }

    if (isAborted())
    {
        qDeleteAll(items);
    }
    else
    {
        mRootItems.append(items);
        emit megaIncomingSharesRootItemsCreated();
    }
}

void NodeRequester::addIncomingSharesRootItem(std::shared_ptr<mega::MegaNode> node)
{
    if (isAborted())
    {
        return;
    }

    if (!isIncomingShareCompatible(node.get()))
    {
        return;
    }

    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
    auto user = std::unique_ptr<mega::MegaUser>(megaApi->getUserFromInShare(node.get()));
    auto item =
        new NodeSelectorModelItemIncomingShare(std::unique_ptr<mega::MegaNode>(node->copy()),
                                               mShowFiles);

    if (item->isValid())
    {
        auto incomingSharesModel = dynamic_cast<NodeSelectorModelIncomingShares*>(mModel);
        if (incomingSharesModel)
        {
            item->setProperty(INDEX_PROPERTY,
                              incomingSharesModel->index(incomingSharesModel->rowCount(), 0));
            connect(item,
                    &NodeSelectorModelItem::infoUpdated,
                    incomingSharesModel,
                    &NodeSelectorModelIncomingShares::onItemInfoUpdated);
            item->setOwner(std::move(user));
        }

        if (isAborted())
        {
            item->deleteLater();
        }
        else
        {
            appendRootItems({item});
        }
    }
    else
    {
        item->deleteLater();
    }
}

void NodeRequester::createRubbishRootItems()
{
    if (!isAborted())
    {
        auto item = new NodeSelectorModelItemRubbish(
            std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getRubbishNode()),
            mShowFiles);
        if (item->isValid())
        {
            mRootItems.append(item);
            emit megaRubbishRootItemsCreated();
        }
        else
        {
            item->deleteLater();
        }
    }
}

void NodeRequester::createBackupRootItems(mega::MegaHandle backupsHandle)
{
    if (backupsHandle != mega::INVALID_HANDLE)
    {
        std::unique_ptr<mega::MegaNode> backupsNode(
            MegaSyncApp->getMegaApi()->getNodeByHandle(backupsHandle));
        if (backupsNode)
        {
            if (!isAborted())
            {
                NodeSelectorModelItem* item =
                    new NodeSelectorModelItemBackup(std::move(backupsNode), mShowFiles);
                // Here we are setting my backups node as vault node in the item, it is not the same
                // vault node that we get doing megaapi->getVaultNode(), we have to hide it here
                // thats why are doing this trick. The real vault is the parent of my backups folder
                mRootItems.append(item);
            }
        }
    }

    if (!isAborted())
    {
        emit megaBackupRootItemsCreated();
    }
}

QList<QPointer<NodeSelectorModelItem>>
    NodeRequester::onAddNodesRequested(QList<std::shared_ptr<mega::MegaNode>> newNodes,
                                       const QModelIndex& parentIndex,
                                       NodeSelectorModelItem* parentItem)
{
    lockDataMutex(true);
    auto lastChild = parentItem->areChildrenInitialized() ? parentItem->getNumChildren() : 0;
    auto childrenItem = parentItem->buildNodes(newNodes);
    lockDataMutex(false);

    const auto childCount = childrenItem.size();
    if (childCount <= 0 || isAborted())
    {
        QMetaObject::invokeMethod(mModel,
                                  "cancelPendingModification",
                                  Qt::BlockingQueuedConnection);
        foreach(auto& childItem, childrenItem)
        {
            childItem->deleteLater();
        }
        return {};
    }

    QMetaObject::invokeMethod(mModel,
                              "beginChildRowsInsertion",
                              Qt::BlockingQueuedConnection,
                              Q_ARG(QModelIndex, parentIndex),
                              Q_ARG(int, lastChild),
                              Q_ARG(int, lastChild + childCount - 1));

    lockDataMutex(true);
    parentItem->initializeChildItems(childrenItem);
    lockDataMutex(false);

    foreach(auto& childItem, childrenItem)
    {
        childItem->setProperty(INDEX_PROPERTY, mModel->index(lastChild, 0, parentIndex));
        lastChild++;
    }

    emit nodesAdded(childrenItem);
    return childrenItem;
}

void NodeRequester::removeItem(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mDataMutex);
    item->deleteLater();
}

void NodeRequester::removeRootItem(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mDataMutex);
    item->deleteLater();
    mRootItems.removeOne(item);
    emit rootItemsDeleted();
}

void NodeRequester::removeRootItem(std::shared_ptr<mega::MegaNode> node)
{
    if (isAborted())
    {
        return;
    }

    auto rootFound = std::find_if(mRootItems.begin(),
                                  mRootItems.end(),
                                  [node](NodeSelectorModelItem* item)
                                  {
                                      return item->getNode()->getHandle() == node->getHandle();
                                  });

    if (rootFound != mRootItems.end())
    {
        mRootItems.removeOne(*rootFound);
        emit rootItemsDeleted();
    }
}

int NodeRequester::rootIndexSize() const
{
    QMutexLocker lock(&mDataMutex);
    return static_cast<int>(mRootItems.size());
}

int NodeRequester::rootIndexOf(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mDataMutex);
    return static_cast<int>(mRootItems.indexOf(item));
}

NodeSelectorModelItem* NodeRequester::getRootItem(int index) const
{
    QMutexLocker lock(&mDataMutex);
    return mRootItems.at(index);
}

void NodeRequester::restartSearch()
{
    if (mCancelToken)
    {
        mCancelToken->cancel();
        mSearchCanceled = true;
        mCancelToken.reset(mega::MegaCancelToken::createInstance());
    }
}

void NodeRequester::cancelCurrentRequest()
{
    if (mCancelToken)
    {
        mSearchCanceled = true;
        mCancelToken->cancel();
    }
}

bool NodeRequester::isAborted()
{
    return mAborted || (mCancelToken && mCancelToken->isCancelled());
}

bool NodeRequester::showFiles() const
{
    return mShowFiles.load();
}

const TabTypes& NodeRequester::searchedTypes() const
{
    return mSearchedTypes;
}

void NodeRequester::setShowFiles(bool show)
{
    mShowFiles = show;
}

void NodeRequester::setShowReadOnlyFolders(bool show)
{
    mShowReadOnlyFolders = show;
}

void NodeRequester::setSyncSetupMode(bool value)
{
    mSyncSetupMode = value;
}

void NodeRequester::abort()
{
    cancelCurrentRequest();
    mAborted = true;
}

void NodeRequester::onSearchItemTypeChanged(TabTypes type)
{
    mSearchedTypes |= type;
}

/* ------------------- MODEL ------------------------- */

const QString MIME_DATA_INTERNAL_MOVE = QLatin1String("application/node_move");

NodeSelectorModel::NodeSelectorModel(QObject* parent):
    QAbstractItemModel(parent),
    mSyncSetupMode(false),
    mIsBeingModified(false),
    mIsProcessingMoves(false),
    mAcceptDragAndDrop(false),
    mAddNodesQueue(this),
    mRemoveNodesQueue(this),
    mExtraSpaceAdded(false),
    mExtraSpaceRemoved(false),
    mRemovingPreviousExtraSpace(false),
    mExtraSpaceEnabled(true)
{
    mCameraFolderAttribute = UserAttributes::CameraUploadFolder::requestCameraUploadFolder();
    mMyChatFilesFolderAttribute = UserAttributes::MyChatFilesFolder::requestMyChatFilesFolder();

    mNodeRequesterThread = new QThread();
    mNodeRequesterWorker = new NodeRequester(this);
    mNodeRequesterWorker->moveToThread(mNodeRequesterThread);
    mNodeRequesterThread->start();

    connect(this,
            &NodeSelectorModel::requestChildNodes,
            mNodeRequesterWorker,
            &NodeRequester::requestNodeAndCreateChildren,
            Qt::QueuedConnection);
    connect(this,
            &NodeSelectorModel::requestAddNodes,
            mNodeRequesterWorker,
            &NodeRequester::onAddNodesRequested,
            Qt::QueuedConnection);
    connect(this, &NodeSelectorModel::removeItem, mNodeRequesterWorker, &NodeRequester::removeItem);
    connect(this,
            &NodeSelectorModel::removeRootItem,
            this,
            [this](NodeSelectorModelItem* item)
            {
                mNodeRequesterWorker->removeRootItem(item);
            });

    connect(mNodeRequesterThread,
            &QThread::finished,
            mNodeRequesterThread,
            &QObject::deleteLater,
            Qt::DirectConnection);
    connect(mNodeRequesterThread,
            &QThread::finished,
            mNodeRequesterWorker,
            &QObject::deleteLater,
            Qt::DirectConnection);

    connect(mNodeRequesterWorker,
            &NodeRequester::nodesReady,
            this,
            &NodeSelectorModel::onChildNodesReady,
            Qt::QueuedConnection);
    connect(mNodeRequesterWorker,
            &NodeRequester::nodesAdded,
            this,
            &NodeSelectorModel::onNodesAdded,
            Qt::QueuedConnection);

    connect(mNodeRequesterWorker,
            &NodeRequester::rootItemsAdded,
            this,
            &NodeSelectorModel::onRootItemAdded,
            Qt::QueuedConnection);

    connect(SyncInfo::instance(),
            &SyncInfo::syncStateChanged,
            this,
            &NodeSelectorModel::onSyncStateChanged);
    connect(SyncInfo::instance(),
            &SyncInfo::syncRemoved,
            this,
            &NodeSelectorModel::onSyncStateChanged);

    connect(this,
            &NodeSelectorModel::finishAsyncRequest,
            this,
            &NodeSelectorModel::checkFinishedRequest,
            Qt::QueuedConnection);

    qRegisterMetaType<std::shared_ptr<mega::MegaNodeList>>("std::shared_ptr<mega::MegaNodeList>");
    qRegisterMetaType<std::shared_ptr<mega::MegaNode>>("std::shared_ptr<mega::MegaNode>");
    qRegisterMetaType<mega::MegaHandle>("mega::MegaHandle");
    qRegisterMetaType<QList<mega::MegaHandle>>("QList<mega::MegaHandle>");
    qRegisterMetaType<QSet<mega::MegaHandle>>("QSet<mega::MegaHandle>");
    qRegisterMetaType<QList<std::shared_ptr<NodeSelectorMergeInfo>>>(
        "QList<std::shared_ptr<MergeInfo>>");

    protectModelWhenPerformingActions();

    mListener = RequestListenerManager::instance().registerAndGetFinishListener(this, false);

    connect(&mRemoveNodesQueue,
            &RemoveNodesQueue::startBeginRemoveRows,
            this,
            &NodeSelectorModel::onStartBeginRemoveRowsAsync);
}

NodeSelectorModel::~NodeSelectorModel()
{
    // Cancel any in-flight request so the worker stops issuing new blocking
    // calls back to this (GUI) thread.
    mNodeRequesterWorker->abort();

    mNodeRequesterThread->quit();

    // The worker may currently be parked on a BlockingQueuedConnection (e.g.
    // beginChildRowsInsertion) waiting for THIS thread to service its posted
    // event. A plain wait() would deadlock: the worker cannot finish until we
    // process that event, and we would never return to the event loop. Keep
    // pumping our event loop until the worker thread has actually finished.
    while (!mNodeRequesterThread->wait(50))
    {
        QCoreApplication::processEvents(QEventLoop::ExcludeUserInputEvents);
    }
}

void NodeSelectorModel::setIsModelBeingModified(bool state)
{
    mIsBeingModified = state;
    emit modelIsBeingModifiedChanged(state);
}

void NodeSelectorModel::cancelPendingModification()
{
    if (mIsBeingModified)
    {
        setIsModelBeingModified(false);
    }
}

void NodeSelectorModel::protectModelWhenPerformingActions()
{
    auto protectModel = [this]()
    {
        setIsModelBeingModified(true);
    };

    connect(this, &NodeSelectorModel::rowsAboutToBeInserted, protectModel);
    connect(this, &NodeSelectorModel::rowsAboutToBeRemoved, protectModel);
    connect(this, &NodeSelectorModel::rowsAboutToBeMoved, protectModel);
    connect(this, &NodeSelectorModel::modelAboutToBeReset, protectModel);

    auto unprotectModel = [this]()
    {
        setIsModelBeingModified(false);
    };
    connect(this, &NodeSelectorModel::rowsInserted, unprotectModel);
    connect(this, &NodeSelectorModel::rowsRemoved, unprotectModel);
    connect(this, &NodeSelectorModel::modelReset, unprotectModel);
    connect(this, &NodeSelectorModel::rowsMoved, unprotectModel);
}

void NodeSelectorModel::executeRemoveExtraSpaceLogic(const QModelIndex& previousIndex)
{
    if (mExtraSpaceEnabled)
    {
        // Remove the previous current index extra row
        if (mExtraSpaceAdded && previousIndex.isValid() && !mExtraSpaceRemoved)
        {
            mRemovingPreviousExtraSpace = true;

            auto lastRow = rowCount(previousIndex) - 1;
            beginRemoveRows(previousIndex, lastRow, lastRow);
            endRemoveRows();

            mRemovingPreviousExtraSpace = false;
            mExtraSpaceRemoved = true;
            mExtraSpaceAdded = false;
        }
    }
}

void NodeSelectorModel::executeAddExtraSpaceLogic(const QModelIndex& currentIndex)
{
    if (isBeingModified())
    {
        // Defer until the pending begin/endInsertRows is closed.
        QMetaObject::invokeMethod(
            this,
            [this, currentIndex]()
            {
                executeAddExtraSpaceLogic(currentIndex);
            },
            Qt::QueuedConnection);
        return;
    }

    if (mExtraSpaceEnabled)
    {
        NodeSelectorModelItem* item =
            static_cast<NodeSelectorModelItem*>(currentIndex.internalPointer());
        if (item && item->areChildrenInitialized())
        {
            if (currentIndex.isValid() && !mExtraSpaceAdded)
            {
                auto currentRowCount(rowCount(currentIndex));
                if (currentRowCount > 0)
                {
                    auto totalRows = rowCount(currentIndex);
                    beginInsertRows(currentIndex, totalRows, totalRows);
                    endInsertRows();
                    mExtraSpaceAdded = true;
                    mExtraSpaceRemoved = false;
                }
            }
        }
    }
}

bool NodeSelectorModel::isExtraSpaceIndex(const QModelIndex& index) const
{
    if (!mExtraSpaceAdded || !index.isValid() || index.internalPointer() != nullptr ||
        !mCurrentRootIndex.isValid())
    {
        return false;
    }

    return index.row() == rowCount(mCurrentRootIndex) - 1;
}

int NodeSelectorModel::columnCount(const QModelIndex&) const
{
    return NodeSelectorModel::Column::last;
}

QVariant NodeSelectorModel::data(const QModelIndex& index, int role) const
{
    if (index.isValid())
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());

        switch (role)
        {
            case toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE):
            {
                return isExtraSpaceIndex(index);
            }
            default:
            {
                break;
            }
        }

        if (item)
        {
            switch (role)
            {
                case Qt::DisplayRole:
                {
                    return getText(index, item);
                }
                case Qt::DecorationRole:
                {
                    return getIcon(index, item);
                }
                case Qt::FontRole:
                {
                    QFont font; // This will use the app default font.
                    if (index.column() == NodeSelectorModel::Column::NODE)
                    {
                        font.setPixelSize(12);
                    }
                    else
                    {
                        font.setPixelSize(10);
                    }
                    return font;
                }
                case Qt::ToolTipRole:
                {
                    if (index.column() == NodeSelectorModel::Column::USER)
                    {
                        if (showAccess(item->getNode().get()))
                        {
                            return item->getOwnerName() + QLatin1String(" (") +
                                   item->getOwnerEmail() + QLatin1String(")");
                        }
                        return QVariant();
                    }
                    else if (item->isTakenDown())
                    {
                        if (item->isFile())
                        {
                            return tr("This file has been the subject of a takedown notice");
                        }
                        else
                        {
                            return tr("This folder has been the subject of a takedown notice");
                        }
                    }
                    else if (mSyncSetupMode)
                    {
                        if ((item->getStatus() == NodeSelectorModelItem::Status::SYNC) ||
                            (item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD))
                        {
                            return tr("Folder already synced");
                        }
                        else if (item->getStatus() == NodeSelectorModelItem::Status::SYNC_PARENT)
                        {
                            return tr("Folder contents already synced");
                        }
                        QToolTip::hideText();
                    }
                    break;
                }
                case toInt(NodeSelectorModelRoles::DATE_ROLE):
                {
                    return QVariant::fromValue(item->getNode()->getCreationTime());
                }
                case toInt(NodeSelectorModelRoles::IS_FILE_ROLE):
                {
                    return QVariant::fromValue(item->getNode()->isFile());
                }
                case toInt(NodeSelectorModelRoles::IS_TAKEN_DOWN_ROLE):
                {
                    return QVariant::fromValue(item->isTakenDown());
                }
                case toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE):
                {
                    return QVariant::fromValue(item->isSyncable() && item->getNode()->isFolder());
                }
                case toInt(NodeSelectorModelRoles::STATUS_ROLE):
                {
                    return QVariant::fromValue(item->getStatus());
                }
                case toInt(NodeSelectorModelRoles::ACCESS_ROLE):
                {
                    return item->getNodeAccess();
                }
                case toInt(NodeSelectorModelRoles::HANDLE_ROLE):
                {
                    return QVariant::fromValue(item->getNode() ? item->getNode()->getHandle() :
                                                                 mega::INVALID_HANDLE);
                }
                case toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE):
                {
                    return QVariant::fromValue(item);
                }
                case toInt(NodeSelectorModelRoles::NODE_ROLE):
                {
                    return QVariant::fromValue(item->getNode());
                }
                case toInt(NodeSelectorModelRoles::LABEL_COLOR_ROLE):
                {
                    auto node = item->getNode();
                    return node ? NodeSelectorLabelColors::colorForLabel(node->getLabel()) :
                                  QColor();
                }
                case toInt(NodeSelectorModelRoles::LABEL_ORDER_ROLE):
                {
                    auto node = item->getNode();
                    if (!node)
                    {
                        return QVariant();
                    }

                    const auto label = node->getLabel();
                    return label == mega::MegaNode::NODE_LBL_UNKNOWN ?
                               mega::MegaNode::NODE_LBL_GREY + 1 :
                               label;
                }
                case toInt(NodeSelectorModelRoles::IS_EXPORTED_ROLE):
                {
                    auto node = item->getNode();
                    return node && node->isExported();
                }
                case toInt(NodeRowDelegateRoles::INIT_ROLE):
                {
                    return item->areChildrenInitialized();
                }
                case toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE):
                {
                    if (index.column() == NodeSelectorModel::Column::USER)
                    {
                        return QSize(20, 20);
                    }
                    else if (index.column() == NodeSelectorModel::Column::NODE)
                    {
                        return QSize(24, 24);
                    }
                    else
                    {
                        return QSize(16, 16);
                    }
                }
                default:
                {
                    break;
                }
            }
        }
    }
    return QVariant();
}

Qt::ItemFlags NodeSelectorModel::flags(const QModelIndex& index) const
{
    auto flags = QAbstractItemModel::flags(index);

    if (index.isValid())
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if (item)
        {
            if (item->getNode() && !item->getNode()->isNodeKeyDecrypted())
            {
                flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }

            if (mAcceptDragAndDrop)
            {
                if (item->isTakenDown())
                {
                    flags &= ~(Qt::ItemIsDropEnabled | Qt::ItemIsDragEnabled);
                }
                else
                {
                    flags |= Qt::ItemIsDropEnabled;

                    if (!item->isSpecialNode() && !item->isInShare())
                    {
                        flags |= Qt::ItemIsDragEnabled;
                    }
                }
            }
        }
        // no item -> extra space row
        else if (isExtraSpaceIndex(index))
        {
            flags |= Qt::ItemIsDropEnabled;
            flags &= ~(Qt::ItemIsSelectable);
        }
    }

    return flags;
}

void NodeSelectorModel::setAcceptDragAndDrop(bool newAcceptDragAndDrop)
{
    mAcceptDragAndDrop = newAcceptDragAndDrop;
}

void NodeSelectorModel::setExtraSpaceEnabled(bool enabled)
{
    mExtraSpaceEnabled = enabled;
}

bool NodeSelectorModel::acceptDragAndDrop(const QMimeData* data)
{
    if (data->hasFormat(MIME_DATA_INTERNAL_MOVE))
    {
        return true;
    }

    if (data->hasUrls())
    {
        // Only accept the drop when at least one URL resolves to a local file.
        // A drag&drop from the OS file manager can carry non-local URLs (web
        // images, iCloud files not downloaded, promised files, etc.) that
        // cannot be uploaded.
        const auto urls = data->urls();
        return std::any_of(urls.cbegin(),
                           urls.cend(),
                           [](const QUrl& url)
                           {
                               return !url.toLocalFile().isEmpty();
                           });
    }

    return false;
}

bool NodeSelectorModel::canDropMimeData(const QMimeData* data,
                                        Qt::DropAction action,
                                        int row,
                                        int column,
                                        const QModelIndex& parent) const
{
    if (action == Qt::CopyAction || action == Qt::MoveAction)
    {
        auto dropIndex(index(row, column, parent));

        if (parent.isValid())
        {
            if (action == Qt::CopyAction)
            {
                return true;
            }
            else
            {
                return checkDraggedMimeData(data, dropIndex);
            }
        }
        else
        {
            return checkDraggedMimeData(data, dropIndex);
        }
    }

    return false;
}

bool NodeSelectorModel::canDropMimeData() const
{
    return true;
}

bool NodeSelectorModel::checkDraggedMimeData(const QMimeData* data,
                                             const QModelIndex& dropIndex) const
{
    auto targetItem = getItemByIndex(dropIndex);
    if (targetItem && targetItem->isTakenDown())
    {
        return false;
    }

    QByteArray encodedData = data->data(MIME_DATA_INTERNAL_MOVE);
    QDataStream stream(&encodedData, QIODevice::ReadOnly);

    while (!stream.atEnd())
    {
        quint64 handle;
        stream >> handle;

        if (Utilities::getNodeAccess(handle) < mega::MegaShare::ACCESS_FULL)
        {
            return false;
        }
    }

    return true;
}

QStringList NodeSelectorModel::mimeTypes() const
{
    static QStringList types{MIME_DATA_INTERNAL_MOVE};
    return types;
}

bool NodeSelectorModel::dropMimeData(const QMimeData* data,
                                     Qt::DropAction action,
                                     int row,
                                     int column,
                                     const QModelIndex& parent)
{
    if (action == Qt::CopyAction || action == Qt::MoveAction)
    {
        return startProcessingNodes(data, parent, MoveActionType::MOVE);
    }

    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool NodeSelectorModel::startProcessingNodes(const QMimeData* data,
                                             const QModelIndex& parent,
                                             MoveActionType type)
{
    auto targetIndex(parent.isValid() ? parent : index(0, 0, QModelIndex()));

    if (targetIndex.data(toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE)).toBool())
    {
        targetIndex = mCurrentRootIndex;
    }

    if (targetIndex.isValid())
    {
        if (NodeSelectorModelItem* chkItem =
                static_cast<NodeSelectorModelItem*>(targetIndex.internalPointer()))
        {
            QByteArray encodedData = data->data(MIME_DATA_INTERNAL_MOVE);
            QDataStream stream(&encodedData, QIODevice::ReadOnly);

            // We use this struct as it is the struct accepted by the Conflict manager
            QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>> nodesToMove;

            auto sourceNode = chkItem->getNode();

            mega::MegaHandle targetFolder(mega::INVALID_HANDLE);
            if (sourceNode->isFile())
            {
                targetFolder = sourceNode->getParentHandle();
            }
            else
            {
                targetFolder = sourceNode->getHandle();
            }

            std::shared_ptr<mega::MegaNode> targetNode(
                MegaSyncApp->getMegaApi()->getNodeByHandle(targetFolder));

            while (!stream.atEnd())
            {
                quint64 handle;
                stream >> handle;

                std::unique_ptr<mega::MegaNode> moveNode(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(handle));

                if (type != MoveActionType::COPY)
                {
                    if (moveNode->getParentHandle() == targetFolder ||
                        moveNode->getHandle() == targetFolder)
                    {
                        continue;
                    }
                }

                QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>> p{handle, targetNode};
                nodesToMove.append(p);
            }

            return processNodesAndCheckConflicts(nodesToMove, sourceNode, type);
        }
    }

    return false;
}

void NodeSelectorModel::checkForDuplicatedSourceFilesWhenRestoring(
    std::shared_ptr<ConflictTypes> conflicts)
{
    QHash<mega::MegaHandle, std::shared_ptr<DuplicatedMoveNodeInfo>> nodesToUpload;
    QHash<mega::MegaHandle, std::shared_ptr<DuplicatedMoveNodeInfo>> nodesToUploadAndReplace;

    auto linkDuplicatedConflicts =
        [](std::shared_ptr<DuplicatedMoveNodeInfo> uploadAndReplaceConflict)
    {
        Utilities::removeRemoteFile(uploadAndReplaceConflict->getSourceItemNode().get());
        uploadAndReplaceConflict->setSolution(NodeItemType::DONT_UPLOAD);
    };

    foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
    {
        if (auto resolvedMoveConflict =
                std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
        {
            if (resolvedMoveConflict->getSolution() == NodeItemType::FILE_UPLOAD_AND_REPLACE)
            {
                auto targetNode(resolvedMoveConflict->getConflictNode());
                auto sourceNode(resolvedMoveConflict->getSourceItemNode());

                // Two or more repeated files in rubbish with no conflict item on the restore tab
                if (targetNode && sourceNode &&
                    targetNode->getParentHandle() == sourceNode->getParentHandle())
                {
                    if (nodesToUpload.contains(targetNode->getHandle()))
                    {
                        linkDuplicatedConflicts(resolvedMoveConflict);
                    }
                    else
                    {
                        nodesToUploadAndReplace.insert(targetNode->getHandle(),
                                                       resolvedMoveConflict);
                    }
                }
                // One or more repeated files in rubbish with a conflict item on the restore tab
                else
                {
                    if (nodesToUploadAndReplace.contains(targetNode->getHandle()))
                    {
                        linkDuplicatedConflicts(resolvedMoveConflict);
                    }
                    else
                    {
                        nodesToUploadAndReplace.insert(targetNode->getHandle(),
                                                       resolvedMoveConflict);
                    }
                }
            }
            // This type of conflict arises when there are two or more repeated files in rubbish
            // with no conflict item on the restore tab
            else if (resolvedMoveConflict->getSolution() == NodeItemType::UPLOAD)
            {
                auto sourceNode(resolvedMoveConflict->getSourceItemNode());
                if (sourceNode)
                {
                    if (auto conflict = nodesToUpload.value(sourceNode->getHandle()))
                    {
                        linkDuplicatedConflicts(conflict);
                    }
                    else
                    {
                        nodesToUpload.insert(sourceNode->getHandle(), resolvedMoveConflict);
                    }
                }
            }
        }
    }
}

void NodeSelectorModel::checkRestoreNodesTargetFolder(std::shared_ptr<ConflictTypes> conflicts)
{
    QSet<mega::MegaHandle> parentTargets;

    for (const auto& resolvedConflict: std::as_const(conflicts->mResolvedConflicts))
    {
        if (resolvedConflict->getSolution() == NodeItemType::DONT_UPLOAD ||
            !resolvedConflict->getParentNode())
        {
            continue;
        }

        parentTargets.insert(resolvedConflict->getParentNode()->getHandle());
    }

    emit itemsAboutToBeRestored(parentTargets);
}

std::optional<NodeSelectorMergeInfo::RestoreMergeType>
    NodeSelectorModel::checkForFoldersToMergeWhenRestoring(std::shared_ptr<ConflictTypes> conflicts)
{
    // There are 3 scenarios when restoring

    // 1) It is a simple folder or file and there is no other item with the same name on the CD ->
    // direct restore -> ONLY "UPLOAD"

    // 2) There are two files/folders with the same name in the rubbish and no other item on the CD
    // -> Merge in the rubbish and then restore -> at least two conflicts -> FOLDER_UPLOAD_AND_MERGE
    // && UPLOAD -> In this case, we end restoring a simple file/folder

    // 3) There are on item on the CD with the same name of one or more items on the rubbish -> We
    // merge the the source folders one by one into the target tab

    // Depending on the scenario, we have 3 types of RestoreMergeType values:

    // 1) No value, we don´t have a merge

    // 2) RestoreMergeType = MERGE_AND_MOVE_TO_TARGET -> We merge folders on rubbish and the we move
    // the final folder to the CD

    // 3) RestoreMergeType = MERGE_ON_EXISTING_TARGET -> We merge folders (one by one)
    //  to the final folder to the CD

    std::optional<NodeSelectorMergeInfo::RestoreMergeType> restoreMergeType;
    QMap<mega::MegaHandle, mega::MegaHandle> handlesToMerge;
    QMap<mega::MegaHandle, std::shared_ptr<DuplicatedMoveNodeInfo>> handlesToUpload;

    auto resetConflict = [](std::shared_ptr<DuplicatedMoveNodeInfo> uploadConflict)
    {
        uploadConflict->setSolution(NodeItemType::DONT_UPLOAD);
    };

    foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
    {
        if (resolvedConflict->getSolution() == NodeItemType::DONT_UPLOAD)
        {
            continue;
        }

        if (auto resolvedMoveConflict =
                std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
        {
            if (resolvedMoveConflict->getSolution() == NodeItemType::FOLDER_UPLOAD_AND_MERGE)
            {
                if (auto uploadConflict =
                        handlesToUpload.value(resolvedMoveConflict->getConflictNode()->getHandle()))
                {
                    restoreMergeType =
                        NodeSelectorMergeInfo::RestoreMergeType::MERGE_AND_MOVE_TO_TARGET;
                    resetConflict(uploadConflict);
                }

                handlesToMerge.insert(resolvedMoveConflict->getConflictNode()->getHandle(),
                                      resolvedMoveConflict->getSourceItemHandle());
            }
            else if (resolvedMoveConflict->getSolution() == NodeItemType::UPLOAD)
            {
                if (handlesToMerge.contains(resolvedMoveConflict->getSourceItemHandle()))
                {
                    restoreMergeType =
                        NodeSelectorMergeInfo::RestoreMergeType::MERGE_AND_MOVE_TO_TARGET;
                    resetConflict(resolvedMoveConflict);
                }
                else
                {
                    handlesToUpload.insert(resolvedMoveConflict->getSourceItemHandle(),
                                           resolvedMoveConflict);
                }
            }
        }
    }

    if (!handlesToMerge.isEmpty() && !restoreMergeType.has_value())
    {
        restoreMergeType = NodeSelectorMergeInfo::RestoreMergeType::MERGE_ON_EXISTING_TARGET;
    }

    return restoreMergeType;
}

void NodeSelectorModel::processMergeQueue(MoveActionType type)
{
    if (mMergeQueue.isEmpty())
    {
        return;
    }

    QList<std::shared_ptr<NodeSelectorMergeInfo>> filteredMerges;
    // Tell the other models that a merge will be performed
    for (auto& merge: mMergeQueue)
    {
        // In this case it is a normal move, not a merge as the merge is done in the source
        if (type == MoveActionType::RESTORE &&
            merge->restoreMergeType ==
                NodeSelectorMergeInfo::RestoreMergeType::MERGE_AND_MOVE_TO_TARGET)
        {
            continue;
        }

        filteredMerges.append(merge);
    }

    if (!filteredMerges.isEmpty())
    {
        emit itemsAboutToBeMerged(filteredMerges, type);
    }

    QThreadPool::globalInstance()->start(
        [this, type]()
        {
            while (!mMergeQueue.isEmpty())
            {
                auto info(mMergeQueue.dequeue());

                std::shared_ptr<MergeMEGAFolders> foldersMerger(std::make_unique<MergeMEGAFolders>(
                    MergeMEGAFolders::ActionForDuplicates::Rename,
                    // Remote is always case sensitive
                    Qt::CaseSensitive,
                    info->type == MoveActionType::COPY ? MergeMEGAFolders::Strategy::Copy :
                                                         MergeMEGAFolders::Strategy::Move));

                auto e = foldersMerger->merge(info->nodeTarget.get(), info->nodeToMerge.get());

                if (e == mega::MegaError::API_OK &&
                    !(type == MoveActionType::RESTORE &&
                      info->restoreMergeType ==
                          NodeSelectorMergeInfo::RestoreMergeType::MERGE_AND_MOVE_TO_TARGET))
                {
                    emit itemMergeFinished(info->nodeToMerge->getHandle(),
                                           info->nodeTarget->getHandle(),
                                           type);
                }

                if (e == mega::MegaError::API_OK && info->type == MoveActionType::RESTORE &&
                    info->restoreMergeType ==
                        NodeSelectorMergeInfo::RestoreMergeType::MERGE_AND_MOVE_TO_TARGET)
                {
                    QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>> nodesToMove;
                    nodesToMove.append(qMakePair(info->nodeTarget->getHandle(), info->parentNode));

                    processNodesAndCheckConflicts(nodesToMove,
                                                  MegaSyncApp->getRubbishNode(),
                                                  info->type);
                }

                if (e != mega::MegaError::API_OK)
                {
                    mFailedMerges.append(info);
                }

                emit finishAsyncRequest(info->nodeTarget->getHandle(), e);
            }

            if (!mFailedMerges.isEmpty())
            {
                emit itemsAboutToBeMergedFailed(mFailedMerges, type);
            }
        });
}

void NodeSelectorModel::processNodesAfterConflictCheck(std::shared_ptr<ConflictTypes> conflicts,
                                                       MoveActionType type)
{
    if (!mOperationTracker.hasRequestGroups())
    {
        mFailedMerges.clear();
    }

    if (conflicts->mResolvedConflicts.isEmpty())
    {
        return;
    }

    mExpectedNodesUpdates.clear();

    std::optional<NodeSelectorMergeInfo::RestoreMergeType> restoreMergeType;

    if (type == MoveActionType::RESTORE)
    {
        checkRestoreNodesTargetFolder(conflicts);
        restoreMergeType = checkForFoldersToMergeWhenRestoring(conflicts);
        checkForDuplicatedSourceFilesWhenRestoring(conflicts);
    }

    QList<mega::MegaHandle> requestHandles;
    QList<std::function<void()>> plannedActions;

    foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
    {
        if (resolvedConflict->getSolution() == NodeItemType::DONT_UPLOAD)
        {
            continue;
        }

        if (auto resolvedMoveConflict =
                std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
        {
            std::shared_ptr<mega::MegaNode> nodeToMove(MegaSyncApp->getMegaApi()->getNodeByHandle(
                resolvedMoveConflict->getSourceItemHandle()));
            if (nodeToMove)
            {
                auto decision = resolvedMoveConflict->getSolution();

                if (decision == NodeItemType::FOLDER_UPLOAD_AND_MERGE)
                {
                    std::shared_ptr<NodeSelectorMergeInfo> info(
                        std::make_shared<NodeSelectorMergeInfo>());
                    info->nodeToMerge = nodeToMove;
                    info->nodeTarget = resolvedMoveConflict->getConflictNode();
                    info->parentNode = resolvedMoveConflict->getParentNode();
                    info->type = type;
                    info->restoreMergeType = restoreMergeType;

                    mMergeQueue.append(info);
                    requestHandles.append(info->nodeTarget->getHandle());
                }
                else
                {
                    mExpectedNodesUpdates.append(nodeToMove->getHandle());
                    requestHandles.append(nodeToMove->getHandle());

                    if (decision == NodeItemType::FILE_UPLOAD_AND_REPLACE)
                    {
                        emit itemAboutToBeReplaced(
                            resolvedMoveConflict->getConflictNode()->getHandle());

                        if (type == MoveActionType::COPY)
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    copyFileAndReplace(nodeToMove,
                                                       resolvedMoveConflict->getConflictNode(),
                                                       resolvedMoveConflict->getParentNode());
                                });
                        }
                        else
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    moveFileAndReplace(nodeToMove,
                                                       resolvedMoveConflict->getConflictNode(),
                                                       resolvedMoveConflict->getParentNode());
                                });
                        }
                    }
                    else if (decision == NodeItemType::UPLOAD_AND_RENAME)
                    {
                        if (type == MoveActionType::COPY)
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    copyNodeAndRename(nodeToMove,
                                                      resolvedMoveConflict->getNewName(),
                                                      resolvedMoveConflict->getParentNode());
                                });
                        }
                        else
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    moveNodeAndRename(nodeToMove,
                                                      resolvedMoveConflict->getNewName(),
                                                      resolvedMoveConflict->getParentNode());
                                });
                        }
                    }
                    else if (decision == NodeItemType::UPLOAD)
                    {
                        if (type == MoveActionType::COPY)
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    copyNode(nodeToMove, resolvedMoveConflict->getParentNode());
                                });
                        }
                        else
                        {
                            plannedActions.append(
                                [this, nodeToMove, resolvedMoveConflict]()
                                {
                                    moveNode(nodeToMove, resolvedMoveConflict->getParentNode());
                                });
                        }
                    }
                }
            }
        }
    }

    mOperationTracker.beginRequestGroup(type, requestHandles);

    for (const auto& action: std::as_const(plannedActions))
    {
        action();
    }

    processMergeQueue(type);

    // We check if the list is empty as merges use other path
    if (!mExpectedNodesUpdates.isEmpty())
    {
        // Set loading view in the source model where the move started
        emit itemsAboutToBeMoved(mExpectedNodesUpdates, type);
    }
}

bool NodeSelectorModel::processNodesAndCheckConflicts(
    const QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>>& handleAndTarget,
    std::shared_ptr<mega::MegaNode> sourceNode,
    MoveActionType type)
{
    if (handleAndTarget.isEmpty())
    {
        return false;
    }

    auto conflicts = CheckDuplicatedNodes::checkMoves(handleAndTarget, sourceNode);

    if (type == MoveActionType::COPY)
    {
        DuplicatedNodeConflictAutoResolution::resolveFolderConflictsForCopy(conflicts);
    }

    if (conflicts->isConflictFree())
    {
        processNodesAfterConflictCheck(conflicts, type);
    }
    else
    {
        if (!handleAndTarget.isEmpty())
        {
            ignoreDuplicatedNodeOptions(handleAndTarget.first().second);
        }

        emit showDuplicatedNodeDialog(conflicts, type);
    }

    return true;
}

QMimeData* NodeSelectorModel::mimeData(const QModelIndexList& indexes) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QSet<mega::MegaHandle> processedHandles;

    for (const QModelIndex& index: indexes)
    {
        if (index.isValid())
        {
            if (NodeSelectorModelItem* chkItem =
                    static_cast<NodeSelectorModelItem*>(index.internalPointer()))
            {
                auto handle(chkItem->getNode()->getHandle());
                if (!processedHandles.contains(handle))
                {
                    processedHandles.insert(handle);
                    stream << static_cast<quint64>(handle);
                }
            }
        }
    }

    mimeData->setData(MIME_DATA_INTERNAL_MOVE, encodedData);
    return mimeData;
}

QMimeData* NodeSelectorModel::mimeData(const QList<mega::MegaHandle>& handles) const
{
    QMimeData* mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QSet<mega::MegaHandle> processedHandles;

    for (const mega::MegaHandle& handle: handles)
    {
        if (!processedHandles.contains(handle))
        {
            processedHandles.insert(handle);
            stream << static_cast<quint64>(handle);
        }
    }

    mimeData->setData(MIME_DATA_INTERNAL_MOVE, encodedData);
    return mimeData;
}

Qt::DropActions NodeSelectorModel::supportedDropActions() const
{
    return Qt::CopyAction;
}

bool NodeSelectorModel::showFiles() const
{
    return mNodeRequesterWorker->showFiles();
}

bool NodeSelectorModel::increaseMovingNodes(int number)
{
    const auto wasIdle = mOperationTracker.beginMoveOperation(number);
    if (wasIdle)
    {
        mIsProcessingMoves = true;
        sendBlockUiSignal(true);
    }

    return wasIdle;
}

void NodeSelectorModel::resetMoveProcessing()
{
    mOperationTracker.clearMoveOperations();
    checkMoveProcessing();
    emit levelsAdded(mIndexesToBeExpanded, true);
}

bool NodeSelectorModel::checkMoveProcessing()
{
    if (mIsProcessingMoves && !isMovingNodes())
    {
        mIsProcessingMoves = false;
        emit itemsMoved();

        sendBlockUiSignal(false);

        return true;
    }

    return false;
}

bool NodeSelectorModel::moveProcessedByNumber(int number)
{
    if (!mOperationTracker.consumeMoveOperations(number))
    {
        return false;
    }

    return checkMoveProcessing();
}

void NodeSelectorModel::finishMovingNodes()
{
    resetMoveProcessing();
}

bool NodeSelectorModel::isMovingNodes() const
{
    return mOperationTracker.hasMoveOperations();
}

bool NodeSelectorModel::pasteNodes(const QList<mega::MegaHandle>& nodesToCopy,
                                   const QModelIndex& targetIndex)
{
    auto data(mimeData(nodesToCopy));
    QModelIndex finalTargetIndex(targetIndex);

    auto item = getItemByIndex(targetIndex);
    if (item)
    {
        auto node = item->getNode();
        if (node && !node->isFolder())
        {
            finalTargetIndex = targetIndex.parent();
        }
    }

    if (startProcessingNodes(data, finalTargetIndex, MoveActionType::COPY))
    {
        return true;
    }

    return false;
}

bool NodeSelectorModel::canPasteNodes(const QList<mega::MegaHandle>& nodesToCopy,
                                      const QModelIndex& indexToPaste)
{
    auto data(mimeData(nodesToCopy));
    return canDropMimeData(data, Qt::CopyAction, -1, -1, indexToPaste);
}

bool NodeSelectorModel::canCopyNodes() const
{
    return true;
}

void NodeSelectorModel::sendBlockUiSignal(bool state)
{
    emit blockUi(state, QPrivateSignal());
}

void NodeSelectorModel::beginRemoveRowsAsync(const mega::MegaHandle& handle)
{
    mRemoveNodesQueue.addStep(handle);
}

void NodeSelectorModel::onStartBeginRemoveRowsAsync(const mega::MegaHandle& handle)
{
    if (handle != mega::INVALID_HANDLE)
    {
        auto index = findIndexByNodeHandle(handle, QModelIndex());
        if (index.isValid())
        {
            // deleteNodeFromModel returns true only when beginRemoveRows/endRemoveRows
            // were actually emitted. If it returns false (e.g. the item is no longer
            // a child of its expected parent), no rowsRemoved signal will fire and the
            // queue would otherwise stall, since RemoveNodesQueue::onRowsRemoved is
            // what advances it.
            if (!deleteNodeFromModel(index))
            {
                mRemoveNodesQueue.skipCurrentStep();
                return;
            }

            // If the loading view is set, decrease the moving number by 1
            if (isMovingNodes())
            {
                moveProcessedByNumber(1);
            }
        }
        else
        {
            mRemoveNodesQueue.skipCurrentStep();
        }
    }
    else
    {
        mRemoveNodesQueue.skipCurrentStep();
    }
}

QModelIndex NodeSelectorModel::index(int row, int column, const QModelIndex& parent) const
{
    QModelIndex index;

    if (hasIndex(row, column, parent))
    {
        if (parent.isValid())
        {
            mNodeRequesterWorker->lockDataMutex(true);
            NodeSelectorModelItem* item(
                static_cast<NodeSelectorModelItem*>(parent.internalPointer()));
            if (item)
            {
                index = createIndex(row, column, item->getChild(row).data());
            }
            mNodeRequesterWorker->lockDataMutex(false);
        }
        else if (mNodeRequesterWorker->rootIndexSize() > row)
        {
            auto rootItem = mNodeRequesterWorker->getRootItem(row);
            index = createIndex(row, column, rootItem);
        }
    }

    return index;
}

QModelIndex NodeSelectorModel::parent(const QModelIndex& index) const
{
    QModelIndex parentIndex;

    if (index.isValid())
    {
        if (isExtraSpaceIndex(index))
        {
            return mCurrentRootIndex;
        }

        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if (item)
        {
            NodeSelectorModelItem* parent = item->getParent();
            if (parent)
            {
                auto indexOfParent = mNodeRequesterWorker->rootIndexOf(parent);
                if (indexOfParent >= 0)
                {
                    parentIndex = createIndex(indexOfParent, 0, parent);
                }
                else
                {
                    parentIndex = createIndex(parent->row(), 0, parent);
                }
            }
        }
    }

    return parentIndex;
}

int NodeSelectorModel::rowCount(const QModelIndex& parent) const
{
    int rows(0);

    if (parent.isValid())
    {
        mNodeRequesterWorker->lockDataMutex(true);
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
        if (item)
        {
            rows = item->areChildrenInitialized() ? item->getNumChildren() : 0;
        }
        if (mExtraSpaceAdded && parent == mCurrentRootIndex)
        {
            rows = rows + 1;
        }
        mNodeRequesterWorker->lockDataMutex(false);
    }
    else
    {
        rows = mNodeRequesterWorker->rootIndexSize();
    }

    return rows;
}

bool NodeSelectorModel::hasChildren(const QModelIndex& parent) const
{
    /////FROM MODEL TESTER:
    // Column 0                | Column 1    |
    // QModelIndex()           |             |
    //    \- topIndex          | topIndex1   |
    //         \- childIndex   | childIndex1 |

    // Common error test #3, the second column should NOT have the same children
    // as the first column in a row.
    // Usually the second column shouldn't have children.

    if (parent.isValid() && parent.column() != NodeSelectorModel::Column::NODE)
    {
        return false;
    }

    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if (item && item->getNode())
    {
        if (item->isTakenDown())
        {
            return false;
        }

        mNodeRequesterWorker->lockDataMutex(true);
        auto numChild = item->getNumChildren() > 0;
        mNodeRequesterWorker->lockDataMutex(false);
        return numChild;
    }
    else
    {
        return QAbstractItemModel::hasChildren(parent);
    }
}

QVariant NodeSelectorModel::headerData(int section, Qt::Orientation orientation, int role) const
{
    if (orientation == Qt::Orientation::Horizontal)
    {
        if (role == Qt::DisplayRole)
        {
            switch (section)
            {
                case NodeSelectorModel::Column::NODE:
                {
                    return tr("Name");
                }
                case NodeSelectorModel::Column::LABEL:
                {
                    return tr("Label");
                }
                case NodeSelectorModel::Column::USER:
                {
                    return tr("Owner");
                }
                case NodeSelectorModel::Column::ACCESS:
                {
                    return tr("Access");
                }
                case NodeSelectorModel::Column::ADDED_DATE:
                {
                    return tr("Date added");
                }
                case NodeSelectorModel::Column::LAST_MODIFIED_DATE:
                {
                    return tr("Last modified");
                }
                case NodeSelectorModel::Column::IS_EXPORTED:
                {
                    return QVariant();
                }
            }
        }
        else if (role == Qt::ToolTipRole)
        {
            switch (section)
            {
                case NodeSelectorModel::Column::LABEL:
                {
                    return tr("Sort by label");
                }
                case NodeSelectorModel::Column::USER:
                {
                    return tr("Sort by owner name");
                }
                case NodeSelectorModel::Column::ACCESS:
                {
                    return tr("Sort by access");
                }
                case NodeSelectorModel::Column::ADDED_DATE:
                {
                    return tr("Sort by date added");
                }
                case NodeSelectorModel::Column::LAST_MODIFIED_DATE:
                {
                    return tr("Sort by last modified date");
                }
                case NodeSelectorModel::Column::NODE:
                {
                    return tr("Sort by name");
                }
            }
        }
        else if (role == toInt(HeaderRoles::ICON_ROLE))
        {
            if (section == NodeSelectorModel::Column::USER)
            {
                return QIcon(QLatin1String("://images/node_selector/icon_small_user.png"));
            }
        }
    }

    return QAbstractItemModel::headerData(section, orientation, role);
}

void NodeSelectorModel::setSyncSetupMode(bool value)
{
    mSyncSetupMode = value;
    mNodeRequesterWorker->setSyncSetupMode(value);
}

bool NodeSelectorModel::addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes,
                                 const QModelIndex& parent)
{
    if (!nodes.isEmpty())
    {
        if (parent.isValid())
        {
            if (isBeingModified())
            {
                mAddNodesQueue.addStep(nodes, parent);
                return true;
            }
            else
            {
                NodeSelectorModelItem* parentItem =
                    static_cast<NodeSelectorModelItem*>(parent.internalPointer());
                if (parentItem && parentItem->getNode()->isFolder() &&
                    parentItem->areChildrenInitialized())
                {
                    setIsModelBeingModified(true);
                    emit requestAddNodes(nodes, parent, parentItem);
                    return true;
                }
            }
        }
    }

    return false;
}

void NodeSelectorModel::onNodesAdded(QList<QPointer<NodeSelectorModelItem>> childrenItem)
{
    endInsertRows();

    if (!childrenItem.isEmpty())
    {
        auto parentIndex(childrenItem.first()->property(INDEX_PROPERTY).toModelIndex().parent());

        // Check if extra space is needed
        if (!mExtraSpaceAdded && mCurrentRootIndex == parentIndex)
        {
            executeAddExtraSpaceLogic(mCurrentRootIndex);
        }

        foreach(auto child, childrenItem)
        {
            auto index = child->property(INDEX_PROPERTY).toModelIndex();
            emit dataChanged(index, index);
        }

        emit modelModified();
        emit nodesAdded(childrenItem);
    }
}

void NodeSelectorModel::onSyncStateChanged(std::shared_ptr<SyncSettings> sync)
{
    if (showsSyncStates() && sync)
    {
        auto syncIndex = findIndexByNodeHandle(sync->getMegaHandle(), QModelIndex());
        auto item = getItemByIndex(syncIndex);
        if (item)
        {
            auto itemStatus = item->getStatus();
            item->calculateSyncStatus();

            if (itemStatus != item->getStatus())
            {
                sendBlockUiSignal(true);
                QThreadPool::globalInstance()->start(
                    [this, item, sync]()
                    {
                        // Update its children
                        if (item->areChildrenInitialized())
                        {
                            for (int index = 0; index < item->getNumChildren(); ++index)
                            {
                                item->getChild(index)->calculateSyncStatus();
                            }
                        }

                        // Update its parent
                        NodeSelectorModelItem* parent(item->getParent());
                        while (parent)
                        {
                            parent->calculateSyncStatus();
                            parent = parent->getParent();
                        }

                        sendBlockUiSignal(false);
                    });
            }
        }
    }
}

void NodeSelectorModel::onRootItemAdded()
{
    endInsertRows();
    emit modelModified();
}

void NodeSelectorModel::beginRootItemsInsertion(int first, int last)
{
    beginInsertRows(QModelIndex(), first, last);
}

void NodeSelectorModel::beginChildRowsInsertion(const QModelIndex& parent, int first, int last)
{
    beginInsertRows(parent, first, last);
}

bool NodeSelectorModel::addToLoadingList(const std::shared_ptr<mega::MegaNode> node)
{
    return node != nullptr;
}

std::shared_ptr<mega::MegaNode> NodeSelectorModel::getNodeToRemove(mega::MegaHandle handle)
{
    auto node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    int access = MegaSyncApp->getMegaApi()->getAccess(node.get());

    // This is for an extra protection as we don´t show the delete action if one of this
    // conditions are not met
    if (!node || access < mega::MegaShare::ACCESS_FULL || !node->isNodeKeyDecrypted())
    {
        return nullptr;
    }

    return node;
}

void NodeSelectorModel::deleteNodes(const QList<mega::MegaHandle>& nodeHandles, bool permanently)
{
    MoveActionType type(permanently ? MoveActionType::DELETE_PERMANENTLY :
                                      MoveActionType::DELETE_RUBBISH);
    emit itemsAboutToBeMoved(nodeHandles, type);

    // It will be unblocked when all requestFinish calls are received (check onRequestFinish)
    QThreadPool::globalInstance()->start(
        [this, nodeHandles, type]()
        {
            QList<std::shared_ptr<mega::MegaNode>> nodesToDelete;
            QList<mega::MegaHandle> requestHandles;

            foreach(auto handle, nodeHandles)
            {
                std::shared_ptr<mega::MegaNode> node(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
                if (node)
                {
                    nodesToDelete.append(node);
                    requestHandles.append(handle);
                }
            }

            mOperationTracker.beginRequestGroup(type, requestHandles);

            for (const auto& node: std::as_const(nodesToDelete))
            {
                // Double protection in case the node properties changed while the node is
                // deleted
                if (type == MoveActionType::DELETE_PERMANENTLY)
                {
                    MegaSyncApp->getMegaApi()->remove(node.get(), mListener.get());
                }
                else
                {
                    auto rubbish = MegaSyncApp->getRubbishNode();
                    moveNode(node, rubbish);
                }
            }
        });
}

bool NodeSelectorModel::areAllNodesEligibleForDeletion(const QList<mega::MegaHandle>& handles)
{
    foreach(auto&& handle, handles)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
        if (!node || !node->isNodeKeyDecrypted() ||
            getNodeAccess(node.get()) < mega::MegaShare::ACCESS_FULL)
        {
            return false;
        }
    }

    // Return false if there are no handles (disabled rows...)
    return !handles.isEmpty();
}

bool NodeSelectorModel::areAllNodesEligibleForRestore(const QList<mega::MegaHandle>& handles) const
{
    auto restorableItems(handles.size());

    for (const auto& nodeHandle: handles)
    {
        std::unique_ptr<mega::MegaNode> node(
            MegaSyncApp->getMegaApi()->getNodeByHandle(nodeHandle));
        if (node && MegaSyncApp->getMegaApi()->isInRubbish(node.get()))
        {
            std::unique_ptr<mega::MegaNode> parentNode(
                MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
            auto previousParentNode = std::shared_ptr<mega::MegaNode>(
                MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));

            if (previousParentNode &&
                !MegaSyncApp->getMegaApi()->isInRubbish(previousParentNode.get()))
            {
                restorableItems--;
            }
        }
    }

    return restorableItems == 0;
}

bool NodeSelectorModel::deleteNodeFromModel(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return false;
    }
    auto item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
    if (!item)
    {
        return false;
    }

    std::shared_ptr<mega::MegaNode> node = item->getNode();
    if (!node)
    {
        return false;
    }

    if (mExtraSpaceAdded && mCurrentRootIndex == index.parent())
    {
        auto currentRowCount(rowCount(index.parent()));
        // 2 is the result of 1 for the extra row + 1 for the row
        // about to be removed
        if (currentRowCount == 2)
        {
            executeRemoveExtraSpaceLogic(mCurrentRootIndex);
        }
    }

    NodeSelectorModelItem* parent =
        static_cast<NodeSelectorModelItem*>(index.parent().internalPointer());
    if (parent)
    {
        int row = parent->indexOf(item);
        if (row < 0)
        {
            return false;
        }
        beginRemoveRows(index.parent(), row, row);
        mNodeRequesterWorker->lockDataMutex(true);
        auto itemToRemove = parent->findChildNode(node);
        mNodeRequesterWorker->lockDataMutex(false);
        emit removeItem(itemToRemove);
        endRemoveRows();
    }
    else
    {
        int row = index.row();
        beginRemoveRows(index.parent(), row, row);
        emit removeRootItem(item);
        endRemoveRows();
    }

    emit modelModified();
    return true;
}

int NodeSelectorModel::getNodeAccess(mega::MegaNode* node)
{
    auto parent = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getParentNode(node));
    if (parent && node)
    {
        auto access(MegaSyncApp->getMegaApi()->getAccess(node));

        if (access >= mega::MegaShare::ACCESS_FULL && (!node->isNodeKeyDecrypted()))
        {
            return mega::MegaShare::ACCESS_UNKNOWN;
        }

        return access;
    }
    else
    {
        return mega::MegaShare::ACCESS_UNKNOWN;
    }
}

void NodeSelectorModel::moveFileAndReplace(std::shared_ptr<mega::MegaNode> moveFile,
                                           std::shared_ptr<mega::MegaNode> conflictTargetFile,
                                           std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    QThreadPool::globalInstance()->start(
        [this, moveFile, targetParentFolder, conflictTargetFile]()
        {
            auto e = Utilities::removeRemoteFile(conflictTargetFile.get());
            if (!e || e->getErrorCode() == mega::MegaError::API_OK)
            {
                MegaSyncApp->getMegaApi()->moveNode(moveFile.get(),
                                                    targetParentFolder.get(),
                                                    mListener.get());
            }
            else
            {
                emit finishAsyncRequest(moveFile->getHandle(), e->getErrorCode());
            }
        });
}

void NodeSelectorModel::copyFileAndReplace(std::shared_ptr<mega::MegaNode> copyItem,
                                           std::shared_ptr<mega::MegaNode> conflictTargetFile,
                                           std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    QThreadPool::globalInstance()->start(
        [this, copyItem, targetParentFolder, conflictTargetFile]()
        {
            auto e = Utilities::removeRemoteFile(conflictTargetFile.get());
            if (!e || e->getErrorCode() == mega::MegaError::API_OK)
            {
                MegaSyncApp->getMegaApi()->copyNode(copyItem.get(),
                                                    targetParentFolder.get(),
                                                    mListener.get());
            }
            else
            {
                emit finishAsyncRequest(copyItem->getHandle(), e->getErrorCode());
            }
        });
}

void NodeSelectorModel::moveNodeAndRename(std::shared_ptr<mega::MegaNode> moveNode,
                                          const QString& newName,
                                          std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    MegaSyncApp->getMegaApi()->moveNode(moveNode.get(),
                                        targetParentFolder.get(),
                                        newName.toUtf8(),
                                        mListener.get());
}

void NodeSelectorModel::copyNodeAndRename(std::shared_ptr<mega::MegaNode> copyNode,
                                          const QString& newName,
                                          std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    MegaSyncApp->getMegaApi()->copyNode(copyNode.get(),
                                        targetParentFolder.get(),
                                        newName.toUtf8(),
                                        mListener.get());
}

void NodeSelectorModel::moveNode(std::shared_ptr<mega::MegaNode> moveNode,
                                 std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    MegaSyncApp->getMegaApi()->moveNode(moveNode.get(), targetParentFolder.get(), mListener.get());
}

void NodeSelectorModel::copyNode(std::shared_ptr<mega::MegaNode> copyNode,
                                 std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    MegaSyncApp->getMegaApi()->copyNode(copyNode.get(), targetParentFolder.get(), mListener.get());
}

void NodeSelectorModel::onRequestFinish(mega::MegaRequest* request, mega::MegaError* e)
{
    auto type(request->getType());

    if (type == mega::MegaRequest::TYPE_MOVE || type == mega::MegaRequest::TYPE_REMOVE ||
        type == mega::MegaRequest::TYPE_COPY || type == mega::MegaRequest::TYPE_RESTORE)
    {
        auto handle(request->getNodeHandle());
        checkFinishedRequest(handle, e->getErrorCode());
    }
}

MessageDialogInfo NodeSelectorModel::buildFailedRequestMessage(
    int requestType,
    const QList<mega::MegaHandle>& failedHandles,
    NodeSelectorOperationTracker::FinishedRequestGroup finishedRequestGroup) const
{
    MessageDialogInfo msgInfo;
    msgInfo.buttonsText.insert(QMessageBox::StandardButton::Ok, tr("Close"));

    MovedItemsTypes movedItemsType = MovedItemsType::NONE;
    if (finishedRequestGroup.movedItemCategories & NodeSelectorOperationTracker::FILES)
    {
        movedItemsType |= MovedItemsType::FILES;
    }
    if (finishedRequestGroup.movedItemCategories & NodeSelectorOperationTracker::FOLDERS)
    {
        movedItemsType |= MovedItemsType::FOLDERS;
    }

    const auto multipleRequest = failedHandles.size() > 1;

    auto failedNode = failedHandles.isEmpty() ?
                          std::unique_ptr<mega::MegaNode>() :
                          std::unique_ptr<mega::MegaNode>(
                              MegaSyncApp->getMegaApi()->getNodeByHandle(failedHandles.first()));

    if (requestType == MoveActionType::MOVE)
    {
        if (multipleRequest || !failedNode)
        {
            if (movedItemsType.testFlag(MovedItemsType::NONE) ||
                movedItemsType.testFlag(MovedItemsType::BOTH))
            {
                msgInfo.titleText = tr("Error moving items");
                msgInfo.descriptionText = tr("The items couldn’t be moved. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FILES))
            {
                msgInfo.titleText = tr("Error moving files");
                msgInfo.descriptionText = tr("The files couldn’t be moved. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FOLDERS))
            {
                msgInfo.titleText = tr("Error moving folders");
                msgInfo.descriptionText = tr("The folders couldn’t be moved. Try again later");
            }
        }
        else if (failedNode->isFile())
        {
            msgInfo.titleText = tr("Error moving file");
            msgInfo.descriptionText = tr("The file %1 couldn’t be moved. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
        else
        {
            msgInfo.titleText = tr("Error moving folder");
            msgInfo.descriptionText = tr("The folder %1 couldn’t be moved. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
    }
    else if (requestType == MoveActionType::COPY)
    {
        if (multipleRequest || !failedNode)
        {
            if (movedItemsType.testFlag(MovedItemsType::NONE) ||
                movedItemsType.testFlag(MovedItemsType::BOTH))
            {
                msgInfo.titleText = tr("Error copying items");
                msgInfo.descriptionText = tr("The items couldn’t be copied. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FILES))
            {
                msgInfo.titleText = tr("Error copying files");
                msgInfo.descriptionText = tr("The files couldn’t be copied. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FOLDERS))
            {
                msgInfo.titleText = tr("Error copying folders");
                msgInfo.descriptionText = tr("The folders couldn’t be copied. Try again later");
            }
        }
        else if (failedNode->isFile())
        {
            msgInfo.titleText = tr("Error copying file");
            msgInfo.descriptionText = tr("The file %1 couldn’t be copied. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
        else
        {
            msgInfo.titleText = tr("Error copying folder");
            msgInfo.descriptionText = tr("The folder %1 couldn’t be copied. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
    }
    else if (requestType == MoveActionType::RESTORE)
    {
        if (multipleRequest || !failedNode)
        {
            if (movedItemsType.testFlag(MovedItemsType::NONE) ||
                movedItemsType.testFlag(MovedItemsType::BOTH))
            {
                msgInfo.titleText = tr("Error restoring items");
                msgInfo.descriptionText = tr("The items couldn’t be restored. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FILES))
            {
                msgInfo.titleText = tr("Error restoring files");
                msgInfo.descriptionText = tr("The files couldn’t be restored. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FOLDERS))
            {
                msgInfo.titleText = tr("Error restoring folders");
                msgInfo.descriptionText = tr("The folders couldn’t be restored. Try again later");
            }
        }
        else if (failedNode->isFile())
        {
            msgInfo.titleText = tr("Error restoring file");
            msgInfo.descriptionText = tr("The file %1 couldn’t be restored. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
        else
        {
            msgInfo.titleText = tr("Error restoring folder");
            msgInfo.descriptionText = tr("The folder %1 couldn’t be restored. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
    }
    else if (requestType >= MoveActionType::DELETE_RUBBISH)
    {
        if (multipleRequest || !failedNode)
        {
            if (movedItemsType.testFlag(MovedItemsType::NONE) ||
                movedItemsType.testFlag(MovedItemsType::BOTH))
            {
                msgInfo.titleText = tr("Error deleting items");
                msgInfo.descriptionText = tr("The items couldn’t be deleted. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FILES))
            {
                msgInfo.titleText = tr("Error deleting files");
                msgInfo.descriptionText = tr("The files couldn’t be deleted. Try again later");
            }
            else if (movedItemsType.testFlag(MovedItemsType::FOLDERS))
            {
                msgInfo.titleText = tr("Error deleting folders");
                msgInfo.descriptionText = tr("The folders couldn’t be deleted. Try again later");
            }
        }
        else if (failedNode->isFile())
        {
            msgInfo.titleText = tr("Error deleting file");
            msgInfo.descriptionText = tr("The file %1 couldn’t be deleted. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
        else
        {
            msgInfo.titleText = tr("Error deleting folder");
            msgInfo.descriptionText = tr("The folder %1 couldn’t be deleted. Try again later")
                                          .arg(MegaNodeNames::getNodeName(failedNode.get()));
        }
    }

    return msgInfo;
}

void NodeSelectorModel::checkFinishedRequest(mega::MegaHandle handle, int errorCode)
{
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    const auto finishedRequestGroup = mOperationTracker.finishRequest(
        handle,
        errorCode != mega::MegaError::API_OK || handle == mega::INVALID_HANDLE,
        node ? (node->isFile() ? NodeSelectorOperationTracker::FILES :
                                 NodeSelectorOperationTracker::FOLDERS) :
               NodeSelectorOperationTracker::NONE);

    if (!finishedRequestGroup.matched || !finishedRequestGroup.groupFinished)
    {
        return;
    }

    if (finishedRequestGroup.failedHandles.isEmpty())
    {
        emit itemRequestsFinished(finishedRequestGroup.type);
    }

    if (!finishedRequestGroup.failedHandles.isEmpty())
    {
        emit showMessageBox(buildFailedRequestMessage(finishedRequestGroup.type,
                                                      finishedRequestGroup.failedHandles,
                                                      finishedRequestGroup));

        auto failedHandles = finishedRequestGroup.failedHandles;
        if (failedHandles.size() != mFailedMerges.size())
        {
            if (!mFailedMerges.isEmpty())
            {
                for (const auto& mergeInfo: std::as_const(mFailedMerges))
                {
                    failedHandles.removeAll(mergeInfo->nodeTarget->getHandle());
                }
            }

            if (!failedHandles.isEmpty())
            {
                emit itemsAboutToBeMovedFailed(failedHandles, finishedRequestGroup.type);
            }
        }
    }
}

void NodeSelectorModel::showFiles(bool show)
{
    mNodeRequesterWorker->setShowFiles(show);
}

void NodeSelectorModel::showReadOnlyFolders(bool show)
{
    mNodeRequesterWorker->setShowReadOnlyFolders(show);
}

QVariant NodeSelectorModel::getIcon(const QModelIndex& index, NodeSelectorModelItem* item) const
{
    auto isDisabled(!(index.flags() & Qt::ItemIsEnabled));
    auto disabledToken = QLatin1String("icon-disabled");

    switch (index.column())
    {
        case NodeSelectorModel::Column::NODE:
        {
            auto iconSize(data(index, toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE)).toSize());
            auto info = getFolderIcon(item);
            auto pixmap = info.first.pixmap(iconSize);

            if (!info.second.isEmpty() || isDisabled)
            {
                pixmap =
                    IconTokenizer::changePixmapColor(pixmap,
                                                     TokenParserWidgetManager::instance()->getColor(
                                                         isDisabled ? disabledToken : info.second))
                        .value_or(QPixmap());
            }
            else if (item->getNode() && !item->getNode()->isFile())
            {
                NodeSelectorLabelColors::LabelGradient labelGradient;
                if (!isDisabled && item->getNode())
                {
                    labelGradient =
                        NodeSelectorLabelColors::gradientForLabel(item->getNode()->getLabel());
                }

                if (labelGradient.from.isValid())
                {
                    pixmap = IconTokenizer::changePixmapToGradient(pixmap,
                                                                   labelGradient.from,
                                                                   labelGradient.to)
                                 .value_or(QPixmap());
                }
            }

            return QVariant::fromValue<QPixmap>(pixmap);
        }
        case NodeSelectorModel::Column::ADDED_DATE:
        case NodeSelectorModel::Column::LAST_MODIFIED_DATE:
        {
            break;
        }
        case NodeSelectorModel::Column::USER:
        {
            // Keep the icon consistent with getUserText(): only inshare roots show the owner.
            if (showAccess(item->getNode().get()))
            {
                return QVariant::fromValue<QPixmap>(item->getOwnerIcon());
            }
            break;
        }
        case NodeSelectorModel::Column::ACCESS:
        {
            if (showAccess(item->getNode().get()))
            {
                auto icon = Utilities::getNodeAccessIcon(item->getNodeAccess());
                return QVariant::fromValue<QPixmap>(
                    IconTokenizer::changePixmapColor(
                        icon.pixmap(
                            data(index, toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE)).toSize()),
                        TokenParserWidgetManager::instance()->getColor(
                            isDisabled ? disabledToken : QLatin1String("icon-primary")))
                        .value_or(QPixmap()));
            }
            break;
        }
        case NodeSelectorModel::Column::IS_EXPORTED:
        {
            if (item->getNode() && item->getNode()->isExported())
            {
                auto iconSize(data(index, toInt(NodeSelectorModelRoles::ICON_SIZE_ROLE)).toSize());
                auto pixmap = Utilities::getPixmap(QLatin1String("link_01"),
                                                   Utilities::AttributeType::SMALL |
                                                       Utilities::AttributeType::THIN |
                                                       Utilities::AttributeType::OUTLINE,
                                                   iconSize);
                auto tokenizedPixmap =
                    IconTokenizer::changePixmapColor(pixmap,
                                                     TokenParserWidgetManager::instance()->getColor(
                                                         QLatin1String("icon-secondary")));
                return QVariant::fromValue<QPixmap>(tokenizedPixmap.value_or(pixmap));
            }
            break;
        }
        default:
        {
            break;
        }
    }
    return QVariant();
}

QVariant NodeSelectorModel::getText(const QModelIndex& index, NodeSelectorModelItem* item) const
{
    switch (index.column())
    {
        case Column::NODE:
        {
            return getDisplayText(item);
        }
        case Column::LABEL:
        {
            return getLabelText(item);
        }
        case Column::ADDED_DATE:
        {
            return getAddedDateText(item);
        }
        case Column::LAST_MODIFIED_DATE:
        {
            return getLastModifiedDateText(item);
        }
        case Column::ACCESS:
        {
            return getAccessText(item);
        }
        case Column::USER:
        {
            return getUserText(item);
        }
        case Column::IS_EXPORTED:
        {
            return {};
        }
        default:
        {
            break;
        }
    }
    return {};
}

QVariant NodeSelectorModel::getDisplayText(NodeSelectorModelItem* item) const
{
    return MegaNodeNames::getNodeName(item->getNode().get());
}

QVariant NodeSelectorModel::getLabelText(NodeSelectorModelItem* item) const
{
    return item && item->getNode() ?
               NodeSelectorLabelColors::nameForLabel(item->getNode()->getLabel()) :
               QVariant();
}

QVariant NodeSelectorModel::getAddedDateText(NodeSelectorModelItem* item) const
{
    return MegaSyncApp->getFormattedDateByCurrentLanguage(
        QDateTime::fromSecsSinceEpoch(item->getNode()->getCreationTime()),
        QLocale::FormatType::ShortFormat);
}

QVariant NodeSelectorModel::getLastModifiedDateText(NodeSelectorModelItem* item) const
{
    return item->getNode()->isFolder() ?
               QVariant() :
               MegaSyncApp->getFormattedDateByCurrentLanguage(
                   QDateTime::fromSecsSinceEpoch(item->getNode()->getModificationTime()),
                   QLocale::FormatType::ShortFormat);
}

QVariant NodeSelectorModel::getAccessText(NodeSelectorModelItem* item) const
{
    return showAccess(item->getNode().get()) ?
               Utilities::getNodeStringAccess(item->getNodeAccess()) :
               QVariant();
}

QVariant NodeSelectorModel::getUserText(NodeSelectorModelItem* item) const
{
    return showAccess(item->getNode().get()) ? item->getOwnerName() : QVariant();
}

QList<QPair<mega::MegaHandle, QModelIndex>> NodeSelectorModel::needsToBeExpanded()
{
    auto auxList(mIndexesToBeExpanded);
    mIndexesToBeExpanded.clear();
    return auxList;
}

QList<QPair<mega::MegaHandle, QModelIndex>> NodeSelectorModel::needsToBeSelected()
{
    auto auxList(mIndexesToBeSelected);
    mIndexesToBeSelected.clear();
    return auxList;
}

void NodeSelectorModel::abort()
{
    // The cancelled request never delivers nodesReady, so a tree path chain in progress would
    // stay in mNodesToLoad forever and keep isLoadingTreePath() stuck at true.
    mNodesToLoad.clear();
    mNodeRequesterWorker->cancelCurrentRequest();
}

bool NodeSelectorModel::canBeDeleted() const
{
    return true;
}

bool NodeSelectorModel::isLoadingTreePath() const
{
    return !mNodesToLoad.isEmpty();
}

void NodeSelectorModel::loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node)
{
    // First, we set the loading view as it can take long to load the tree path to the node
    if (!isMovingNodes())
    {
        sendBlockUiSignal(true);
    }

    mNodesToLoad.clear();
    mNodesToLoad.append(node);

    auto p_node =
        std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getParentNode(node.get()));

    // The vault node is not represented in the node selector, hence if the parent of a node is the
    // vault it doesn´t have to be added to the node list to load. If it is added the loading of a
    // specific node will stops working in backups screen.
    while (addToLoadingList(p_node))
    {
        mIndexesToBeExpanded.append(qMakePair(p_node->getHandle(), QModelIndex()));
        mNodesToLoad.append(p_node);
        p_node.reset(MegaSyncApp->getMegaApi()->getParentNode(p_node.get()));
    }

    if (!fetchMoreRecursively(QModelIndex()))
    {
        sendBlockUiSignal(false);
        mNodesToLoad.clear();
    }
}

bool NodeSelectorModel::fetchMoreRecursively(const QModelIndex& parentIndex)
{
    auto result(false);
    if (!mNodesToLoad.isEmpty())
    {
        auto node = mNodesToLoad.last();
        if (node)
        {
            auto indexToCheck = getIndexFromNode(node, parentIndex);
            if (indexToCheck.isValid())
            {
                if (canFetchMore(indexToCheck))
                {
                    fetchMore(indexToCheck);
                    result = true;
                }
                else
                {
                    result = continueWithNextItemToLoad(indexToCheck);
                }
            }
        }
    }

    return result;
}

// This method looks only in the parent layer, not recursively
QModelIndex NodeSelectorModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> node,
                                                const QModelIndex& parent)
{
    if (node)
    {
        auto childrenCount = rowCount(parent);
        for (int row = 0; row < childrenCount; ++row)
        {
            auto indexToCheck = index(row, 0, parent);
            NodeSelectorModelItem* item =
                static_cast<NodeSelectorModelItem*>(indexToCheck.internalPointer());
            if (item)
            {
                if (item->getNode()->getHandle() == node->getHandle())
                {
                    return indexToCheck;
                }
            }
        }
    }

    return QModelIndex();
}

void NodeSelectorModel::rootItemsLoaded()
{
    endResetModel();
}

void NodeSelectorModel::addRootItems()
{
    sendBlockUiSignal(true);
    beginResetModel();
    createRootNodes();
}

void NodeSelectorModel::loadLevelFinished()
{
    if (mAddExpaceWhenLoadingFinish)
    {
        // Full deferral (the root change happened while the model was mid-change): now do the
        // whole transition. mCurrentRootIndex still holds the previous root here, so the phantom
        // row is removed from it correctly before committing the pending root.
        if (mPendingRootIndex.isValid())
        {
            executeRemoveExtraSpaceLogic(mCurrentRootIndex);
            commitCurrentRootIndex(mPendingRootIndex);
            mPendingRootIndex = QModelIndex();
            emit currentRootIndexChanged();
        }
        // Otherwise the root was already committed and only the phantom row add was deferred
        // until its children loaded.
        executeAddExtraSpaceLogic(mCurrentRootIndex);
        mAddExpaceWhenLoadingFinish = false;
    }

    emit levelsAdded(mIndexesToBeExpanded);
}

bool NodeSelectorModel::canFetchMore(const QModelIndex& parent) const
{
    if (!parent.isValid())
    {
        return false;
    }
    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if (item)
    {
        if (item->isTakenDown())
        {
            return false;
        }

        return item->canFetchMore();
    }
    else
    {
        return mNodeRequesterWorker->rootIndexSize() < rootItemsCount();
    }
}

void NodeSelectorModel::setCurrentRootIndex(const QModelIndex& index)
{
    const auto newRootIndex = index.isValid() ? index : getTopRootIndex();
    if (mCurrentRootIndex == newRootIndex)
    {
        return;
    }

    // While the model is mid-change we cannot mutate rows: keep the old fully-deferred behaviour
    // (remove + commit + add applied together once the level finishes loading). This is not the
    // first-folder-entry case the immediate commit targets, so deferring the commit here is fine.
    if (isBeingModified())
    {
        mPendingRootIndex = newRootIndex;
        mAddExpaceWhenLoadingFinish = true;
        return;
    }

    // Remove the phantom "extra space" row from the previous root WHILE it is still the current
    // root, so rowCount() still accounts for it and the correct (last) row is removed. Only then
    // commit the new root immediately, so breadcrumb/header/columns update even before the new
    // children load.
    executeRemoveExtraSpaceLogic(mCurrentRootIndex);
    commitCurrentRootIndex(newRootIndex);
    emit currentRootIndexChanged();

    // Adding the phantom row needs the new root's children loaded to know where to insert it.
    NodeSelectorModelItem* item =
        static_cast<NodeSelectorModelItem*>(mCurrentRootIndex.internalPointer());
    if (!mCurrentRootIndex.isValid() || (item && item->areChildrenInitialized()))
    {
        executeAddExtraSpaceLogic(mCurrentRootIndex);
        mAddExpaceWhenLoadingFinish = false;
    }
    else
    {
        mAddExpaceWhenLoadingFinish = true;
    }
}

QModelIndex NodeSelectorModel::getCurrentRootIndex() const
{
    return mCurrentRootIndex;
}

QModelIndex NodeSelectorModel::commitCurrentRootIndex(const QModelIndex& index)
{
    mCurrentRootIndex = index.isValid() ? index : getTopRootIndex();
    return mCurrentRootIndex;
}

QModelIndex NodeSelectorModel::rootIndex(const QModelIndex& visualRootIndex) const
{
    if (!visualRootIndex.isValid())
    {
        return getTopRootIndex();
    }

    return visualRootIndex;
}

QModelIndex NodeSelectorModel::getTopRootIndex() const
{
    return index(0, 0);
}

bool NodeSelectorModel::isRequestingNodes() const
{
    return mNodeRequesterWorker->isRequestingNodes();
}

bool NodeSelectorModel::fetchItemChildren(const QModelIndex& parent)
{
    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if (!item || item->areChildrenInitialized())
    {
        return false;
    }

    // A previous request is already in flight; its nodesReady will balance the caller's counter.
    if (item->requestingChildren())
    {
        return true;
    }

    // Just in case the children changed
    item->resetChildrenCounter();
    const auto itemNumChildren = item->getNumChildren();
    if (itemNumChildren > 0)
    {
        // fetchItemChildren runs synchronously from QTreeView's layout() (Qt calls fetchMore()
        // while expanding a row). sendBlockUiSignal(true) shows the loading scene, which detaches
        // the view's model (setModel(nullptr)) and clears the view's internal viewItems mid-layout;
        // layout() then indexes the now-empty vector (QVector out of range -> assert in debug,
        // silent out-of-bounds read in release). Defer the UI block so it runs after layout()
        // returns. The child fetch itself is already async (requestChildNodes is a queued
        // connection), so no data is lost by deferring the block. The sort path blocks the UI
        // synchronously through its own sendBlockUiSignal call, so that detach still precedes the
        // background sort.
        QMetaObject::invokeMethod(
            this,
            [this]()
            {
                sendBlockUiSignal(true);
            },
            Qt::QueuedConnection);
        emit requestChildNodes(item, parent);

        return true;
    }

    return false;
}

void NodeSelectorModel::onChildNodesReady(NodeSelectorModelItem* parent, int insertedCount)
{
    if (insertedCount > 0)
    {
        endInsertRows();
    }

    auto index = parent->property(INDEX_PROPERTY).value<QModelIndex>();
    continueWithNextItemToLoad(index);
}

bool NodeSelectorModel::continueWithNextItemToLoad(const QModelIndex& parentIndex)
{
    bool result = false;

    if (!mNodesToLoad.isEmpty())
    {
        // The last one has been already processed
        auto lastNode = mNodesToLoad.takeLast();
        if (!mNodesToLoad.isEmpty())
        {
            result = fetchMoreRecursively(parentIndex);
            if (result)
            {
                mIndexesToBeExpanded.append(qMakePair(lastNode->getHandle(), parentIndex));
            }
            else if (!mNodesToLoad.isEmpty())
            {
                // The last node is empty
                mNodesToLoad.removeLast();
            }
        }
    }

    if (mNodesToLoad.isEmpty())
    {
        loadLevelFinished();
    }

    return result;
}

bool NodeSelectorModel::showAccess(mega::MegaNode* node) const
{
    return node->isInShare();
}

QModelIndex NodeSelectorModel::findIndexByNodeHandle(const mega::MegaHandle& handle,
                                                     const QModelIndex& parent)
{
    for (int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex idx = index(i, NodeSelectorModel::Column::NODE, parent);
        if (idx.isValid())
        {
            if (NodeSelectorModelItem* chkItem =
                    static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
            {
                if (chkItem->getNode()->getHandle() == handle)
                {
                    return idx;
                }
            }
        }
    }
    for (int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex child = parent.isValid() ? index(i, NodeSelectorModel::Column::NODE, parent) :
                                               index(i, NodeSelectorModel::Column::NODE);
        if (child.isValid())
        {
            auto ret = findIndexByNodeHandle(handle, child);
            if (ret.isValid())
            {
                return ret;
            }
        }
    }

    return QModelIndex();
}

NodeSelectorModelItem* NodeSelectorModel::getItemByIndex(const QModelIndex& index)
{
    return qvariant_cast<NodeSelectorModelItem*>(
        index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
}

void NodeSelectorModel::updateItemNode(const QModelIndex& indexToUpdate,
                                       std::shared_ptr<mega::MegaNode> node)
{
    auto item = getItemByIndex(indexToUpdate);
    if (item)
    {
        // updateNode() may walk the item's child subtree (propagateNodeAccessToChildren on a
        // share-permission change). That traversal reads mChildItems, which the NodeRequester
        // worker structurally mutates (createChildItems/initializeChildItems/appendNodes) under
        // this same data mutex. Serialize against the worker so the GUI-thread walk cannot race
        // an in-flight child fetch of the affected subtree.
        mNodeRequesterWorker->lockDataMutex(true);
        item->updateNode(node);
        mNodeRequesterWorker->lockDataMutex(false);
        updateRow(indexToUpdate);
    }
}

void NodeSelectorModel::updateRow(const QModelIndex& indexToUpdate)
{
    auto firstColumnIndex = index(indexToUpdate.row(), 0, indexToUpdate.parent());
    auto lastColumnIndex = index(indexToUpdate.row(), columnCount() - 1, indexToUpdate.parent());
    emit dataChanged(firstColumnIndex, lastColumnIndex);
}

QPair<QIcon, QString> NodeSelectorModel::getFolderIcon(NodeSelectorModelItem* item) const
{
    QIcon icon;
    QString token;

    if (item)
    {
        if (item->isTakenDown())
        {
            return qMakePair(QIcon(QStringLiteral(":/taken_down_medium")), QString());
        }

        auto node = item->getNode();

        if (node)
        {
            if (node->getType() >= mega::MegaNode::TYPE_FOLDER)
            {
                if (node->getHandle() == mCameraFolderAttribute->getCameraUploadFolderHandle() ||
                    node->getHandle() ==
                        mCameraFolderAttribute->getCameraUploadFolderSecondaryHandle())
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_CAMERA_UPLOADS,
                                                      Utilities::AttributeType::MEDIUM);
                }
                else if (node->getHandle() ==
                         mMyChatFilesFolderAttribute->getMyChatFilesFolderHandle())
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_CHAT,
                                                      Utilities::AttributeType::MEDIUM);
                }
                else if (node->isInShare())
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_USERS,
                                                      Utilities::AttributeType::MEDIUM);
                }
                else if (node->isOutShare())
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_OUTGOING_SHARE,
                                                      Utilities::AttributeType::MEDIUM);
                }
                else if (item->getStatus() == NodeSelectorModelItem::Status::SYNC)
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_SYNC,
                                                      Utilities::AttributeType::MEDIUM);
                }
                else
                {
                    auto searchItem = dynamic_cast<NodeSelectorModelItemSearch*>(item);

                    if ((searchItem && searchItem->getType() & TabType::BACKUP) ||
                        item->getStatus() == NodeSelectorModelItem::Status::BACKUP)
                    {
                        if (item->isDeviceFolder())
                        {
                            QString nodeDeviceId(QString::fromUtf8(node->getDeviceId()));
                            if (!nodeDeviceId.isEmpty())
                            {
                                // TODO, future: choose icon according to host OS
                                if (nodeDeviceId ==
                                    QString::fromUtf8(MegaSyncApp->getMegaApi()->getDeviceId()))
                                {
#ifdef Q_OS_WINDOWS
                                    icon = Utilities::getIcon(QLatin1String("pc-windows-dark"),
                                                              Utilities::AttributeType::MEDIUM |
                                                                  Utilities::AttributeType::SOLID);
#elif defined(Q_OS_MACOS)
                                    icon = Utilities::getIcon(QLatin1String("pc-mac-dark"),
                                                              Utilities::AttributeType::MEDIUM |
                                                                  Utilities::AttributeType::SOLID);
#elif defined(Q_OS_LINUX)
                                    icon = Utilities::getIcon(QLatin1String("pc-linux-dark"),
                                                              Utilities::AttributeType::MEDIUM |
                                                                  Utilities::AttributeType::SOLID);
#endif
                                }
                                else
                                {
                                    icon = Utilities::getIcon(QLatin1String("pc-dark"),
                                                              Utilities::AttributeType::MEDIUM |
                                                                  Utilities::AttributeType::SOLID);
                                }
                                token = QLatin1String("background-inverse");
                            }
                        }
                        else if (item->isBackupFolder())
                        {
                            icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_BACKUP_2,
                                                              Utilities::AttributeType::MEDIUM);
                        }
                    }
                }

                if (icon.isNull())
                {
                    icon = Utilities::getFolderPixmap(Utilities::FolderType::TYPE_NORMAL,
                                                      Utilities::AttributeType::SMALL);
                }
            }
            else
            {
                icon = Utilities::getExtensionPixmap(QString::fromUtf8(node->getName()),
                                                     Utilities::AttributeType::SMALL);
            }
        }
    }

    return qMakePair(icon, token);
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Add nodes queue (To avoid calling beginInsertRows more than once at the same time)
AddNodesQueue::AddNodesQueue(NodeSelectorModel* model):
    mModel(model)
{
    connect(mModel,
            &NodeSelectorModel::modelIsBeingModifiedChanged,
            this,
            &AddNodesQueue::onNodesAdded);
}

void AddNodesQueue::addStep(const QList<std::shared_ptr<mega::MegaNode>>& nodes,
                            const QModelIndex& parentIndex)
{
    Info info;
    info.nodesToAdd = nodes;
    info.parentIndex = parentIndex;
    mSteps.append(info);
}

void AddNodesQueue::onNodesAdded(bool state)
{
    if (!state && !mSteps.isEmpty())
    {
        auto info(mSteps.dequeue());
        mModel->addNodes(info.nodesToAdd, info.parentIndex);
    }
}

////////////////////////////////////////////////////////////////////////////////////////////
/// Remove nodes queue (To avoid calling beginRemoveRows more than once at the same time)
RemoveNodesQueue::RemoveNodesQueue(NodeSelectorModel* model):
    mModel(model)
{
    connect(mModel, &NodeSelectorModel::rowsRemoved, this, &RemoveNodesQueue::onRowsRemoved);
}

void RemoveNodesQueue::addStep(const mega::MegaHandle& handle)
{
    mSteps.enqueue(handle);

    if (mSteps.size() == 1)
    {
        emit startBeginRemoveRows(handle);
    }
}

void RemoveNodesQueue::skipCurrentStep()
{
    if (!mSteps.isEmpty())
    {
        mSteps.dequeue();

        if (!mSteps.isEmpty())
        {
            emit startBeginRemoveRows(mSteps.head());
        }
    }
}

void RemoveNodesQueue::onRowsRemoved()
{
    if (!mSteps.isEmpty())
    {
        // Remove the previously used
        mSteps.dequeue();

        if (!mSteps.isEmpty())
        {
            auto nextStepHandle(mSteps.head());
            emit startBeginRemoveRows(nextStepHandle);
        }
    }
}
