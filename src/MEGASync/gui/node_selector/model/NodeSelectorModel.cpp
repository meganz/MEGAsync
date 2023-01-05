#include "node_selector/model/NodeSelectorModel.h"
#include "node_selector/model/NodeSelectorModelSpecialised.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Preferences.h"
#include "syncs/control/SyncInfo.h"
#include "UserAttributesRequests/CameraUploadFolder.h"
#include "UserAttributesRequests/MyChatFilesFolder.h"
#include "UserAttributesRequests/MyBackupsHandle.h"
#include "MegaNodeNames.h"

#include "mega/types.h"

#include <QApplication>
#include <QToolTip>

const char* INDEX_PROPERTY = "INDEX";

NodeRequester::NodeRequester(NodeSelectorModel *model)
    : mModel(model),
      mCancelToken(mega::MegaCancelToken::createInstance())
{}

void NodeRequester::lockMutex(bool state) const
{
    state ? mMutex.lock() : mMutex.unlock();
}

void NodeRequester::requestNodeAndCreateChildren(NodeSelectorModelItem* item, const QModelIndex& parentIndex, bool showFiles)
{
    if(item)
    {
        auto node = item->getNode();
        item->setProperty(INDEX_PROPERTY, parentIndex);
        qDebug()<<item->getNode()->getName();
        if(!item->requestingChildren() && !item->areChildrenInitialized())
        {
            item->setRequestingChildren(true);
            mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

            auto childNodesFiltered = mega::MegaNodeList::createInstance();
            showFiles ?
                childNodesFiltered = megaApi->getChildren(node.get(), mega::MegaApi::ORDER_NONE, mCancelToken.get())
                    : childNodesFiltered = megaApi->getChildrenFromType(item->getNode().get(), mega::MegaNode::TYPE_FOLDER, mega::MegaApi::ORDER_NONE, mCancelToken.get());

            if(!isAborted())
            {
                lockMutex(true);
                item->createChildItems(std::unique_ptr<mega::MegaNodeList>(childNodesFiltered));
                lockMutex(false);
                emit nodesReady(item);
            }
            else
            {
                delete childNodesFiltered;
            }
        }
    }
}

void NodeRequester::search(const QString &text)
{
    if(text.isEmpty())
    {
        return;
    }
    qDeleteAll(mRootItems);
    mRootItems.clear();
    mSearchCanceled = false;
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    auto nodeList = std::unique_ptr<mega::MegaNodeList>(megaApi->search(text.toUtf8(), mCancelToken.get()));
    QList<NodeSelectorModelItem*> items;

    for(int i = 0; i < nodeList->size(); i++)
    {
        if(isAborted() || mSearchCanceled)
        {
            break;
        }
        if(megaApi->isInRubbish(nodeList->get(i)))
        {
            continue;
        }
        auto node = std::unique_ptr<mega::MegaNode>(nodeList->get(i)->copy());
        if(node->isFolder() || (node->isFile() && mShowFiles))
        {
            auto item = new NodeSelectorModelItemSearch(std::move(node));
            items.append(item);
        }
    }

    if(isAborted() || mSearchCanceled)
    {
        qDeleteAll(items);
    }
    else
    {
        mRootItems.append(items);
        emit searchItemsCreated(items);
    }

}

void NodeRequester::createCloudDriveRootItem()
{
    auto root = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getRootNode());

    if(!isAborted())
    {
        auto item = new NodeSelectorModelItemCloudDrive(std::move(root), mShowFiles);
        mRootItems.append(item);
        emit megaCloudDriveRootItemCreated(item);
    }
}

