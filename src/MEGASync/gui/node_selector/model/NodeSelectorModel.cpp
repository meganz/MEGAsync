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
const int PROTECTION_INTERVAL = 10000; /*10 seconds*/

NodeSelectorModel::NodeSelectorModel(QObject* parent):
    QAbstractItemModel(parent),
    mRequiredRights(mega::MegaShare::ACCESS_READ),
    mDisplayFiles(false),
    mSyncSetupMode(false),
    mIsBeingModified(false),
    mIsProcessingMoves(false),
    mAcceptDragAndDrop(false),
    mMoveRequestsCounter(0),
    mAddNodesQueue(this),
    mRowAdded(false),
    mRowRemoved(false)
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

    protectModelWhenPerformingActions();
    protectModelAgainstUpdateBlockingState();

    mListener = RequestListenerManager::instance().registerAndGetFinishListener(this, false);

    qRegisterMetaType<QList<mega::MegaHandle>>("QList<mega::MegaHandle>");
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

void NodeSelectorModel::protectModelAgainstUpdateBlockingState()
{
    mProtectionAgainstBlockedStateWhileUpdating.setInterval(PROTECTION_INTERVAL);
    mProtectionAgainstBlockedStateWhileUpdating.setSingleShot(true);
    connect(&mProtectionAgainstBlockedStateWhileUpdating,
            &QTimer::timeout,
            this,
            &NodeSelectorModel::resetMoveProcessing);
}

void NodeSelectorModel::executeExtraSpaceLogic()
{
    // Remove the previous current index extra row
    if (mPreviousRootIndex.isValid() && !mRowRemoved)
    {
        auto totalRows = rowCount(mPreviousRootIndex);
        beginRemoveRows(mPreviousRootIndex, totalRows, totalRows);
        endRemoveRows();
        mRowRemoved = true;
    }

    if (mCurrentRootIndex.isValid() && !mRowAdded)
    {
        auto totalRows = rowCount(mCurrentRootIndex);
        beginInsertRows(mCurrentRootIndex, totalRows, totalRows);
        endInsertRows();
        mRowAdded = true;
    }
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
                if(index.column() == STATUS || index.column() == USER)
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
                flags |= (Qt::ItemIsDragEnabled | Qt::ItemIsDropEnabled);
            }
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
    Q_UNUSED(data);
    Q_UNUSED(action);
    Q_UNUSED(row);
    Q_UNUSED(parent);
    Q_UNUSED(column);

    if (action == Qt::CopyAction)
    {
        if (parent.isValid())
        {
            auto item = getItemByIndex(parent);
            if (item)
            {
                auto node = item->getNode();
                if (!node || node->isFolder())
                {
                    return true;
                }
            }
        }
        else
        {
            return true;
        }
    }

    return false;
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
    if (action == Qt::CopyAction)
    {
        return startProcessingNodes(data, parent, ActionType::MOVE);
    }

    return QAbstractItemModel::dropMimeData(data, action, row, column, parent);
}

bool NodeSelectorModel::startProcessingNodes(const QMimeData* data,
                                             const QModelIndex& parent,
                                             ActionType type)
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

                if (type != ActionType::COPY)
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

