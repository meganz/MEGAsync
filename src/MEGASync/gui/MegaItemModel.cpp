#include "MegaItemModel.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Preferences.h"
#include "syncs/control/SyncInfo.h"
#include "UserAttributesRequests/CameraUploadFolder.h"
#include "UserAttributesRequests/MyChatFilesFolder.h"
#include "UserAttributesRequests/MyBackupsHandle.h"

#include "mega/types.h"

#include <QApplication>
#include <QToolTip>

using namespace mega;

const char* INDEX_PROPERTY = "INDEX";

NodeRequester::NodeRequester(MegaItemModel *model)
    : mModel(model),
      mCancelToken(MegaCancelToken::createInstance())
{}

void NodeRequester::lockMutex(bool state) const
{
    state ? mMutex.lock() : mMutex.unlock();
}

void NodeRequester::requestNodeAndCreateChildren(MegaItem* item, const QModelIndex& parentIndex, bool showFiles)
{
    if(item)
    {
        auto node = item->getNode();
        item->setProperty(INDEX_PROPERTY, parentIndex);

        if(!item->requestingChildren() && !item->childrenAreInit())
        {
            item->setRequestingChildren(true);
            MegaApi* megaApi = MegaSyncApp->getMegaApi();

            auto childNodesFiltered = MegaNodeList::createInstance();
            showFiles ?
                childNodesFiltered = megaApi->getChildren(node.get(), MegaApi::ORDER_NONE, mCancelToken.get())
                    : childNodesFiltered = megaApi->getChildrenFromType(item->getNode().get(), MegaNode::TYPE_FOLDER, MegaApi::ORDER_NONE, mCancelToken.get());

            if(!isAborted())
            {
                lockMutex(true);
                item->createChildItems(std::unique_ptr<mega::MegaNodeList>(childNodesFiltered));
                lockMutex(false);
                emit nodesReady(item);
            }
        }
    }
}

void NodeRequester::createCloudDriveRootItem()
{
    auto root = std::unique_ptr<MegaNode>(MegaSyncApp->getMegaApi()->getRootNode());

    if(!isAborted())
    {
        auto item = new MegaItem(std::move(root), mShowFiles);
        mRootItems.append(item);
        emit megaCloudDriveRootItemCreated(item);
    }
}