void NodeRequester::createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList)
{
    QList<NodeSelectorModelItem*> items;
    for(int i = 0; i < nodeList->size(); i++)
    {
        if(isAborted())
        {
            break;
        }

        auto node = std::unique_ptr<mega::MegaNode>(nodeList->get(i)->copy());
        auto user = std::unique_ptr<mega::MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(node.get()));
        NodeSelectorModelItem* item = new NodeSelectorModelItemIncomingShare(std::move(node), mShowFiles);

        items.append(item);

        auto incomingSharesModel = dynamic_cast<NodeSelectorModelIncomingShares*>(mModel);
        if(incomingSharesModel)
        {
            item->setProperty(INDEX_PROPERTY, incomingSharesModel->index(0,i));
            connect(item, &NodeSelectorModelItem::infoUpdated, incomingSharesModel, &NodeSelectorModelIncomingShares::onItemInfoUpdated);
            item->setOwner(move(user));
        }
    }

    if(isAborted())
    {
        qDeleteAll(items);
    }
    else
    {
        mRootItems.append(items);
        emit megaIncomingSharesRootItemsCreated(items);
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
                //Here we are setting my backups node as vault node in the item, it is not the same vault node that we get
                //doing megaapi->getVaultNode(), we have to hide it here thats why are doing this trick.
                //The real vault is the parent of my backups folder
                //NodeSelectorModelItem* item = new NodeSelectorModelItem(std::move(backupsNode), mShowFiles);
                //item->setAsVaultNode();
                mRootItems.append(item);
                emit megaBackupRootItemsCreated(item);
            }
        }
    }
    else
    {
        emit megaBackupRootItemsCreated(nullptr);
    }
}

void NodeRequester::onAddNodeRequested(std::shared_ptr<mega::MegaNode> newNode, const QModelIndex& parentIndex, NodeSelectorModelItem *parentItem)
{
    lockMutex(true);
    auto childItem = parentItem->addNode(newNode);
    lockMutex(false);
    childItem->setProperty(INDEX_PROPERTY, mModel->index(parentItem->getNumChildren() -1 ,0, parentIndex));

    if(!isAborted())
    {
        emit nodeAdded(childItem);
    }
    else
    {
        removeItem(childItem);
    }
}

void NodeRequester::removeItem(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mMutex);
    item->deleteLater();
}

void NodeRequester::removeRootItem(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mMutex);
    item->deleteLater();
    mRootItems.removeOne(item);
}

int NodeRequester::rootIndexSize() const
{
    QMutexLocker lock(&mMutex);
    return mRootItems.size();
}

int NodeRequester::rootIndexOf(NodeSelectorModelItem* item)
{
    QMutexLocker lock(&mMutex);
    return mRootItems.indexOf(item);
}

NodeSelectorModelItem *NodeRequester::getRootItem(int index) const
{
    QMutexLocker lock(&mMutex);
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
        mCancelToken->cancel();
    }
}

void NodeRequester::finishWorker()
{
    qDeleteAll(mRootItems);
    connect(thread(), &QThread::finished, thread(), [this]()
    {
        thread()->deleteLater();
        deleteLater();
    });
    thread()->quit();
}

bool NodeRequester::isAborted()
{
    return mAborted || (mCancelToken && mCancelToken->isCancelled());
}

void NodeRequester::setShowFiles(bool newShowFiles)
{
    mShowFiles = newShowFiles;
}

void NodeRequester::abort()
{
    mAborted = true;
    finishWorker();
}

/* ------------------- MODEL ------------------------- */

const int NodeSelectorModel::ROW_HEIGHT = 24;