void NodeSelectorModel::processNodesAfterConflictCheck(std::shared_ptr<ConflictTypes> conflicts,
                                                       ActionType type)
{
    if (conflicts->mResolvedConflicts.isEmpty())
    {
        return;
    }

    QList<mega::MegaHandle> handlesToMove;

    // There are 3 scenarios when restoring

    // 1) It is a simple folder or file and there is no other item with the same name on the CD ->
    // direct restore -> ONLY "UPLOAD"

    // 2) There are two files/folders with the same name in the rubbish and no other item on the CD
    // -> Merge in the rubbish and then restore -> at least two conflicts -> FOLDER_UPLOAD_AND_MERGE
    // && UPLOAD -> In this case, we end restoring a simple file/folder

    // 3) There are on item on the CD with the same name of one or more items on the rubbish -> We
    // merge first in the -> ONLY "FOLDER_UPLOAD_AND_MERGE"

    // Depeding on the scenario, we have 3 types of RestoreMergeType values:

    // 1) No value, we don´t have a merge
    // 2) RestoreMergeType = MERGE_AND_MOVE_TO_TARGET -> We merge folders on rubbish and the we move
    // the final folder to the CD 3) RestoreMergeType = MERGE_ON_EXISTING_TARGET -> We merge folders
    // to the final folder to the CD

    std::optional<RestoreMergeType> restoreMergeType;
    QMap<mega::MegaHandle, mega::MegaHandle> handlesToMerge;
    QSet<mega::MegaHandle> handlesToUpload;

    if (type == ActionType::RESTORE)
    {
        foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
        {
            if (auto resolvedMoveConflict =
                    std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
            {
                if (resolvedMoveConflict->getSolution() == NodeItemType::FOLDER_UPLOAD_AND_MERGE)
                {
                    if (handlesToUpload.contains(
                            resolvedMoveConflict->getConflictNode()->getHandle()))
                    {
                        restoreMergeType = RestoreMergeType::MERGE_AND_MOVE_TO_TARGET;
                    }

                    handlesToMerge.insert(resolvedMoveConflict->getConflictNode()->getHandle(),
                                          resolvedMoveConflict->getSourceItemHandle());
                }
                else if (resolvedMoveConflict->getSolution() == NodeItemType::UPLOAD)
                {
                    if (handlesToMerge.contains(resolvedMoveConflict->getSourceItemHandle()))
                    {
                        restoreMergeType = RestoreMergeType::MERGE_AND_MOVE_TO_TARGET;
                    }
                    else
                    {
                        handlesToUpload.insert(resolvedMoveConflict->getSourceItemHandle());
                    }
                }
            }
        }

        if (!handlesToMerge.isEmpty() && !restoreMergeType.has_value())
        {
            restoreMergeType = RestoreMergeType::MERGE_ON_EXISTING_TARGET;
        }

        if (restoreMergeType.has_value())
        {
            initMovingNodes(handlesToMerge.size() + handlesToUpload.size());
        }
    }

    foreach(auto resolvedConflict, conflicts->mResolvedConflicts)
    {
        if (auto resolvedMoveConflict = std::dynamic_pointer_cast<DuplicatedMoveNodeInfo>(resolvedConflict))
        {
            std::shared_ptr<mega::MegaNode> nodeToMove(
                MegaSyncApp->getMegaApi()->getNodeByHandle(resolvedMoveConflict->getSourceItemHandle()));
            if (nodeToMove)
            {
                auto decision = resolvedMoveConflict->getSolution();
                mRequestByHandle.insert(nodeToMove->getHandle(), mega::MegaRequest::TYPE_MOVE);

                if (decision == NodeItemType::FOLDER_UPLOAD_AND_MERGE)
                {
                    if (type != ActionType::RESTORE)
                    {
                        handlesToMove.append(nodeToMove->getHandle());
                    }

                    mergeFolders(nodeToMove,
                                 resolvedMoveConflict->getConflictNode(),
                                 resolvedMoveConflict->getParentNode(),
                                 type,
                                 restoreMergeType);
                }
                else
                {
                    handlesToMove.append(nodeToMove->getHandle());

                    if (decision == NodeItemType::FILE_UPLOAD_AND_REPLACE)
                    {
                        if (type == ActionType::COPY)
                        {
                            copyFileAndReplace(nodeToMove,
                                               resolvedMoveConflict->getConflictNode(),
                                               resolvedMoveConflict->getParentNode());
                        }
                        else
                        {
                            // Duplicate the value, as the node will be removed and then added to
                            // the restored model
                            handlesToMove.append(nodeToMove->getHandle());

                            moveFileAndReplace(nodeToMove,
                                               resolvedMoveConflict->getConflictNode(),
                                               resolvedMoveConflict->getParentNode());
                        }
                    }
                    else if (decision == NodeItemType::UPLOAD_AND_RENAME)
                    {
                        if (type == ActionType::COPY)
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
                        if (type == ActionType::COPY)
                        {
                            copyNode(nodeToMove, resolvedMoveConflict->getParentNode());
                        }
                        else
                        {
                            if (type == ActionType::RESTORE &&
                                handlesToMerge.contains(nodeToMove->getHandle()))
                            {
                                mMoveAfterMergeWhileRestoring.insert(
                                    handlesToMerge.value(nodeToMove->getHandle()),
                                    resolvedMoveConflict->getParentNode()->getHandle());
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
    }

    // Set loading view in the source model where the move started
    emit itemsAboutToBeMoved(handlesToMove, type);
}

bool NodeSelectorModel::processNodesAndCheckConflicts(
    const QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>>& handleAndTarget,
    std::shared_ptr<mega::MegaNode> sourceNode,
    ActionType type)
{
    if (handleAndTarget.isEmpty())
    {
        return false;
    }

    auto conflicts = CheckDuplicatedNodes::checkMoves(handleAndTarget, sourceNode);

    if (!conflicts->isEmpty())
    {
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

        mProtectionAgainstBlockedStateWhileUpdating.stop();

        emit itemsMoved();

        sendBlockUiSignal(false);

        return true;
    }

    return false;
}

void NodeSelectorModel::restartProtectionAgainstUpdateBlockingState()
{
    // mProtectionAgainstBlockedStateWhileUpdating.start(PROTECTION_INTERVAL);
}

bool NodeSelectorModel::moveProcessed()
{
    if (mMoveRequestsCounter > 0)
    {
        restartProtectionAgainstUpdateBlockingState();
        mMoveRequestsCounter--;
        return checkMoveProcessing();
    }

    return false;
}

bool NodeSelectorModel::isMovingNodes() const
{
    return mIsProcessingMoves;
}

bool NodeSelectorModel::pasteNodes(const QList<mega::MegaHandle>& nodesToCopy,
                                   const QModelIndex& indexToPaste)
{
    auto data(mimeData(nodesToCopy));
    if (canDropMimeData(data, Qt::CopyAction, -1, -1, indexToPaste))
    {
        if (startProcessingNodes(data, indexToPaste, NodeSelectorModel::ActionType::COPY))
        {
            return true;
        }
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

bool NodeSelectorModel::increaseMovingNodes()
{
    if (mMoveRequestsCounter == 0)
    {
        initMovingNodes(1);
        return true;
    }
    else
    {
        mMoveRequestsCounter++;
        restartProtectionAgainstUpdateBlockingState();
        return false;
    }
}

void NodeSelectorModel::increaseMergeMovingNodes(mega::MegaHandle handle, int type)
{
    if (mMoveRequestsCounter == 0)
    {
        initMovingNodes(1);
    }
    else
    {
        increaseMovingNodes();
    }
    emit mergeItemAboutToBeMoved(handle, type);
}

void NodeSelectorModel::initMovingNodes(int number)
{
    mIsProcessingMoves = true;
    mMoveRequestsCounter = number;
    restartProtectionAgainstUpdateBlockingState();
    sendBlockUiSignal(true);
}

void NodeSelectorModel::sendBlockUiSignal(bool state)
{
    emit blockUi(state, QPrivateSignal());
}

void NodeSelectorModel::sendDisableBlockUiSystemSignal(bool state)
{
    emit disableBlockUiSystem(state, QPrivateSignal());
}

void NodeSelectorModel::selectIndexesByHandleAsync(const QList<mega::MegaHandle>& handles)
{
    for (auto& handle: qAsConst(handles))
    {
        mIndexesToBeSelected.append(qMakePair(handle, QModelIndex()));
    }
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
                auto data = item->getChild(row).data();
                if(data)
                {
                    index =  createIndex(row, column, data);
                }
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

void NodeSelectorModel::addNodes(QList<std::shared_ptr<mega::MegaNode>> nodes, const QModelIndex &parent)
{
    if(!nodes.isEmpty())
    {
        if(parent.isValid())
        {
            if (isBeingModified())
            {
                mAddNodesQueue.addStep(nodes, parent);
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
                }
            }
        }
    }
}

void NodeSelectorModel::onNodesAdded(QList<QPointer<NodeSelectorModelItem>> childrenItem)
{
    endInsertRows();

    foreach(auto child, childrenItem)
    {
        auto index = child->property(INDEX_PROPERTY).toModelIndex();
        emit dataChanged(index, index);
    }

    // If we are moving nodes, the loading view is visible
    if (isMovingNodes())
    {
        foreach(auto child, childrenItem)
        {
            Q_UNUSED(child);
            moveProcessed();
        }
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
    emit itemsAboutToBeMoved(nodeHandles,
                             permanently ? ActionType::DELETE_PERMANENTLY :
                                           ActionType::DELETE_RUBBISH);

    // It will be unblocked when all requestFinish calls are received (check onRequestFinish)
    QtConcurrent::run([this, nodeHandles, permanently]() {
        foreach(auto handle, nodeHandles)
        {
            std::shared_ptr<mega::MegaNode> node(
                MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
            if (node)
            {
                mRequestByHandle.insert(handle, mega::MegaRequest::TYPE_REMOVE);

                // Double protection in case the node properties changed while the node is deleted
                if (permanently)
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

    for (auto nodeHandle: qAsConst(handles))
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

void NodeSelectorModel::mergeFolders(std::shared_ptr<mega::MegaNode> moveFolder,
                                     std::shared_ptr<mega::MegaNode> conflictTargetFolder,
                                     std::shared_ptr<mega::MegaNode> targetParentFolder,
                                     ActionType type,
                                     std::optional<RestoreMergeType> restoreType)
{
    QtConcurrent::run(
        [this, moveFolder, targetParentFolder, conflictTargetFolder, type, restoreType]()
        {
            MergeMEGAFolders foldersMerger(MergeMEGAFolders::ActionForDuplicates::Rename,
                                           // Remote is always case sensitive
                                           Qt::CaseSensitive,
                                           type == ActionType::COPY ?
                                               MergeMEGAFolders::Strategy::COPY :
                                               MergeMEGAFolders::Strategy::MOVE);

            if (restoreType.has_value())
            {
                if (restoreType.value() == RestoreMergeType::MERGE_ON_EXISTING_TARGET)
                {
                    connect(&foldersMerger,
                            &MergeMEGAFolders::nestedItemMerged,
                            this,
                            [this, type](mega::MegaHandle handle)
                            {
                                increaseMergeMovingNodes(handle, type);
                            });
                }
                else if (restoreType.value() == RestoreMergeType::MERGE_AND_MOVE_TO_TARGET)
                {
                    connect(&foldersMerger,
                            &MergeMEGAFolders::nestedItemMerged,
                            this,
                            [this](mega::MegaHandle)
                            {
                                increaseMovingNodes();
                            });
                }
            }

            connect(&foldersMerger,
                    &MergeMEGAFolders::finished,
                    this,
                    &NodeSelectorModel::moveProcessed);

            auto e = foldersMerger.merge(conflictTargetFolder.get(), moveFolder.get());

            if (type == ActionType::RESTORE)
            {
                if (e == mega::MegaError::API_OK)
                {
                    auto moveToParentHandle(
                        mMoveAfterMergeWhileRestoring.take(moveFolder->getHandle()));
                    if (moveToParentHandle != mega::INVALID_HANDLE)
                    {
                        std::shared_ptr<mega::MegaNode> targetNode(
                            MegaSyncApp->getMegaApi()->getNodeByHandle(moveToParentHandle));
                        QList<QPair<mega::MegaHandle, std::shared_ptr<mega::MegaNode>>> nodesToMove;
                        nodesToMove.append(
                            qMakePair(conflictTargetFolder->getHandle(), targetNode));

                        processNodesAndCheckConflicts(nodesToMove,
                                                      MegaSyncApp->getRubbishNode(),
                                                      type);
                    }
                }
            }

            emit finishAsyncRequest(moveFolder->getHandle(), e);
        });
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
        type == mega::MegaRequest::TYPE_COPY)
    {
        auto handle(request->getNodeHandle());
        checkFinishedRequest(handle, e->getErrorCode());

        if ((e && e->getErrorCode() != mega::MegaError::API_OK))
        {
            moveProcessed();
        }
    }
}

void NodeSelectorModel::checkFinishedRequest(mega::MegaHandle handle, int errorCode)
{
    auto requestType = mRequestByHandle.take(handle);

    if (errorCode != mega::MegaError::API_OK)
    {
        mRequestFailedByHandle.insert(handle, requestType);
    }

    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    if (node)
    {
        MovedItemsType itemType = node->isFile() ? MovedItemsType::FILES : MovedItemsType::FOLDERS;
        mMovedItemsType |= itemType;
    }

    if (mRequestByHandle.isEmpty())
    {
        if (!mRequestFailedByHandle.isEmpty())
        {
            QMegaMessageBox::MessageBoxInfo msgInfo;
            msgInfo.title = MegaSyncApp->getMEGAString();

            auto multipleRequest(mRequestFailedByHandle.size() > 1);

            if (requestType == mega::MegaRequest::TYPE_MOVE)
            {
                if (multipleRequest)
                {
                    if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                        mMovedItemsType.testFlag(MovedItemsType::BOTH))
                    {
                        msgInfo.text = tr("Error moving items");
                        msgInfo.informativeText =
                            tr("The items couldn´t be moved. Try again later");
                    }
                    else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                    {
                        msgInfo.text = tr("Error moving files");
                        msgInfo.informativeText =
                            tr("The files couldn´t be moved. Try again later");
                    }
                    else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                    {
                        msgInfo.text = tr("Error moving folders");
                        msgInfo.informativeText =
                            tr("The folders couldn´t be moved. Try again later");
                    }
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(
                        mRequestFailedByHandle.firstKey()));

                    if (node->isFile())
                    {
                        msgInfo.text = tr("Error moving file");
                        msgInfo.informativeText =
                            tr("The file %1 couldn´t be moved. Try again later")
                                .arg(MegaNodeNames::getNodeName(node.get()));
                    }
                    else
                    {
                        msgInfo.text = tr("Error moving folder");
                        msgInfo.informativeText =
                            tr("The folder %1 couldn´t be moved. Try again later")
                                .arg(MegaNodeNames::getNodeName(node.get()));
                    }
                }
            }
            else if (requestType == mega::MegaRequest::TYPE_REMOVE)
            {
                if (multipleRequest)
                {
                    if (mMovedItemsType.testFlag(MovedItemsType::NONE) ||
                        mMovedItemsType.testFlag(MovedItemsType::BOTH))
                    {
                        msgInfo.text = tr("Error removing items");
                        msgInfo.informativeText =
                            tr("The items couldn´t be removed. Try again later");
                    }
                    else if (mMovedItemsType.testFlag(MovedItemsType::FILES))
                    {
                        msgInfo.text = tr("Error removing files");
                        msgInfo.informativeText =
                            tr("The files couldn´t be removed. Try again later");
                    }
                    else if (mMovedItemsType.testFlag(MovedItemsType::FOLDERS))
                    {
                        msgInfo.text = tr("Error removing folders");
                        msgInfo.informativeText =
                            tr("The folders couldn´t be removed. Try again later");
                    }
                }
                else
                {
                    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(
                        mRequestFailedByHandle.firstKey()));

                    if (node->isFile())
                    {
                        msgInfo.text = tr("Error removing file");
                        msgInfo.informativeText =
                            tr("The file %1 couldn’t be removed. Try again later")
                                .arg(MegaNodeNames::getNodeName(node.get()));
                    }
                    else
                    {
                        msgInfo.text = tr("Error removing folder");
                        msgInfo.informativeText =
                            tr("The folder %1 couldn’t be removed. Try again later")
                                .arg(MegaNodeNames::getNodeName(node.get()));
                    }
                }
            }

            // Show dialog
            emit showMessageBox(msgInfo);

            // Reset value
            mRequestFailedByHandle.clear();
        }

        // Reset values for next move action
        mMovedItemsType = MovedItemsType::NONE;
        emit allNodeRequestsFinished();
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
        default:
            break;
    }
    return QVariant();
}

QList<QPair<mega::MegaHandle, QModelIndex>>& NodeSelectorModel::needsToBeExpanded()
{
    return mIndexesToBeExpanded;
}

QList<QPair<mega::MegaHandle, QModelIndex>>& NodeSelectorModel::needsToBeSelected()
{
    return mIndexesToBeSelected;
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

    mIndexesToBeSelected.append(qMakePair(node->getHandle(), QModelIndex()));

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

QModelIndex NodeSelectorModel::getIndexFromHandle(const mega::MegaHandle& handle,
                                                  const QModelIndex& parent)
{
    std::shared_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(handle));
    return getIndexFromNode(node, parent);
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
    executeExtraSpaceLogic();

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

void NodeSelectorModel::setCurrentRootIndex(const QModelIndex& rootIndex)
{
    mPreviousRootIndex = mCurrentRootIndex;
    mCurrentRootIndex = rootIndex.isValid() ? rootIndex : index(0, 0);

    mRowAdded = false;
    mRowRemoved = false;

    NodeSelectorModelItem* item =
        static_cast<NodeSelectorModelItem*>(mCurrentRootIndex.internalPointer());
    if (item && item->areChildrenInitialized())
    {
        executeExtraSpaceLogic();
    }
}

QModelIndex NodeSelectorModel::rootIndex(const QModelIndex& visualRootIndex) const
{
    if (!visualRootIndex.isValid())
    {
        return index(0, 0);
    }

    return visualRootIndex;
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
        auto lastColumnIndex = index(indexToUpdate.row(), columnCount()-1, indexToUpdate.parent());
        emit dataChanged(indexToUpdate, lastColumnIndex);
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
