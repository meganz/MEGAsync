#include "NodeSelectorModel.h"

#include "CameraUploadFolder.h"
#include "MegaApiSynchronizedRequest.h"
#include "MegaApplication.h"
#include "MegaNodeNames.h"
#include "MergeMEGAFolders.h"
#include "MyChatFilesFolder.h"
#include "NodeSelectorModelSpecialised.h"
#include "RequestListenerManager.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

#include <QApplication>
#include <QToolTip>

const char* INDEX_PROPERTY = "INDEX";

NodeRequester::NodeRequester(NodeSelectorModel *model)
    : QObject(nullptr),
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

void NodeRequester::requestNodeAndCreateChildren(NodeSelectorModelItem* item, const QModelIndex& parentIndex)
{
    if(item)
    {
        auto node = item->getNode();
        item->setProperty(INDEX_PROPERTY, parentIndex);
        if(!item->requestingChildren() && !item->areChildrenInitialized())
        {
            item->setRequestingChildren(true);
            mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

            mNodesRequested = true;

            std::unique_ptr<mega::MegaSearchFilter> searchFilter(mega::MegaSearchFilter::createInstance());
            searchFilter->byNodeType(mShowFiles ? mega::MegaNode::TYPE_UNKNOWN : mega::MegaNode::TYPE_FOLDER);
            searchFilter->byLocationHandle(node->getHandle());

            std::unique_ptr<mega::MegaNodeList> childNodesFiltered(megaApi->getChildren(searchFilter.get(),
                                                                                        mega::MegaApi::ORDER_NONE,
                                                                                        mCancelToken.get()));
            mNodesRequested = false;
            if(!isAborted())
            {
                connect(item, &NodeSelectorModelItem::updateLoadingMessage, this, &NodeRequester::updateLoadingMessage);
                lockDataMutex(true);
                item->createChildItems(std::move(childNodesFiltered));
                lockDataMutex(false);
                disconnect(item, &NodeSelectorModelItem::updateLoadingMessage, this, &NodeRequester::updateLoadingMessage);
                emit nodesReady(item);
            }
        }
    }
}

void NodeRequester::search(const QString &text, NodeSelectorModelItemSearch::Types typesAllowed)
{
    if(text.isEmpty())
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

    auto nodeList = std::unique_ptr<mega::MegaNodeList>(MegaSyncApp->getMegaApi()->search(searchFilter.get(), mega::MegaApi::ORDER_NONE, mCancelToken.get()));
    QList<NodeSelectorModelItem*> items;
    mSearchedTypes = NodeSelectorModelItemSearch::Type::NONE;

    for(int i = 0; i < nodeList->size(); i++)
    {
        auto item = createSearchItem(nodeList->get(i), typesAllowed);
        if(item)
        {
            items.append(item);
        }
    }

    if(isAborted() || mSearchCanceled)
    {
        qDeleteAll(items);
    }
    else
    {
        QMutexLocker d(&mDataMutex);
        mRootItems.append(items);
        emit searchItemsCreated();
    }
}

void NodeRequester::addSearchRootItem(QList<std::shared_ptr<mega::MegaNode>> nodes, NodeSelectorModelItemSearch::Types typesAllowed)
{
    QList<NodeSelectorModelItem*> items;
    foreach(auto node, nodes)
    {
        auto item = createSearchItem(node.get(), typesAllowed);
        if(item)
        {
            items.append(item);
        }
    }

    if(isAborted())
    {
        qDeleteAll(items);
    }
    else
    {
        if(!items.isEmpty())
        {
            QMutexLocker d(&mDataMutex);
            mRootItems.append(items);
            emit rootItemsAdded();
        }
    }
}

NodeSelectorModelItem *NodeRequester::createSearchItem(mega::MegaNode *node, NodeSelectorModelItemSearch::Types typesAllowed)
{
    if(isAborted() || mSearchCanceled)
    {
        return nullptr;
    }
    if ((node->isFile() && !mShowFiles))
    {
        return nullptr;
    }
    else if(mSyncSetupMode)
    {
        int access = MegaSyncApp->getMegaApi()->getAccess(node);
        if(access != mega::MegaShare::ACCESS_FULL && access != mega::MegaShare::ACCESS_OWNER)
        {
            return nullptr;
        }
    }
    else if(!mShowReadOnlyFolders)
    {
        if(MegaSyncApp->getMegaApi()->getAccess(node) == mega::MegaShare::ACCESS_READ
            || !node->isNodeKeyDecrypted())
        {
            return nullptr;
        }
    }

    NodeSelectorModelItemSearch::Types type = NodeSelectorModelSearch::calculateSearchType(node);

    if(typesAllowed & type)
    {
        mSearchedTypes |= type;
        auto nodeUptr = std::unique_ptr<mega::MegaNode>(node->copy());
        auto item = new NodeSelectorModelItemSearch(std::move(nodeUptr), type);
        connect(item,
                &NodeSelectorModelItemSearch::typeChanged,
                this,
                &NodeRequester::onSearchItemTypeChanged);
        return item;
    }

    return nullptr;
}

void NodeRequester::createCloudDriveRootItem()
{
    auto root = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getRootNode());

    if(!isAborted())
    {
        auto item = new NodeSelectorModelItemCloudDrive(std::move(root), mShowFiles);
        mRootItems.append(item);
        emit megaCloudDriveRootItemCreated();
    }
}