void NodeRequester::createIncomingSharesRootItems(std::shared_ptr<mega::MegaNodeList> nodeList)
{
    QList<MegaItem*> items;
    for(int i = 0; i < nodeList->size(); i++)
    {
        if(isAborted())
        {
            break;
        }

        auto node = nodeList->get(i)->copy();
        auto user = std::unique_ptr<MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(node));
        MegaItem* item = new MegaItem(std::unique_ptr<MegaNode>(node), mShowFiles);
        items.append(item);

        auto incomingSharesModel = dynamic_cast<MegaItemModelIncomingShares*>(mModel);
        if(incomingSharesModel)
        {
            item->setProperty(INDEX_PROPERTY, incomingSharesModel->index(0,i));
            connect(item, &MegaItem::infoUpdated, incomingSharesModel, &MegaItemModelIncomingShares::onItemInfoUpdated);
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
    if (backupsHandle != INVALID_HANDLE)
    {
        std::unique_ptr<mega::MegaNode> backupsNode(MegaSyncApp->getMegaApi()->getNodeByHandle(backupsHandle));
        if(backupsNode)
        {
            if(!isAborted())
            {
                MegaItem* item = new MegaItem(std::move(backupsNode), mShowFiles);
                item->setAsVaultNode();
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

void NodeRequester::onAddNodeRequested(std::shared_ptr<MegaNode> newNode, const QModelIndex& parentIndex, MegaItem *parentItem)
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

void NodeRequester::removeItem(MegaItem* item)
{
    QMutexLocker lock(&mMutex);
    item->deleteLater();
}

void NodeRequester::removeRootItem(MegaItem* item)
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

int NodeRequester::rootIndexOf(MegaItem* item)
{
    QMutexLocker lock(&mMutex);
    return mRootItems.indexOf(item);
}

MegaItem *NodeRequester::getRootItem(int index) const
{
    QMutexLocker lock(&mMutex);
    return mRootItems.at(index);
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

const int MegaItemModel::ROW_HEIGHT = 20;

MegaItemModel::MegaItemModel(QObject *parent) :
    QAbstractItemModel(parent),
    mRequiredRights(MegaShare::ACCESS_READ),
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

    connect(this, &MegaItemModel::requestChildNodes, mNodeRequesterWorker, &NodeRequester::requestNodeAndCreateChildren, Qt::QueuedConnection);
    connect(this, &MegaItemModel::requestAddNode, mNodeRequesterWorker, &NodeRequester::onAddNodeRequested, Qt::QueuedConnection);
    connect(this, &MegaItemModel::removeItem, mNodeRequesterWorker, &NodeRequester::removeItem);
    connect(this, &MegaItemModel::removeRootItem, mNodeRequesterWorker, &NodeRequester::removeRootItem);
    connect(this, &MegaItemModel::deleteWorker, mNodeRequesterWorker, &NodeRequester::abort);

    connect(mNodeRequesterWorker, &NodeRequester::nodesReady, this, &MegaItemModel::onChildNodesReady, Qt::QueuedConnection);
    connect(mNodeRequesterWorker, &NodeRequester::nodeAdded, this, &MegaItemModel::onNodeAdded, Qt::QueuedConnection);

    qRegisterMetaType<std::shared_ptr<mega::MegaNodeList>>("std::shared_ptr<mega::MegaNodeList>");
    qRegisterMetaType<std::shared_ptr<mega::MegaNode>>("std::shared_ptr<mega::MegaNode>");
    qRegisterMetaType<mega::MegaHandle>("mega::MegaHandle");
}

MegaItemModel::~MegaItemModel()
{
}

int MegaItemModel::columnCount(const QModelIndex &) const
{
    return last;
}

QVariant MegaItemModel::data(const QModelIndex &index, int role) const
{
    if (index.isValid())
    {
        MegaItem* item = static_cast<MegaItem*>(index.internalPointer());
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
                        if((item->getStatus() == MegaItem::SYNC)
                                || (item->getStatus() == MegaItem::SYNC_CHILD))
                        {
                            return tr("Folder already synced");
                        }
                        else if(item->getStatus() == MegaItem::SYNC_PARENT)
                        {
                            return tr("Folder contents already synced");
                        }
                        QToolTip::hideText();
                    }
                    break;
                }
                case toInt(MegaItemModelRoles::DATE_ROLE):
                {
                    return QVariant::fromValue(item->getNode()->getCreationTime());
                }
                case toInt(MegaItemModelRoles::IS_FILE_ROLE):
                {
                    return QVariant::fromValue(item->getNode()->isFile());
                }
                case toInt(MegaItemModelRoles::STATUS_ROLE):
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
                    return item->isRoot() || item->isVault()? -10 : 0;
                }
                case toInt(NodeRowDelegateRoles::INIT_ROLE):
                {
                    return item->childrenAreInit();
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

QModelIndex MegaItemModel::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;

    if (hasIndex(row, column, parent))
    {
        if (parent.isValid())
        {
            mNodeRequesterWorker->lockMutex(true);
            MegaItem* item(static_cast<MegaItem*>(parent.internalPointer()));
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

QModelIndex MegaItemModel::parent(const QModelIndex &index) const
{
    QModelIndex parentIndex;

    if(index.isValid())
    {
        MegaItem* item = static_cast<MegaItem*>(index.internalPointer());
        if(item)
        {
            MegaItem* parent = item->getParent();
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

int MegaItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        mNodeRequesterWorker->lockMutex(true);
        MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
        auto rows = item ? item->getNumChildren() : 0;
        mNodeRequesterWorker->lockMutex(false);
        return rows;
    }
    return mNodeRequesterWorker->rootIndexSize();
}

bool MegaItemModel::hasChildren(const QModelIndex &parent) const
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

    MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
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

QVariant MegaItemModel::headerData(int section, Qt::Orientation orientation, int role) const
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

void MegaItemModel::setSyncSetupMode(bool value)
{
    mSyncSetupMode = value;
}

void MegaItemModel::addNode(std::shared_ptr<MegaNode> node, const QModelIndex &parent)
{
    MegaItem* parentItem = static_cast<MegaItem*>(parent.internalPointer());
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

void MegaItemModel::onNodeAdded(MegaItem* childItem)
{
    endInsertRows();

    auto index = childItem->property(INDEX_PROPERTY).toModelIndex();
    mIndexesActionInfo.indexesToBeExpanded.append(index);

    mIndexesActionInfo.needsToBeSelected = true;
    mIndexesActionInfo.needsToBeEntered = true;
    emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded);
}


void MegaItemModel::removeNode(const QModelIndex &index)
{
    if(!index.isValid())
    {
        return;
    }
    auto item = static_cast<MegaItem*>(index.internalPointer());
    if(item)
    {
        std::shared_ptr<MegaNode> node = item->getNode();
        if (node)
        {
            MegaItem* parent = static_cast<MegaItem*>(index.parent().internalPointer());
            if(parent)
            {
                auto item = static_cast<MegaItem*>(index.internalPointer());
                if(item)
                {
                    int row = parent->indexOf(item);
                    beginRemoveRows(index.parent(), row, row);
                    mNodeRequesterWorker->lockMutex(true);
                    auto itemToRemove = parent->findChildNode(node);
                    mNodeRequesterWorker->lockMutex(false);
                    emit removeItem(itemToRemove);
                    endRemoveRows();
                }
            }
            else
            {
                int row = index.row();
                auto item(static_cast<MegaItem*>(index.internalPointer()));
                if(item)
                {
                    beginRemoveRows(index.parent(), row, row);
                    emit removeRootItem(item);
                    endRemoveRows();
                }
            }
        }
    }
}

void MegaItemModel::showFiles(bool show)
{
    mShowFiles = show;
    mNodeRequesterWorker->setShowFiles(mShowFiles);
}

QVariant MegaItemModel::getIcon(const QModelIndex &index, MegaItem* item) const
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

QVariant MegaItemModel::getText(const QModelIndex &index, MegaItem *item) const
{
    switch(index.column())
    {
        case COLUMN::NODE:
        {
            if(item->isVault())
            {
                return QCoreApplication::translate("MegaNodeNames", UserAttributes::MyBackupsHandle::DEFAULT_BACKUPS_ROOT_DIRNAME);
            }
            else if(item->isRoot())
            {
                return QApplication::translate("MegaNodeNames", item->getNode()->getName());
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
            if(item->isRoot() || item->isVault())
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
    return QVariant(QLatin1String(""));
}

MegaItemModel::IndexesActionInfo MegaItemModel::needsToBeExpandedAndSelected()
{
    IndexesActionInfo info = mIndexesActionInfo;
    clearIndexesNodeInfo();

    return info;
}

void MegaItemModel::clearIndexesNodeInfo()
{
    mIndexesActionInfo = IndexesActionInfo();
}

void MegaItemModel::abort()
{
    mNodeRequesterWorker->cancelCurrentRequest();
    emit deleteWorker();
}

bool MegaItemModel::canBeDeleted() const
{
    return true;
}

int MegaItemModel::insertPosition(const std::unique_ptr<MegaNode>& node)
{
    int type = node->getType();

    for (auto i = 0; i < mNodeRequesterWorker->rootIndexSize(); ++i)
    {
        std::shared_ptr<MegaNode> otherNode = mNodeRequesterWorker->getRootItem(i)->getNode();
        int otherNodeType = otherNode->getType();
        if (type >= otherNodeType && qstricmp(node->getName(), otherNode->getName()) <= 0)
        {
            return i;
        }
    }

    return -1;
}

void MegaItemModel::loadTreeFromNode(const std::shared_ptr<mega::MegaNode> node)
{
    //First, we se the loading view as it can take long to load the tree path to the node
    emit blockUi(true);

    clearIndexesNodeInfo();
    mIndexesActionInfo.needsToBeSelected = true;

    mNodesToLoad.clear();
    mNodesToLoad.append(node);

    auto p_node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getParentNode(node.get()));
    while(p_node)
    {
        mNodesToLoad.append(p_node);
        p_node.reset(MegaSyncApp->getMegaApi()->getParentNode(p_node.get()));
    }

    if(!fetchMoreRecursively(QModelIndex()))
    {
        emit blockUi(false);
        mNodesToLoad.clear();
        clearIndexesNodeInfo();
    }
}

bool MegaItemModel::fetchMoreRecursively(const QModelIndex& parentIndex)
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
                continueWithNextItemToLoad(indexToCheck);
            }
        }
    }

    return result;
}

QModelIndex MegaItemModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> node, const QModelIndex &parent)
{
    if(node)
    {
        auto childrenCount = rowCount(parent);
        for(int row = 0; row < childrenCount; ++row)
        {
            auto indexToCheck = index(row,0,parent);
            MegaItem* item = static_cast<MegaItem*>(indexToCheck.internalPointer());
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

void MegaItemModel::rootItemsLoaded()
{
    blockSignals(true);
    endResetModel();
    blockSignals(false);
}

void MegaItemModel::addRootItems()
{
    beginResetModel();
    createRootNodes();
}

void MegaItemModel::loadLevelFinished()
{
   emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded);
}

bool MegaItemModel::canFetchMore(const QModelIndex &parent) const
{
    MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
    if(item)
    {
        return item->canFetchMore();
    }
    else
    {
        return mNodeRequesterWorker->rootIndexSize() < rootItemsCount();
    }
}

void MegaItemModel::fetchItemChildren(const QModelIndex& parent)
{
    emit blockUi(true);

    MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());

    if(!item->childrenAreInit() && !item->requestingChildren())
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

void MegaItemModel::onChildNodesReady(MegaItem* parent)
{
    auto index = parent->property(INDEX_PROPERTY).value<QModelIndex>();
    mIndexesActionInfo.indexesToBeExpanded.append(index);
    continueWithNextItemToLoad(index);
}

void MegaItemModel::continueWithNextItemToLoad(const QModelIndex& parentIndex)
{
    if(!mNodesToLoad.isEmpty())
    {
        //The last one has been already processed
        mNodesToLoad.removeLast();
        if(!mNodesToLoad.isEmpty())
        {
            if(!fetchMoreRecursively(parentIndex) && !mNodesToLoad.isEmpty())
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
}

QModelIndex MegaItemModel::findItemByNodeHandle(const mega::MegaHandle& handle, const QModelIndex &parent)
{
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex idx = index(i, COLUMN::NODE, parent);
        if(idx.isValid())
        {
            if(MegaItem* chkItem = static_cast<MegaItem*>(idx.internalPointer()))
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

QIcon MegaItemModel::getFolderIcon(MegaItem *item) const
{
    if(item)
    {
        auto node = item->getNode();

        if(node)
        {
            if (node->getType() >= MegaNode::TYPE_FOLDER)
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
MegaItemModelCloudDrive::MegaItemModelCloudDrive(QObject *parent)
    : MegaItemModel(parent)
{
}

void MegaItemModelCloudDrive::createRootNodes()
{
    emit requestCloudDriveRootCreation();
}

int MegaItemModelCloudDrive::rootItemsCount() const
{
    return 1;
}

void MegaItemModelCloudDrive::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void MegaItemModelCloudDrive::firstLoad()
{
    connect(this, &MegaItemModelCloudDrive::requestCloudDriveRootCreation, mNodeRequesterWorker, &NodeRequester::createCloudDriveRootItem);
    connect(mNodeRequesterWorker, &NodeRequester::megaCloudDriveRootItemCreated, this, &MegaItemModelCloudDrive::onRootItemCreated, Qt::QueuedConnection);

    addRootItems();
}

void MegaItemModelCloudDrive::onRootItemCreated(MegaItem *item)
{
    rootItemsLoaded();

    //Add the item of the Cloud Drive
    auto rootIndex(index(0,0));
    if(canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
MegaItemModelIncomingShares::MegaItemModelIncomingShares(QObject *parent)
    : MegaItemModel(parent)
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    mSharedNodeList = std::unique_ptr<MegaNodeList>(megaApi->getInShares());
}

void MegaItemModelIncomingShares::onItemInfoUpdated(int role)
{
    if(MegaItem* item = static_cast<MegaItem*>(sender()))
    {
        for(int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx = index(i, COLUMN::USER); //we only update this column because we retrieve the data in async mode
            if(idx.isValid())                         //so it is possible that we doesn´t have the information from the start
            {
                if(MegaItem* chkItem = static_cast<MegaItem*>(idx.internalPointer()))
                {
                    if(chkItem == item)
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

void MegaItemModelIncomingShares::onRootItemsCreated(QList<MegaItem *> items)
{
    rootItemsLoaded();

    if(!mNodesToLoad.isEmpty())
    {
        auto index = getIndexFromNode(mNodesToLoad.last(), QModelIndex());
        if(canFetchMore(index))
        {
            fetchMore(index);
        }
    }
    else
    {
        loadLevelFinished();
    }
}

void MegaItemModelIncomingShares::createRootNodes()
{
    emit requestIncomingSharesRootCreation(mSharedNodeList);
}

int MegaItemModelIncomingShares::rootItemsCount() const
{
    return mSharedNodeList->size();
}

void MegaItemModelIncomingShares::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void MegaItemModelIncomingShares::firstLoad()
{
    connect(this, &MegaItemModelIncomingShares::requestIncomingSharesRootCreation, mNodeRequesterWorker, &NodeRequester::createIncomingSharesRootItems);
    connect(mNodeRequesterWorker, &NodeRequester::megaIncomingSharesRootItemsCreated, this, &MegaItemModelIncomingShares::onRootItemsCreated, Qt::QueuedConnection);

    addRootItems();
}

///////////////////////////////////////////////////////////////////////////////////////////////
MegaItemModelBackups::MegaItemModelBackups(QObject *parent)
    : MegaItemModel(parent)
{
}

MegaItemModelBackups::~MegaItemModelBackups()
{
}

void MegaItemModelBackups::createRootNodes()
{
    emit requestBackupsRootCreation(mBackupsHandle);
}

int MegaItemModelBackups::rootItemsCount() const
{
    return 1;
}

void MegaItemModelBackups::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void MegaItemModelBackups::firstLoad()
{
    connect(this, &MegaItemModelBackups::requestBackupsRootCreation, mNodeRequesterWorker, &NodeRequester::createBackupRootItems);
    connect(mNodeRequesterWorker, &NodeRequester::megaBackupRootItemsCreated, this, &MegaItemModelBackups::onRootItemCreated, Qt::QueuedConnection);

    auto backupsRequest = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mBackupsHandle = backupsRequest->getMyBackupsHandle();
    connect(backupsRequest.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &MegaItemModelBackups::onMyBackupsHandleReceived);

    if(backupsRequest->isAttributeReady())
    {
        onMyBackupsHandleReceived(backupsRequest->getMyBackupsHandle());
    }
    else
    {
        addRootItems();
    }

}

bool MegaItemModelBackups::canBeDeleted() const
{
    return false;
}

void MegaItemModelBackups::onMyBackupsHandleReceived(mega::MegaHandle handle)
{
    mBackupsHandle = handle;
    if (mBackupsHandle != INVALID_HANDLE)
    {
        addRootItems();
    }
}

void MegaItemModelBackups::onRootItemCreated(MegaItem *item)
{
    rootItemsLoaded();

    if(!item)
    {
        loadLevelFinished();
    }
    else
    {
        //Add the item of the Backups Drive
        auto rootIndex(index(0,0));
        if(canFetchMore(rootIndex))
        {
            fetchItemChildren(rootIndex);
        }
    }
}