NodeSelectorModel::NodeSelectorModel(QObject *parent) :
    QAbstractItemModel(parent),
    mRequiredRights(mega::MegaShare::ACCESS_READ),
    mDisplayFiles(false),
    mSyncSetupMode(false),
    mShowFiles(true)
{
    mCameraFolderAttribute = UserAttributes::CameraUploadFolder::requestCameraUploadFolder();
    mMyChatFilesFolderAttribute = UserAttributes::MyChatFilesFolder::requestMyChatFilesFolder();

    mNodeRequesterThread = new QThread();
    mNodeRequesterWorker = new NodeRequester(this);
    mNodeRequesterWorker->moveToThread(mNodeRequesterThread);
    mNodeRequesterThread->start();

    connect(this, &NodeSelectorModel::requestChildNodes, mNodeRequesterWorker, &NodeRequester::requestNodeAndCreateChildren, Qt::QueuedConnection);
    connect(this, &NodeSelectorModel::requestAddNode, mNodeRequesterWorker, &NodeRequester::onAddNodeRequested, Qt::QueuedConnection);
    connect(this, &NodeSelectorModel::removeItem, mNodeRequesterWorker, &NodeRequester::removeItem);
    connect(this, &NodeSelectorModel::removeRootItem, mNodeRequesterWorker, &NodeRequester::removeRootItem);
    connect(this, &NodeSelectorModel::deleteWorker, mNodeRequesterWorker, &NodeRequester::abort);

    connect(mNodeRequesterWorker, &NodeRequester::nodesReady, this, &NodeSelectorModel::onChildNodesReady, Qt::QueuedConnection);
    connect(mNodeRequesterWorker, &NodeRequester::nodeAdded, this, &NodeSelectorModel::onNodeAdded, Qt::QueuedConnection);

    qRegisterMetaType<std::shared_ptr<mega::MegaNodeList>>("std::shared_ptr<mega::MegaNodeList>");
    qRegisterMetaType<std::shared_ptr<mega::MegaNode>>("std::shared_ptr<mega::MegaNode>");
    qRegisterMetaType<mega::MegaHandle>("mega::MegaHandle");
}