bool NodeRequester::isIncomingShareCompatible(mega::MegaNode *node)
{
    if(mSyncSetupMode)
    {
        if(MegaSyncApp->getMegaApi()->getAccess(node) != mega::MegaShare::ACCESS_FULL)
        {
            return false;;
        }
    }
    else if(!mShowReadOnlyFolders)
    {
        if(MegaSyncApp->getMegaApi()->getAccess(node) == mega::MegaShare::ACCESS_READ
            || !node->isNodeKeyDecrypted())
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
    for(int i = 0; i < nodeList->size(); i++)
    {
        if(isAborted())
        {
            break;
        }

        if(!isIncomingShareCompatible(nodeList->get(i)))
        {
            continue;
        }

        auto node = std::unique_ptr<mega::MegaNode>(nodeList->get(i)->copy());
        auto user = std::unique_ptr<mega::MegaUser>(megaApi->getUserFromInShare(node.get()));
        NodeSelectorModelItem* item = new NodeSelectorModelItemIncomingShare(std::move(node), mShowFiles);

        items.append(item);

        auto incomingSharesModel = dynamic_cast<NodeSelectorModelIncomingShares*>(mModel);
        if(incomingSharesModel)
        {
            item->setProperty(INDEX_PROPERTY, incomingSharesModel->index(i,0));
            connect(item, &NodeSelectorModelItem::infoUpdated, incomingSharesModel, &NodeSelectorModelIncomingShares::onItemInfoUpdated);
            item->setOwner(std::move(user));
        }
    }

    if(isAborted())
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
    if(isAborted())
    {
        return;
    }

    if(!isIncomingShareCompatible(node.get()))
    {
        return;
    }

    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
    auto user = std::unique_ptr<mega::MegaUser>(megaApi->getUserFromInShare(node.get()));
    NodeSelectorModelItem* item = new NodeSelectorModelItemIncomingShare(std::unique_ptr<mega::MegaNode>(node->copy()), mShowFiles);

    auto incomingSharesModel = dynamic_cast<NodeSelectorModelIncomingShares*>(mModel);
    if(incomingSharesModel)
    {
        item->setProperty(INDEX_PROPERTY, incomingSharesModel->index(incomingSharesModel->rowCount(),0));
        connect(item, &NodeSelectorModelItem::infoUpdated, incomingSharesModel, &NodeSelectorModelIncomingShares::onItemInfoUpdated);
        item->setOwner(std::move(user));
    }

    if(isAborted())
    {
        item->deleteLater();
    }
    else
    {
        mRootItems.append(item);
        emit rootItemsAdded();
    }
}

void NodeRequester::createRubbishRootItems()
{
    if (!isAborted())
    {
        auto item = new NodeSelectorModelItemRubbish(
            std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getRubbishNode()),
            mShowFiles);
        mRootItems.append(item);
        emit megaRubbishRootItemsCreated();
    }
}

void NodeRequester::createBackupRootItems(mega::MegaHandle backupsHandle)
{
    if (backupsHandle != mega::INVALID_HANDLE)
    {
        std::unique_ptr<mega::MegaNode> backupsNode(MegaSyncApp->getMegaApi()->getNodeByHandle(backupsHandle));
        if(backupsNode)
        {
            if(!isAborted())
            {
                NodeSelectorModelItem* item = new NodeSelectorModelItemBackup(std::move(backupsNode), mShowFiles);
                // Here we are setting my backups node as vault node in the item, it is not the same
                // vault node that we get doing megaapi->getVaultNode(), we have to hide it here
                // thats why are doing this trick. The real vault is the parent of my backups folder
                mRootItems.append(item);
            }
        }
    }

    if(!isAborted())
    {
        emit megaBackupRootItemsCreated();
    }
}

void NodeRequester::onAddNodesRequested(QList<std::shared_ptr<mega::MegaNode>> newNodes, const QModelIndex& parentIndex, NodeSelectorModelItem *parentItem)
{
    auto lastChild = parentItem->getNumChildren();
    lockDataMutex(true);
    auto childrenItem = parentItem->addNodes(newNodes);
    lockDataMutex(false);
    foreach(auto& childItem, childrenItem)
    {
        childItem->setProperty(INDEX_PROPERTY, mModel->index(lastChild, 0, parentIndex));
        lastChild++;
    }

    if(!isAborted())
    {
        emit nodesAdded(childrenItem);
    }
    else
    {
        foreach(auto& childItem, childrenItem)
        {
            removeItem(childItem);
        }
    }
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
}

void NodeRequester::removeRootItem(std::shared_ptr<mega::MegaNode> node)
{
    if(isAborted())
    {
        return;
    }

    auto rootFound = std::find_if(mRootItems.begin(), mRootItems.end(), [node](NodeSelectorModelItem* item){
        return item->getNode()->getHandle() == node->getHandle();
    });

    if(rootFound != mRootItems.end())
    {
        mRootItems.removeOne(*rootFound);
        emit rootItemsDeleted();
    }
}

int NodeRequester::rootIndexSize() const
{
    QMutexLocker lock(&mDataMutex);
    return mRootItems.size();
}

int NodeRequester::rootIndexOf(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mDataMutex);
    return mRootItems.indexOf(item);
}

NodeSelectorModelItem *NodeRequester::getRootItem(int index) const
{
    QMutexLocker lock(&mDataMutex);
    return mRootItems.at(index);
}

void NodeRequester::restartSearch()
{
    if(mCancelToken)
    {
        mCancelToken->cancel();
        mSearchCanceled = true;
        mCancelToken.reset(mega::MegaCancelToken::createInstance());
    }
}