NodeSelectorModel::~NodeSelectorModel()
{
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
                        return item->getOwnerName();
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
                case toInt(NodeSelectorModelRoles::STATUS_ROLE):
                {
                    return QVariant::fromValue(item->getStatus());
                }
                case toInt(NodeRowDelegateRoles::ENABLED_ROLE):
                {
                    if(mSyncSetupMode)
                    {
                        return item->isSyncable();
                    }
                    return true;
                }
                case toInt(NodeRowDelegateRoles::INDENT_ROLE):
                {
                    return item->isCloudDrive() || item->isVault()? -10 : 0;
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

QModelIndex NodeSelectorModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;

    if (hasIndex(row, column, parent))
    {
        if (parent.isValid())
        {
            mNodeRequesterWorker->lockMutex(true);
            NodeSelectorModelItem* item(static_cast<NodeSelectorModelItem*>(parent.internalPointer()));
            if(item)
            {
                index =  createIndex(row, column, item->getChild(row).data());
            }
            mNodeRequesterWorker->lockMutex(false);
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
    if (parent.isValid())
    {
        mNodeRequesterWorker->lockMutex(true);
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
        auto rows = item ? item->getNumChildren() : 0;
        mNodeRequesterWorker->lockMutex(false);
        return rows;
    }
    return mNodeRequesterWorker->rootIndexSize();
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
        mNodeRequesterWorker->lockMutex(true);
        auto numChild = item->getNumChildren() > 0;
        mNodeRequesterWorker->lockMutex(false);
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
        else if(role == Qt::DecorationRole)
        {
            if(section == STATUS)
            {
                return QIcon(QLatin1String("://images/node_selector/icon-small-MEGA.png"));
            }
            else if(section == USER)
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
}

void NodeSelectorModel::addNode(std::shared_ptr<mega::MegaNode> node, const QModelIndex &parent)
{
    NodeSelectorModelItem* parentItem = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    if(parentItem)
    {
        clearIndexesNodeInfo();

        mNodeRequesterWorker->lockMutex(true);
        int numchildren = parentItem->getNumChildren();
        mNodeRequesterWorker->lockMutex(false);

        beginInsertRows(parent, numchildren, numchildren);
        emit requestAddNode(node, parent, parentItem);
    }
}

void NodeSelectorModel::onNodeAdded(NodeSelectorModelItem* childItem)
{
    endInsertRows();

    auto index = childItem->property(INDEX_PROPERTY).toModelIndex();
    mIndexesActionInfo.indexesToBeExpanded.append(index);

    mIndexesActionInfo.needsToBeSelected = true;
    mIndexesActionInfo.needsToBeEntered = true;
    emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded);
}

bool NodeSelectorModel::addToLoadingList(const std::shared_ptr<mega::MegaNode> node)
{
    return node != nullptr;
}


void NodeSelectorModel::removeNode(const QModelIndex &index)
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
                mNodeRequesterWorker->lockMutex(true);
                auto itemToRemove = parent->findChildNode(node);
                mNodeRequesterWorker->lockMutex(false);
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

void NodeSelectorModel::showFiles(bool show)
{
    mShowFiles = show;
    mNodeRequesterWorker->setShowFiles(mShowFiles);
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
                return MegaNodeNames::getNodeName(item->getNode()->getName());
            }

            QString nodeName = QString::fromUtf8(item->getNode()->getName());

            if(nodeName == QLatin1String("NO_KEY") || nodeName == QLatin1String("CRYPTO_ERROR"))
            {
                nodeName = QCoreApplication::translate("MegaError", "Decryption error");
            }

            return QVariant(nodeName);
        }
        case COLUMN::DATE:
        {
            if(item->isCloudDrive() || item->isVault())
            {
                return QVariant();
            }

            const QString language = MegaSyncApp->getCurrentLanguageCode();
            QLocale locale(language);
            QDateTime dateTime = dateTime.fromSecsSinceEpoch(item->getNode()->getCreationTime());
            QDateTime currentDate = currentDate.currentDateTime();
            QLatin1String dateFormat ("dd MMM yyyy");
            QString timeFormat = locale.timeFormat(QLocale::ShortFormat);

            int hours = dateTime.time().hour();

            if(currentDate.toString(dateFormat)
                    == dateTime.toString(dateFormat))
            {
                return tr("Today at %1", "", hours).arg(locale.toString(dateTime, timeFormat));
            }

            currentDate = currentDate.addDays(-1); //for checking if it was yesterday

            if(currentDate.toString(dateFormat)
                    == dateTime.toString(dateFormat))
            {
                return tr("Yesterday at %1", "", hours).arg(locale.toString(dateTime, timeFormat));
            }
            //First: day Second: hour. This is done for allow translators to change the order
            //in case there are any language that needs to put in another order.
            return tr("%1 at %2", "", hours).arg(locale.toString(dateTime, dateFormat), locale.toString(dateTime, timeFormat));
        }
        default:
            break;
    }
    return QVariant();
}

NodeSelectorModel::IndexesActionInfo NodeSelectorModel::needsToBeExpandedAndSelected()
{
    IndexesActionInfo info = mIndexesActionInfo;
    clearIndexesNodeInfo();

    return info;
}

void NodeSelectorModel::clearIndexesNodeInfo()
{
    mIndexesActionInfo = IndexesActionInfo();
}

void NodeSelectorModel::abort()
{
    mNodeRequesterWorker->cancelCurrentRequest();
    emit deleteWorker();
}

bool NodeSelectorModel::canBeDeleted() const
{
    return true;
}

void NodeSelectorModel::loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node)
{
    //First, we set the loading view as it can take long to load the tree path to the node
    emit blockUi(true);

    clearIndexesNodeInfo();
    mIndexesActionInfo.needsToBeSelected = true;

    mNodesToLoad.clear();
    mNodesToLoad.append(node);

    auto p_node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getParentNode(node.get()));

    //The vault node is not represented in the node selector, hence if the parent of a node is the vault
    //it doesn´t have to be added to the node list to load. If it is added the loading of a specific node
    //will stops working in backups screen.
    while(addToLoadingList(p_node))
    {
        mNodesToLoad.append(p_node);
        p_node.reset(MegaSyncApp->getMegaApi()->getParentNode(p_node.get()));
    }

    if(!fetchMoreRecursively(QModelIndex()))
    {
        emit blockUi(false);
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
            if(canFetchMore(indexToCheck))
            {
                fetchMore(indexToCheck);
                result = true;
            }
            else
            {
                mIndexesActionInfo.indexesToBeExpanded.append(indexToCheck);
                result = continueWithNextItemToLoad(indexToCheck);
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

void NodeSelectorModel::rootItemsLoaded()
{
    blockSignals(true);
    endResetModel();
    blockSignals(false);
}

void NodeSelectorModel::addRootItems()
{
    emit blockUi(true);
    beginResetModel();
    createRootNodes();
}

void NodeSelectorModel::loadLevelFinished()
{
   emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded);
}

bool NodeSelectorModel::canFetchMore(const QModelIndex &parent) const
{
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

void NodeSelectorModel::fetchItemChildren(const QModelIndex& parent)
{
    emit blockUi(true);

    NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(parent.internalPointer());
    qDebug()<<"Fetching item children:"<<item->getNode()->getName();
    if(!item->areChildrenInitialized() && !item->requestingChildren())
    {
        int itemNumChildren = item->getNumChildren();
        if(itemNumChildren > 0)
        {
            blockSignals(true);
            beginInsertRows(parent, 0, itemNumChildren-1);
            blockSignals(false);
            emit requestChildNodes(item, parent, mShowFiles);
        }
        else
        {
            emit blockUi(false);
        }
    }
    else
    {
        emit blockUi(false);
    }
}

void NodeSelectorModel::onChildNodesReady(NodeSelectorModelItem* parent)
{
    auto index = parent->property(INDEX_PROPERTY).value<QModelIndex>();
    mIndexesActionInfo.indexesToBeExpanded.append(index);
    continueWithNextItemToLoad(index);
}

bool NodeSelectorModel::continueWithNextItemToLoad(const QModelIndex& parentIndex)
{
    bool result = false;

    if(!mNodesToLoad.isEmpty())
    {
        //The last one has been already processed
        mNodesToLoad.removeLast();
        if(!mNodesToLoad.isEmpty())
        {
            result = fetchMoreRecursively(parentIndex);
            if(!result && !mNodesToLoad.isEmpty())
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

QModelIndex NodeSelectorModel::findItemByNodeHandle(const mega::MegaHandle& handle, const QModelIndex &parent)
{
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex idx = index(i, COLUMN::NODE, parent);
        if(idx.isValid())
        {
            if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
            {
                if(chkItem->getNode()->isFolder() && chkItem->getNode()->getHandle() == handle)
                {
                    return idx;
                }
            }
        }
    }
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex child = parent.child(i, COLUMN::NODE);
        if(child.isValid())
        {
          auto ret = findItemByNodeHandle(handle, child);
          if(ret.isValid())
          {
              return ret;
          }
        }
    }

    return QModelIndex();
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
                    icon.addFile(QLatin1String("://images/icons/folder/small-folder-outgoing_disabled.png"), QSize(), QIcon::Disabled);
                    return icon;
                }
                else if(node->getHandle() == MegaSyncApp->getRootNode()->getHandle())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/ico-cloud-drive.png"));
                    return icon;
                }
                else if(item->isVault())
                {
                    QIcon icon;
                    icon.addFile(QLatin1String("://images/node_selector/Backups_small_ico.png"));
                    return icon;
                }
                else
                {
                    QString nodeDeviceId (QString::fromUtf8(node->getDeviceId()));
                    if (!nodeDeviceId.isEmpty())
                    {
                        std::unique_ptr<mega::MegaNode> parent (MegaSyncApp->getMegaApi()->getNodeByHandle(node->getParentHandle()));
                        if (parent)
                        {
                            QString parentDeviceId (QString::fromUtf8(parent->getDeviceId()));
                            if (parentDeviceId.isEmpty())
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
                        }
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