void NodeRequester::cancelCurrentRequest()
{
    if(mCancelToken)
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

const NodeSelectorModelItemSearch::Types &NodeRequester::searchedTypes() const
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

void NodeRequester::onSearchItemTypeChanged(NodeSelectorModelItemSearch::Types type)
{
    mSearchedTypes |= type;
}

/* ------------------- MODEL ------------------------- */

const int NodeSelectorModel::ROW_HEIGHT = 25;
const QString MIME_DATA_INTERNAL_MOVE = QLatin1String("application/node_move");

NodeSelectorModel::NodeSelectorModel(QObject* parent):
    QAbstractItemModel(parent),
    mSyncSetupMode(false),
    mIsBeingModified(false),
    mIsProcessingMoves(false),
    mAcceptDragAndDrop(false),
    mMoveRequestsCounter(0),
    mAddNodesQueue(this),
    mExtraSpaceAdded(false),
    mExtraSpaceRemoved(false),
    mRemovingPreviousExtraSpace(false)
{
    mCameraFolderAttribute = UserAttributes::CameraUploadFolder::requestCameraUploadFolder();
    mMyChatFilesFolderAttribute = UserAttributes::MyChatFilesFolder::requestMyChatFilesFolder();

    mNodeRequesterThread = new QThread();
    mNodeRequesterWorker = new NodeRequester(this);
    mNodeRequesterWorker->moveToThread(mNodeRequesterThread);
    mNodeRequesterThread->start();

    connect(this, &NodeSelectorModel::requestChildNodes, mNodeRequesterWorker, &NodeRequester::requestNodeAndCreateChildren, Qt::QueuedConnection);
    connect(this, &NodeSelectorModel::requestAddNodes, mNodeRequesterWorker, &NodeRequester::onAddNodesRequested, Qt::QueuedConnection);
    connect(this, &NodeSelectorModel::removeItem, mNodeRequesterWorker, &NodeRequester::removeItem);
    connect(this, &NodeSelectorModel::removeRootItem, this, [this](NodeSelectorModelItem* item)
            {
                mNodeRequesterWorker->removeRootItem(item);
            });

    connect(mNodeRequesterThread, &QThread::finished, mNodeRequesterThread, &QObject::deleteLater, Qt::DirectConnection);
    connect(mNodeRequesterThread, &QThread::finished, mNodeRequesterWorker, &QObject::deleteLater, Qt::DirectConnection);

    connect(mNodeRequesterWorker, &NodeRequester::nodesReady, this, &NodeSelectorModel::onChildNodesReady, Qt::QueuedConnection);
    connect(mNodeRequesterWorker, &NodeRequester::nodesAdded, this, &NodeSelectorModel::onNodesAdded, Qt::QueuedConnection);

    connect(mNodeRequesterWorker, &NodeRequester::rootItemsAdded, this, &NodeSelectorModel::onRootItemAdded, Qt::QueuedConnection);
    connect(mNodeRequesterWorker, &NodeRequester::rootItemsDeleted, this, &NodeSelectorModel::onRootItemDeleted, Qt::QueuedConnection);

    connect(mNodeRequesterWorker, &NodeRequester::updateLoadingMessage, this, &NodeSelectorModel::updateLoadingMessage, Qt::DirectConnection);

    connect(SyncInfo::instance(), &SyncInfo::syncStateChanged, this, &NodeSelectorModel::onSyncStateChanged);
    connect(SyncInfo::instance(), &SyncInfo::syncRemoved, this, &NodeSelectorModel::onSyncStateChanged);

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
}

NodeSelectorModel::~NodeSelectorModel()
{
    mNodeRequesterThread->quit();
    mNodeRequesterThread->wait();
}

void NodeSelectorModel::setIsModelBeingModified(bool state)
{
    mIsBeingModified = state;
    emit modelIsBeingModifiedChanged(state);
}

void NodeSelectorModel::protectModelWhenPerformingActions()
{
    auto protectModel = [this](){
        setIsModelBeingModified(true);
    };

    connect(this, &NodeSelectorModel::rowsAboutToBeInserted, protectModel);
    connect(this, &NodeSelectorModel::rowsAboutToBeRemoved, protectModel);
    connect(this, &NodeSelectorModel::rowsAboutToBeMoved, protectModel);
    connect(this, &NodeSelectorModel::modelAboutToBeReset, protectModel);

    auto unprotectModel = [this](){
        setIsModelBeingModified(false);
    };
    connect(this, &NodeSelectorModel::rowsInserted, unprotectModel);
    connect(this, &NodeSelectorModel::rowsRemoved, unprotectModel);
    connect(this, &NodeSelectorModel::modelReset, unprotectModel);
    connect(this, &NodeSelectorModel::rowsMoved, unprotectModel);
}

void NodeSelectorModel::executeRemoveExtraSpaceLogic(const QModelIndex& previousIndex)
{
    if (canDropMimeData())
    {
        // Remove the previous current index extra row
        if (mExtraSpaceAdded && previousIndex.isValid() && !mExtraSpaceRemoved)
        {
            mRemovingPreviousExtraSpace = true;

            auto lastRow = rowCount(previousIndex) - 1;
            beginRemoveRows(previousIndex, lastRow, lastRow);
            endRemoveRows();

            mAddedIndex = QModelIndex();
            mRemovingPreviousExtraSpace = false;
            mExtraSpaceRemoved = true;
            mExtraSpaceAdded = false;
        }
    }
}

void NodeSelectorModel::executeAddExtraSpaceLogic(const QModelIndex& currentIndex)
{
    if (canDropMimeData())
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
                    mAddedIndex = createIndex(totalRows, 0, nullptr);
                }
            }
        }
    }
}

void NodeSelectorModel::executeExtraSpaceLogic()
{
    executeRemoveExtraSpaceLogic(mCurrentRootIndex);
    mCurrentRootIndex = mPendingRootIndex;
    executeAddExtraSpaceLogic(mCurrentRootIndex);

    mPendingRootIndex = QModelIndex();
}

int NodeSelectorModel::columnCount(const QModelIndex &) const
{
    return last;
}

QVariant NodeSelectorModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());

        switch (role)
        {
            case toInt(NodeSelectorModelRoles::EXTRA_ROW_ROLE):
            {
                return item == nullptr;
            }
            default:
            {
                break;
            }
        }

        if (item)
        {
            switch(role)
            {
            case Qt::DecorationRole:
            {
                return getIcon(index, item);
            }
            case Qt::DisplayRole:
            {
                return getText(index, item);
            }
            case Qt::SizeHintRole:
            {
                return QSize(0, ROW_HEIGHT);
            }
            case  Qt::TextAlignmentRole:
            {
                if (index.column() == STATUS || index.column() == USER || index.column() == ACCESS)
                {
                    return QVariant::fromValue<Qt::Alignment>(Qt::AlignHCenter | Qt::AlignCenter);
                }
                break;
            }
            case Qt::ToolTipRole:
            {
                if(index.column() == USER)
                {
                    return item->getOwnerName() + QLatin1String(" (") + item->getOwnerEmail() + QLatin1String(")");
                }
                else if(mSyncSetupMode)
                {
                    if((item->getStatus() == NodeSelectorModelItem::Status::SYNC)
                            || (item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD))
                    {
                        return tr("Folder already synced");
                    }
                    else if(item->getStatus() == NodeSelectorModelItem::Status::SYNC_PARENT)
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
                return Utilities::getNodeAccess(item->getNode().get());
            }
            case toInt(NodeSelectorModelRoles::HANDLE_ROLE):
            {
                return QVariant::fromValue(item->getNode() ?
                                               item->getNode()->getHandle()
                                             : mega::INVALID_HANDLE);
            }
            case toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE):
            {
                return QVariant::fromValue(item);
            }
            case toInt(NodeSelectorModelRoles::NODE_ROLE):
            {
                return QVariant::fromValue(item->getNode());
            }
            case toInt(NodeRowDelegateRoles::INDENT_ROLE):
            {
                return item->isCloudDrive() || item->isVault() || item->isRubbishBin()? -10 : 0;
            }
            case toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE):
            {
                return item->isCloudDrive() || item->isVault() ? true : false;
            }
            case toInt(NodeRowDelegateRoles::INIT_ROLE):
            {
                return item->areChildrenInitialized();
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

Qt::ItemFlags NodeSelectorModel::flags(const QModelIndex &index) const
{
    auto flags = QAbstractItemModel::flags(index);

    if (index.isValid())
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if (item)
        {
            if((mSyncSetupMode && !item->isSyncable()) || (item->getNode() && !item->getNode()->isNodeKeyDecrypted()))
            {
                flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
            }

            if(mAcceptDragAndDrop)
            {
                flags |= Qt::ItemIsDropEnabled;

                if (!item->isSpecialNode() && !item->isInShare())
                {
                    flags |= Qt::ItemIsDragEnabled;
                }
            }
        }
        //no item -> extra space row
        else if(mExtraSpaceAdded && mAddedIndex.parent() == index.parent())
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

bool NodeSelectorModel::acceptDragAndDrop(const QMimeData* data)
{
    return (data->hasUrls() || data->hasFormat(MIME_DATA_INTERNAL_MOVE));
}

bool NodeSelectorModel::canDropMimeData(const QMimeData* data,
    Qt::DropAction action,
    int row,
    int column,
    const QModelIndex& parent) const
{
    Q_UNUSED(row);
    Q_UNUSED(column);

    if (action == Qt::CopyAction || action == Qt::MoveAction)
    {
        if (parent.isValid())
        {
            if (action == Qt::CopyAction)
            {
                return true;
            }
            else
            {
                return checkDraggedMimeData(data);
            }
        }
        else
        {
            return checkDraggedMimeData(data);
        }
    }

    return false;
}

bool NodeSelectorModel::canDropMimeData() const
{
    return true;
}

bool NodeSelectorModel::checkDraggedMimeData(const QMimeData* data) const
{
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

                nodesToMove.append(
                    qMakePair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>(handle,
                                                                                 targetNode));
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

    QtConcurrent::run(
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
    // Reset values
    {
        QWriteLocker lock(&mRequestCounterLock);
        mRequestsBeingProcessed.clear();
    }
    mRequestFailedByHandle.clear();
    mFailedMerges.clear();

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

    int requestCounter(0);

    foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
    {
        if (resolvedConflict->getSolution() == NodeItemType::DONT_UPLOAD)
        {
            continue;
        }

        if (auto resolvedMoveConflict = std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
        {
            std::shared_ptr<mega::MegaNode> nodeToMove(
                MegaSyncApp->getMegaApi()->getNodeByHandle(resolvedMoveConflict->getSourceItemHandle()));
            if (nodeToMove)
            {
                requestCounter++;

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
                }
                else
                {
                    mExpectedNodesUpdates.append(nodeToMove->getHandle());

                    if (decision == NodeItemType::FILE_UPLOAD_AND_REPLACE)
                    {
                        emit itemAboutToBeReplaced(
                            resolvedMoveConflict->getConflictNode()->getHandle());

                        if (type == MoveActionType::COPY)
                        {
                            copyFileAndReplace(nodeToMove,
                                               resolvedMoveConflict->getConflictNode(),
                                               resolvedMoveConflict->getParentNode());
                        }
                        else
                        {
                            moveFileAndReplace(nodeToMove,
                                               resolvedMoveConflict->getConflictNode(),
                                               resolvedMoveConflict->getParentNode());
                        }
                    }
                    else if (decision == NodeItemType::UPLOAD_AND_RENAME)
                    {
                        if (type == MoveActionType::COPY)
                        {
                            copyNodeAndRename(nodeToMove,
                                              resolvedMoveConflict->getNewName(),
                                              resolvedMoveConflict->getParentNode());
                        }
                        else
                        {
                            moveNodeAndRename(nodeToMove,
                                              resolvedMoveConflict->getNewName(),
                                              resolvedMoveConflict->getParentNode());
                        }
                    }
                    else if (decision == NodeItemType::UPLOAD)
                    {
                        if (type == MoveActionType::COPY)
                        {
                            copyNode(nodeToMove, resolvedMoveConflict->getParentNode());
                        }
                        else
                        {
                            moveNode(nodeToMove, resolvedMoveConflict->getParentNode());
                        }
                    }
                }
            }
        }
    }

    initRequestsBeingProcessed(type, requestCounter);
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

    if (!conflicts->isEmpty())
    {
        if (!handleAndTarget.isEmpty())
        {
            ignoreDuplicatedNodeOptions(handleAndTarget.first().second);
        }

        emit showDuplicatedNodeDialog(conflicts, type);
    }
    else
    {
        processNodesAfterConflictCheck(conflicts, type);
    }

    return true;
}

QMimeData* NodeSelectorModel::mimeData(const QModelIndexList &indexes) const
{
    QMimeData *mimeData = new QMimeData;
    QByteArray encodedData;

    QDataStream stream(&encodedData, QIODevice::WriteOnly);
    QSet<mega::MegaHandle> processedHandles;

    for(const QModelIndex& index : indexes)
    {
        if(index.isValid())
        {
            if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(index.internalPointer()))
            {
                auto handle(chkItem->getNode()->getHandle());
                if(!processedHandles.contains(handle))
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
    QMimeData *mimeData = new QMimeData;
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

void NodeSelectorModel::resetMoveProcessing()
{
    mMoveRequestsCounter = 0;
    checkMoveProcessing();
    emit levelsAdded(mIndexesToBeExpanded, true);
}

bool NodeSelectorModel::checkMoveProcessing()
{
    if (mIsProcessingMoves && mMoveRequestsCounter == 0)
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
    if (number > 0 && mMoveRequestsCounter > 0)
    {
        mMoveRequestsCounter -= number;
        if (mMoveRequestsCounter < 0)
        {
            mMoveRequestsCounter = 0;
        }

        return checkMoveProcessing();
    }

    return false;
}

bool NodeSelectorModel::isMovingNodes() const
{
    return mIsProcessingMoves;
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

bool NodeSelectorModel::increaseMovingNodes(int number)
{
    if (mMoveRequestsCounter == 0)
    {
        mIsProcessingMoves = true;
        mMoveRequestsCounter = number;
        sendBlockUiSignal(true);
        return true;
    }
    else
    {
        mMoveRequestsCounter += number;
        return false;
    }
}

void NodeSelectorModel::sendBlockUiSignal(bool state)
{
    emit blockUi(state, QPrivateSignal());
}

QModelIndex NodeSelectorModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;

    if (hasIndex(row, column, parent))
    {
        if (parent.isValid())
        {
            mNodeRequesterWorker->lockDataMutex(true);
            NodeSelectorModelItem* item(static_cast<NodeSelectorModelItem*>(parent.internalPointer()));
            if(item)
            {
                index = createIndex(row, column, item->getChild(row).data());
            }
            mNodeRequesterWorker->lockDataMutex(false);
        }
        else if(mNodeRequesterWorker->rootIndexSize() > row)
        {
            auto rootItem = mNodeRequesterWorker->getRootItem(row);
            index = createIndex(row, column, rootItem);
        }
    }

    return index;
}

QModelIndex NodeSelectorModel::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;

    if(index.isValid())
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if(item)
        {
            NodeSelectorModelItem* parent = item->getParent();
            if (parent)
            {
                auto indexOfParent = mNodeRequesterWorker->rootIndexOf(parent);
                if(indexOfParent >= 0)
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

int NodeSelectorModel::rowCount(const QModelIndex &parent) const
{
    int rows(0);

    if (parent.isValid())
    {
        mNodeRequesterWorker->lockDataMutex(true);
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
        rows = item ? item->getNumChildren() : 0;
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

bool NodeSelectorModel::hasChildren(const QModelIndex &parent) const
{
    /////FROM MODEL TESTER:
    // Column 0                | Column 1    |
    // QModelIndex()           |             |
    //    \- topIndex          | topIndex1   |
    //         \- childIndex   | childIndex1 |

    // Common error test #3, the second column should NOT have the same children
    // as the first column in a row.
    // Usually the second column shouldn't have children.

    if(parent.isValid() && parent.column() != NODE)
    {
        return false;
    }

    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if(item && item->getNode())
    {
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
    if(orientation == Qt::Orientation::Horizontal)
    {
        if(role == Qt::DisplayRole)
        {
             switch(section)
             {
             case STATUS:
             case USER:
             {
                 return QLatin1String();
             }
             case ACCESS:
             {
                 return tr("Access");
             }
             case DATE:
             {
                 return tr("Recently used");
             }
             case NODE:
             {
                 return tr("Name");
             }
             }
        }
        else if(role == Qt::ToolTipRole)
        {
            switch(section)
            {
            case STATUS:
            {
                return tr("Sort by status");
            }
            case USER:
            {
                return tr("Sort by owner name");
            }
            case ACCESS:
            {
                return tr("Sort by access");
            }
            case DATE:
            {
                return tr("Sort by date");
            }
            case NODE:
            {
                return tr("Sort by name");
            }
            }
        }
        else if(role == toInt(HeaderRoles::ICON_ROLE))
        {
            if(section == USER)
            {
                return QIcon(QLatin1String("://images/node_selector/icon_small_user.png"));
            }
            else if(section == STATUS)
            {
                return QIcon(QLatin1String("://images/node_selector/icon-small-MEGA.png"));
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
    if(!nodes.isEmpty())
    {
        if(parent.isValid())
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
                    auto totalRows = rowCount(parent);
                    beginInsertRows(parent, totalRows, totalRows + nodes.size() - 1);
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
    if(showsSyncStates() && sync)
    {
        auto syncIndex = findIndexByNodeHandle(sync->getMegaHandle(), QModelIndex());
        auto item = getItemByIndex(syncIndex);
        if(item)
        {
            auto itemStatus = item->getStatus();
            item->calculateSyncStatus();

            if(itemStatus != item->getStatus())
            {
                sendBlockUiSignal(true);
                QtConcurrent::run([this, item, sync](){

                    //Update its children
                    if(item->areChildrenInitialized())
                    {
                        for(int index = 0; index < item->getNumChildren(); ++index)
                        {
                            item->getChild(index)->calculateSyncStatus();
                        }
                    }

                    //Update its parent
                    NodeSelectorModelItem* parent(item->getParent());
                    while(parent)
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
}

void NodeSelectorModel::onRootItemDeleted()
{
    endRemoveRows();
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
    QtConcurrent::run(
        [this, nodeHandles, type]()
        {
            auto requestCounter(0);

            foreach(auto handle, nodeHandles)
            {
                std::shared_ptr<mega::MegaNode> node(
                    MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
                if (node)
                {
                    requestCounter++;

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
            }

            initRequestsBeingProcessed(type, requestCounter);
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

    //Return false if there are no handles (disabled rows...)
    return !handles.isEmpty();
}

bool NodeSelectorModel::areAllNodesEligibleForRestore(const QList<mega::MegaHandle>& handles) const
{
    auto restorableItems(handles.size());

    for (const auto& nodeHandle: handles)
    {
        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(nodeHandle));
        if(node && MegaSyncApp->getMegaApi()->isInRubbish(node.get()))
        {
            std::unique_ptr<mega::MegaNode> parentNode(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
            auto previousParentNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(node->getRestoreHandle()));

            if(previousParentNode && !MegaSyncApp->getMegaApi()->isInRubbish(previousParentNode.get()))
            {
                restorableItems--;
            }
        }
    }

    return restorableItems == 0;
}

void NodeSelectorModel::deleteNodeFromModel(const QModelIndex& index)
{
    if(!index.isValid())
    {
        return;
    }
    auto item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
    if(item)
    {
        std::shared_ptr<mega::MegaNode> node = item->getNode();
        if (node)
        {
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

            NodeSelectorModelItem* parent = static_cast<NodeSelectorModelItem*>(index.parent().internalPointer());
            if(parent)
            {
                int row = parent->indexOf(item);
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
        }
    }
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
    QtConcurrent::run(
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
    QtConcurrent::run(
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
    MegaSyncApp->getMegaApi()->moveNode(
        moveNode.get(),
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

void NodeSelectorModel::moveNode(std::shared_ptr<mega::MegaNode> moveNode, std::shared_ptr<mega::MegaNode> targetParentFolder)
{
    MegaSyncApp->getMegaApi()->moveNode(
        moveNode.get(),
        targetParentFolder.get(),
        mListener.get());
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

void NodeSelectorModel::checkFinishedRequest(mega::MegaHandle handle, int errorCode)
{
    int requestType = requestFinished();

    if (errorCode != mega::MegaError::API_OK || handle == mega::INVALID_HANDLE)
    {
        mRequestFailedByHandle.insert(handle, requestType);
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));

    if (node)
    {
        MovedItemsType itemType = node->isFile() ? MovedItemsType::FILES : MovedItemsType::FOLDERS;
        mMovedItemsType |= itemType;

        if (mRequestsBeingProcessed.counter == 0)
        {
            if (!mRequestFailedByHandle.isEmpty())
            {
                QMegaMessageBox::MessageBoxInfo msgInfo;
                msgInfo.buttonsText.insert(QMessageBox::StandardButton::Ok, tr("Close"));
                msgInfo.title = MegaSyncApp->getMEGAString();

                auto multipleRequest(mRequestFailedByHandle.size() > 1);

                if (requestType == MoveActionType::MOVE)
                {
                    if (multipleRequest)
                    {
                        if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                            mMovedItemsType.testFlag(MovedItemsType::BOTH))
                        {
                            msgInfo.text = tr("Error moving items");
                            msgInfo.informativeText =
                                tr("The items couldn’t be moved. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                        {
                            msgInfo.text = tr("Error moving files");
                            msgInfo.informativeText =
                                tr("The files couldn’t be moved. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                        {
                            msgInfo.text = tr("Error moving folders");
                            msgInfo.informativeText =
                                tr("The folders couldn’t be moved. Try again later");
                        }
                    }
                    else
                    {
                        std::unique_ptr<mega::MegaNode> node(
                            MegaSyncApp->getMegaApi()->getNodeByHandle(
                                mRequestFailedByHandle.firstKey()));

                        if (node->isFile())
                        {
                            msgInfo.text = tr("Error moving file");
                            msgInfo.informativeText =
                                tr("The file %1 couldn’t be moved. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                        else
                        {
                            msgInfo.text = tr("Error moving folder");
                            msgInfo.informativeText =
                                tr("The folder %1 couldn’t be moved. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                    }
                }
                else if (requestType == MoveActionType::COPY)
                {
                    if (multipleRequest)
                    {
                        if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                            mMovedItemsType.testFlag(MovedItemsType::BOTH))
                        {
                            msgInfo.text = tr("Error copying items");
                            msgInfo.informativeText =
                                tr("The items couldn’t be copied. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                        {
                            msgInfo.text = tr("Error copying files");
                            msgInfo.informativeText =
                                tr("The files couldn’t be copied. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                        {
                            msgInfo.text = tr("Error copying folders");
                            msgInfo.informativeText =
                                tr("The folders couldn’t be copied. Try again later");
                        }
                    }
                    else
                    {
                        std::unique_ptr<mega::MegaNode> node(
                            MegaSyncApp->getMegaApi()->getNodeByHandle(
                                mRequestFailedByHandle.firstKey()));

                        if (node->isFile())
                        {
                            msgInfo.text = tr("Error copying file");
                            msgInfo.informativeText =
                                tr("The file %1 couldn’t be copied. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                        else
                        {
                            msgInfo.text = tr("Error copying folder");
                            msgInfo.informativeText =
                                tr("The folder %1 couldn’t be copied. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                    }
                }
                else if (requestType == MoveActionType::RESTORE)
                {
                    if (multipleRequest)
                    {
                        if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                            mMovedItemsType.testFlag(MovedItemsType::BOTH))
                        {
                            msgInfo.text = tr("Error restoring items");
                            msgInfo.informativeText =
                                tr("The items couldn’t be restored. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                        {
                            msgInfo.text = tr("Error restoring files");
                            msgInfo.informativeText =
                                tr("The files couldn’t be restored. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                        {
                            msgInfo.text = tr("Error restoring folders");
                            msgInfo.informativeText =
                                tr("The folders couldn’t be restored. Try again later");
                        }
                    }
                    else
                    {
                        std::unique_ptr<mega::MegaNode> node(
                            MegaSyncApp->getMegaApi()->getNodeByHandle(
                                mRequestFailedByHandle.firstKey()));

                        if (node->isFile())
                        {
                            msgInfo.text = tr("Error restoring file");
                            msgInfo.informativeText =
                                tr("The file %1 couldn’t be restored. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                        else
                        {
                            msgInfo.text = tr("Error restoring folder");
                            msgInfo.informativeText =
                                tr("The folder %1 couldn’t be restored. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                    }
                }
                else if (requestType >= MoveActionType::DELETE_RUBBISH)
                {
                    if (multipleRequest)
                    {
                        if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                            mMovedItemsType.testFlag(MovedItemsType::BOTH))
                        {
                            msgInfo.text = tr("Error deleting items");
                            msgInfo.informativeText =
                                tr("The items couldn’t be deleted. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                        {
                            msgInfo.text = tr("Error deleting files");
                            msgInfo.informativeText =
                                tr("The files couldn’t be deleted. Try again later");
                        }
                        else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                        {
                            msgInfo.text = tr("Error deleting folders");
                            msgInfo.informativeText =
                                tr("The folders couldn’t be deleted. Try again later");
                        }
                    }
                    else
                    {
                        std::unique_ptr<mega::MegaNode> node(
                            MegaSyncApp->getMegaApi()->getNodeByHandle(
                                mRequestFailedByHandle.firstKey()));

                        if (node->isFile())
                        {
                            msgInfo.text = tr("Error deleting file");
                            msgInfo.informativeText =
                                tr("The file %1 couldn’t be deleted. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                        else
                        {
                            msgInfo.text = tr("Error deleting folder");
                            msgInfo.informativeText =
                                tr("The folder %1 couldn’t be deleted. Try again later")
                                    .arg(MegaNodeNames::getNodeName(node.get()));
                        }
                    }
                }

                // Show dialog
                emit showMessageBox(msgInfo);
            }

            // Reset values for next move action
            mMovedItemsType = MovedItemsType::NONE;
            emit allNodeRequestsFinished();
        }
    }

    if (mRequestsBeingProcessed.counter == 0 && !mRequestFailedByHandle.isEmpty())
    {
        if (mRequestFailedByHandle.size() != mFailedMerges.size())
        {
            if (!mFailedMerges.isEmpty())
            {
                for (const auto& mergeInfo: std::as_const(mFailedMerges))
                {
                    mRequestFailedByHandle.remove(mergeInfo->nodeTarget->getHandle());
                }
            }

            emit itemsAboutToBeMovedFailed(mRequestFailedByHandle.keys(), requestType);
        }

        // Reset value
        mRequestFailedByHandle.clear();
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

QVariant NodeSelectorModel::getIcon(const QModelIndex &index, NodeSelectorModelItem* item) const
{
    switch(index.column())
    {
    case COLUMN::NODE:
    {
        return QVariant::fromValue<QIcon>(getFolderIcon(item));
    }
    case COLUMN::DATE:
    {
        break;
    }
    case COLUMN::USER:
    {
        return QVariant::fromValue<QPixmap>(item->getOwnerIcon());
    }
    case COLUMN::STATUS:
    {
        return QVariant::fromValue<QIcon>(item->getStatusIcons());
    }
    default:
        break;
    }
    return QVariant();
}

QVariant NodeSelectorModel::getText(const QModelIndex &index, NodeSelectorModelItem *item) const
{
    switch(index.column())
    {
        case COLUMN::NODE:
        {
            if(item->isVault() || item->isCloudDrive())
            {
                return MegaNodeNames::getRootNodeName(item->getNode().get());
            }
            // SDK returns "Rubbish Bin" and we use "Rubbish bin", so we cannot directly translate
            // the node name (we don´t have "Rubbish Bin" in our translation files)
            else if (item->isRubbishBin())
            {
                return MegaNodeNames::getRubbishName();
            }
            else
            {
                return MegaNodeNames::getNodeName(item->getNode().get());
            }
        }
        case COLUMN::DATE:
        {
            if(item->isCloudDrive() || item->isVault())
            {
                return QVariant();
            }

            QDateTime dateTime = dateTime.fromSecsSinceEpoch(item->getNode()->getCreationTime());
            return MegaSyncApp->getFormattedDateByCurrentLanguage(dateTime, QLocale::FormatType::ShortFormat);
        }
        case COLUMN::ACCESS:
        {
            // Only for the top parent inshare
            if (showAccess(item->getNode().get()))
            {
                return Utilities::getNodeStringAccess(item->getNode().get());
            }
        }
        default:
            break;
    }
    return QVariant();
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
    mNodeRequesterWorker->cancelCurrentRequest();
}

bool NodeSelectorModel::canBeDeleted() const
{
    return true;
}

void NodeSelectorModel::loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node)
{
    //First, we set the loading view as it can take long to load the tree path to the node
    if(!isMovingNodes())
    {
        sendBlockUiSignal(true);
    }

    mNodesToLoad.clear();
    mNodesToLoad.append(node);

    auto p_node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getParentNode(node.get()));

    //The vault node is not represented in the node selector, hence if the parent of a node is the vault
    //it doesn´t have to be added to the node list to load. If it is added the loading of a specific node
    //will stops working in backups screen.
    while(addToLoadingList(p_node))
    {
        mIndexesToBeExpanded.append(qMakePair(p_node->getHandle(), QModelIndex()));
        mNodesToLoad.append(p_node);
        p_node.reset(MegaSyncApp->getMegaApi()->getParentNode(p_node.get()));
    }

    if(!fetchMoreRecursively(QModelIndex()))
    {
        sendBlockUiSignal(false);
        mNodesToLoad.clear();
    }
}

bool NodeSelectorModel::fetchMoreRecursively(const QModelIndex& parentIndex)
{
    auto result(false);
    if(!mNodesToLoad.isEmpty())
    {
        auto node = mNodesToLoad.last();
        if(node)
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

void NodeSelectorModel::initRequestsBeingProcessed(int type, int counter)
{
    QWriteLocker lock(&mRequestCounterLock);
    mRequestsBeingProcessed.counter = counter;
    mRequestsBeingProcessed.type = type;
}

int NodeSelectorModel::requestFinished()
{
    QReadLocker lock(&mRequestCounterLock);
    mRequestsBeingProcessed.counter--;
    return mRequestsBeingProcessed.type;
}

// This method looks only in the parent layer, not recursively
QModelIndex NodeSelectorModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex &parent)
{
    if(node)
    {
        auto childrenCount = rowCount(parent);
        for(int row = 0; row < childrenCount; ++row)
        {
            auto indexToCheck = index(row,0,parent);
            NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(indexToCheck.internalPointer());
            if(item)
            {
                if(item->getNode()->getHandle() == node->getHandle())
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
    if(mAddExpaceWhenLoadingFinish)
    {
        executeExtraSpaceLogic();
        mAddExpaceWhenLoadingFinish = false;
    }

    emit levelsAdded(mIndexesToBeExpanded);
}

bool NodeSelectorModel::canFetchMore(const QModelIndex &parent) const
{
    if(!parent.isValid())
    {
        return false;
    }
    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if(item)
    {
        return item->canFetchMore();
    }
    else
    {
        return mNodeRequesterWorker->rootIndexSize() < rootItemsCount();
    }
}

void NodeSelectorModel::setCurrentRootIndex(const QModelIndex& index)
{
    mPendingRootIndex = index.isValid() ? index : getTopRootIndex();

    NodeSelectorModelItem* item =
        static_cast<NodeSelectorModelItem*>(mPendingRootIndex.internalPointer());

    if(item && item->areChildrenInitialized())
    {
        executeExtraSpaceLogic();
        mAddExpaceWhenLoadingFinish = false;
        mPendingRootIndex = QModelIndex();
    }
    else
    {
        mAddExpaceWhenLoadingFinish = true;
    }
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

void NodeSelectorModel::fetchItemChildren(const QModelIndex& parent)
{
    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if(!item->areChildrenInitialized() && !item->requestingChildren())
    {
        // Just in case the children changed
        item->resetChildrenCounter();
        int itemNumChildren = item->getNumChildren();
        if(itemNumChildren > 0)
        {
            sendBlockUiSignal(true);

            blockSignals(true);
            beginInsertRows(parent, 0, itemNumChildren-1);
            blockSignals(false);
            auto info = std::make_shared<MessageInfo>();
            info->message = QLatin1String("Requesting nodes...");
            emit updateLoadingMessage(info);
            emit requestChildNodes(item, parent);

            // Unblock UI when children are added async
            return;
        }
    }
}

void NodeSelectorModel::onChildNodesReady(NodeSelectorModelItem* parent)
{
    auto info = std::make_shared<MessageInfo>();
    info->message = QLatin1String("Filtering items...");
    emit updateLoadingMessage(info);

    auto index = parent->property(INDEX_PROPERTY).value<QModelIndex>();
    continueWithNextItemToLoad(index);
}

bool NodeSelectorModel::continueWithNextItemToLoad(const QModelIndex& parentIndex)
{
    bool result = false;

    if(!mNodesToLoad.isEmpty())
    {
        //The last one has been already processed
        auto lastNode = mNodesToLoad.takeLast();
        if(!mNodesToLoad.isEmpty())
        {
            result = fetchMoreRecursively(parentIndex);
            if (result)
            {
                mIndexesToBeExpanded.append(qMakePair(lastNode->getHandle(), parentIndex));
            }
            else if (!mNodesToLoad.isEmpty())
            {
                //The last node is empty
                mNodesToLoad.removeLast();
            }
        }
    }

    if(mNodesToLoad.isEmpty())
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
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex idx = index(i, COLUMN::NODE, parent);
        if(idx.isValid())
        {
            if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
            {
                if(chkItem->getNode()->getHandle() == handle)
                {
                    return idx;
                }
            }
        }
    }
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex child = parent.isValid() ? index(i, COLUMN::NODE, parent) : index(i, COLUMN::NODE);
        if(child.isValid())
        {
            auto ret = findIndexByNodeHandle(handle, child);
            if(ret.isValid())
            {
                return ret;
            }
        }
    }

    return QModelIndex();
}

NodeSelectorModelItem* NodeSelectorModel::getItemByIndex(const QModelIndex &index)
{
    return qvariant_cast<NodeSelectorModelItem*>(index.data(toInt(NodeSelectorModelRoles::MODEL_ITEM_ROLE)));
}

void NodeSelectorModel::updateItemNode(const QModelIndex &indexToUpdate, std::shared_ptr<mega::MegaNode> node)
{
    auto item = getItemByIndex(indexToUpdate);
    if(item)
    {
        item->updateNode(node);
        updateRow(indexToUpdate);
    }
}

void NodeSelectorModel::updateRow(const QModelIndex& indexToUpdate)
{
    auto firstColumnIndex = index(indexToUpdate.row(), 0, indexToUpdate.parent());
    auto lastColumnIndex = index(indexToUpdate.row(), columnCount()-1, indexToUpdate.parent());
    emit dataChanged(firstColumnIndex, lastColumnIndex);
}

QIcon NodeSelectorModel::getFolderIcon(NodeSelectorModelItem *item) const
{
    if(item)
    {
        auto node = item->getNode();

        if(node)
        {
            if (node->getType() >= mega::MegaNode::TYPE_FOLDER)
            {
                if(node->getHandle() == mCameraFolderAttribute->getCameraUploadFolderHandle()
                        || node->getHandle() == mCameraFolderAttribute->getCameraUploadFolderSecondaryHandle())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/icons/folder/small-camera-sync.png"), QSize(), QIcon::Normal);
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-camera-sync-disabled.png"), QSize(), QIcon::Disabled);
                    return icon;;
                }
                else if(node->getHandle() == mMyChatFilesFolderAttribute->getMyChatFilesFolderHandle())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/icons/folder/small-chat-files.png"), QSize(), QIcon::Normal);
                    icon.addFile(QLatin1String("://images/icons/folder/small-chat-files-disabled.png"), QSize(), QIcon::Disabled);
                    return icon;
                }
                else if (node->isInShare())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-incoming.png"), QSize(), QIcon::Normal);
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-incoming-disabled.png"), QSize(), QIcon::Disabled);
                    return icon;
                }
                else if (node->isOutShare())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-outgoing.png"), QSize(), QIcon::Normal);
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-outgoing-disabled.png"), QSize(), QIcon::Disabled);
                    return icon;
                }
                else if(node->getHandle() == MegaSyncApp->getRootNode()->getHandle())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/ico-cloud-drive.png"), QSize(16, 16));
                    return icon;
                }
                else if(node->getHandle() == MegaSyncApp->getRubbishNode()->getHandle())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/node_selector/view/trash.png"),
                                 QSize(16, 16));
                    return icon;
                }
                else if(item->isVault())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/node_selector/Backups_small_ico.png"),
                                 QSize(16, 16));
                    return icon;
                }
                else
                {
                    QString nodeDeviceId (QString::fromUtf8(node->getDeviceId()));
                    if (!nodeDeviceId.isEmpty())
                    {
                        // TODO, future: choose icon according to host OS
                        if (nodeDeviceId == QString::fromUtf8(MegaSyncApp->getMegaApi()->getDeviceId()))
                        {
#ifdef Q_OS_WINDOWS
                            const QIcon thisDeviceIcon (QLatin1String("://images/icons/pc/pc-win_24.png"));
#elif defined(Q_OS_MACOS)
                            const QIcon thisDeviceIcon (QLatin1String("://images/icons/pc/pc-mac_24.png"));
#elif defined(Q_OS_LINUX)
                            const QIcon thisDeviceIcon (QLatin1String("://images/icons/pc/pc-linux_24.png"));
#endif
                            return thisDeviceIcon;
                        }
                        return QIcon(QLatin1String("://images/icons/pc/pc_24.png"));
                    }
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder.png"), QSize(), QIcon::Normal);
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-disabled.png"), QSize(), QIcon::Disabled);
                    return icon;
                }
            }
            else
            {
                return Utilities::getExtensionPixmapSmall(QString::fromUtf8(node->getName()));
            }
        }
    }

    return QIcon();
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
