#include "MegaItemModel.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "model/Model.h"
#include "mega/types.h"

#include <QApplication>
#include <QToolTip>

using namespace mega;

void NodeRequester::requestNodes(MegaItem* node, bool showFiles)
{
    NodeInfo info;
    info.parent = node;
    info.showFiles = showFiles ;
    mNodesToRequest.append(info);

    processRequest();
}

void NodeRequester::processRequest()
{
    if(!mNodesToRequest.isEmpty())
    {
        auto info = mNodesToRequest.takeFirst();
        if(!info.parent->requestingChildren() && !info.parent->childrenAreInit())
        {
            info.parent->setRequestingChildren(true);

            MegaApi* megaApi = MegaSyncApp->getMegaApi();

            auto childNodesFiltered = MegaNodeList::createInstance();
            if(!info.showFiles)
            {
                auto childNodes = std::unique_ptr<MegaChildrenLists>(megaApi->getFileFolderChildren(info.parent->getNode().get()));
                childNodesFiltered = childNodes->getFolderList()->copy();

//                auto childNodes = megaApi->getChildren(info.parent->getNode().get());
//                for(int i = 0; i < childNodes->size();++i)
//                {
//                    auto childNode = childNodes->get(i);
//                    if(childNode->isFile() && !info.showFiles)
//                    {
//                        break;
//                    }
//                    childNodesFiltered->addNode(childNodes->get(i));
//                }
            }
            else
            {
                childNodesFiltered = megaApi->getChildren(info.parent->getNode().get());
            }


             //Here we could add the child directly to the MetaItem...but first we should protect the list
            emit nodesReady(info.parent, childNodesFiltered);
        }
    }
}


const int MegaItemModel::ROW_HEIGHT = 20;

MegaItemModel::MegaItemModel(QObject *parent) :
    QAbstractItemModel(parent),
    mRequiredRights(MegaShare::ACCESS_READ),
    mDisplayFiles(false),
    mSyncSetupMode(false),
    mShowFiles(true)
{
    mNodeRequesterThread = new QThread();
    mNodeRequesterWorker = new NodeRequester();
    mNodeRequesterWorker->moveToThread(mNodeRequesterThread);
    mNodeRequesterThread->start();

    connect(this, &MegaItemModel::requestChildNodes, mNodeRequesterWorker, &NodeRequester::requestNodes, Qt::QueuedConnection);
    connect(mNodeRequesterWorker, &NodeRequester::nodesReady, this, &MegaItemModel::onChildNodesReady, Qt::QueuedConnection);
}

MegaItemModel::~MegaItemModel()
{
    mNodeRequesterThread->quit();
    mNodeRequesterThread->deleteLater();
    mNodeRequesterWorker->deleteLater();
}

int MegaItemModel::columnCount(const QModelIndex &) const
{
    return last;
}

QVariant MegaItemModel::data(const QModelIndex &index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    MegaItem *item = static_cast<MegaItem*>(index.internalPointer());
    if (!item)
    {
        return QVariant();
    }

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
            return item->isRoot()? -10 : 0;
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
    return QVariant();
}

QModelIndex MegaItemModel::index(int row, int column, const QModelIndex &parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (parent.isValid())
    {
        MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
        return createIndex(row, column, item->getChild(row));
    }
    else if(mRootItems.size() > row)
    {
        return createIndex(row, column, mRootItems.at(row));
    }
    return QModelIndex();
}

QModelIndex MegaItemModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }
    MegaItem *item = static_cast<MegaItem*>(index.internalPointer());
    MegaItem *parent = item->getParent();
    if (!parent)
    {
        return QModelIndex();
    }
    if(mRootItems.contains(parent))
    {
        return createIndex(mRootItems.indexOf(parent), 0, parent);
    }
    return createIndex(parent->row(), 0, parent);
}

int MegaItemModel::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        MegaItem *item = static_cast<MegaItem*>(parent.internalPointer());
        if(!item)
        {
            return 0;
        }

        return item->getNumItemChildren();
    }
    return mRootItems.size();
}

int MegaItemModel::countTotalRows(const QModelIndex& idx)
{
    int count = 0;
    int rowCounter = rowCount(idx);
    count += rowCounter;
    for( int r = 0; r < rowCounter; ++r )
    {
        count += countTotalRows(index(r,0,idx));
    }
    return count;
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

    MegaItem *item = static_cast<MegaItem*>(parent.internalPointer());
    if(item && item->getNode())
    {
        if(!mShowFiles)
        {
            return MegaSyncApp->getMegaApi()->getNumChildFolders(item->getNode().get()) > 0;
        }
        return MegaSyncApp->getMegaApi()->hasChildren(item->getNode().get());
    }

    return QAbstractItemModel::hasChildren(parent);
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

void MegaItemModel::addRootItems()
{
    blockSignals(true);
    lockMutex(true);
    beginResetModel();
    mRootItems.append(getRootItems());
    endResetModel();
    lockMutex(false);
    blockSignals(false);
}

bool MegaItemModel::canFetchMore(const QModelIndex &parent) const
{
    MegaItem *item = static_cast<MegaItem*>(parent.internalPointer());
    if(item)
    {   
        if(!item->childrenAreInit())
        {
            if(!mShowFiles)
            {
                return MegaSyncApp->getMegaApi()->getNumChildFolders(item->getNode().get()) > 0;
            }

            return MegaSyncApp->getMegaApi()->hasChildren(item->getNode().get());
        }

        return item->getNumChildren() != item->getNumItemChildren();
    }
    else
    {
        return mRootItems.size() < rootItemsCount();
    }
}

void MegaItemModel::setSyncSetupMode(bool value)
{
    mSyncSetupMode = value;
}

void MegaItemModel::addNode(std::unique_ptr<MegaNode> node, const QModelIndex &parent)
{
    MegaItem *parentItem = static_cast<MegaItem*>(parent.internalPointer());
    int numchildren = parentItem->getNumChildren();
    beginInsertRows(parent, numchildren, numchildren);
    parentItem->addNode(move(node));
    endInsertRows();
}

void MegaItemModel::removeNode(const QModelIndex &item)
{
    if(!item.isValid())
    {
        return;
    }
    std::shared_ptr<MegaNode> node = (static_cast<MegaItem*>(item.internalPointer()))->getNode();
    MegaItem *parent = static_cast<MegaItem*>(item.parent().internalPointer());
    if (!node)
    {
        return;
    }
    if(parent)
    {
        int index = parent->indexOf(static_cast<MegaItem*>(item.internalPointer()));
        beginRemoveRows(item.parent(), index, index);
        parent->removeNode(node);
    }
    else
    {
        int index = item.row();
        beginRemoveRows(item.parent(), index, index);
        mRootItems.removeOne(static_cast<MegaItem*>(item.internalPointer()));
    }

    endRemoveRows();
}

void MegaItemModel::showFiles(bool show)
{
    mShowFiles = show;
}

std::shared_ptr<MegaNode> MegaItemModel::getNode(const QModelIndex &index) const
{
    if(index.model()!=this)
    {
        return nullptr;
    }

    MegaItem *item = static_cast<MegaItem*>(index.internalPointer());
    if (!item)
    {
        return nullptr;
    }
    return item->getNode();
}

QVariant MegaItemModel::getIcon(const QModelIndex &index, MegaItem* item) const
{
    switch(index.column())
    {
    case COLUMN::NODE:
    {
        return QVariant::fromValue<QIcon>(item->getFolderIcon());
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

            if(item->isRoot())
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
            if(item->isRoot())
            {
                return QVariant();
            }
            const QString language = MegaSyncApp->getCurrentLanguageCode();
            QLocale locale(language);
            QDateTime dateTime = dateTime.fromSecsSinceEpoch(item->getNode()->getCreationTime());
            QDateTime currentDate = currentDate.currentDateTime();
            QLatin1String dateFormat ("dd MMM yyyy");
            QString timeFormat = locale.timeFormat(QLocale::ShortFormat);

            if(currentDate.toString(dateFormat)
                    == dateTime.toString(dateFormat))
            {
                return tr("Today at %1").arg(locale.toString(dateTime, timeFormat));
            }

            currentDate = currentDate.addDays(-1); //for checking if it was yesterday

            if(currentDate.toString(dateFormat)
                    == dateTime.toString(dateFormat))
            {
                return tr("Yesterday at %1").arg(locale.toString(dateTime, timeFormat));
            }
            //First: day Second: hour. This is done for allow translators to change the order
            //in case there are any language that needs to put in another order.
            return tr("%1 at %2").arg(locale.toString(dateTime, dateFormat), locale.toString(dateTime, timeFormat));
        }
        default:
            break;
    }
    return QVariant(QLatin1String(""));
}

void MegaItemModel::lockMutex(bool state)
{
    state ? mLoadingMutex.lock() : mLoadingMutex.unlock();
}

bool MegaItemModel::tryLock()
{
    return mLoadingMutex.tryLock();
}

int MegaItemModel::insertPosition(const std::unique_ptr<MegaNode>& node)
{
    int type = node->getType();
    int i;
    for (i = 0; i < mRootItems.size(); ++i)
    {
        std::shared_ptr<MegaNode> otherNode = mRootItems.at(i)->getNode();
        int otherNodeType = otherNode->getType();
        if (type >= otherNodeType && qstricmp(node->getName(), otherNode->getName()) <= 0)
        {
            break;
        }
    }
    return i;
}

void MegaItemModel::fetchItemChildren(const QModelIndex& parent) const
{
    MegaItem *item = static_cast<MegaItem*>(parent.internalPointer());

    if(!item->childrenAreInit() && !item->requestingChildren())
    {
        emit blockUi(true);
        emit requestChildNodes(item, mShowFiles);
        item->setProperty("INDEX", parent);
    }
}

void MegaItemModel::createChildItems(const QModelIndex &index, MegaItem *parent)
{
    int itemNumChildren = parent->getNumChildren();

    blockSignals(true);
    lockMutex(true);
    beginInsertRows(index, 0, itemNumChildren-1);
    parent->createChildItems();
    endInsertRows();
    lockMutex(false);
    blockSignals(false);

    emit rowsAdded(index, itemNumChildren);
}

void MegaItemModel::onChildNodesReady(MegaItem* parent, mega::MegaNodeList *nodes)
{
    if(nodes->size() > 0)
    {
        parent->setChildren(nodes);
        auto index = parent->property("INDEX").value<QModelIndex>();
        createChildItems(index, parent);
    }
    else
    {
        delete nodes;
    }
}

QModelIndex MegaItemModel::findItemByNodeHandle(const mega::MegaHandle& handle, const QModelIndex &parent)
{
    for(int i = 0; i < rowCount(parent); ++i)
    {
        QModelIndex idx = index(i, COLUMN::NODE, parent);
        if(idx.isValid())
        {
            QString name = idx.data(Qt::DisplayRole).toString();
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

MegaItemModelCloudDrive::MegaItemModelCloudDrive(QObject *parent)
    : MegaItemModel(parent)
    , mDelegateListener(mega::make_unique<QTMegaRequestListener>(MegaSyncApp->getMegaApi(), this))
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    megaApi->getCameraUploadsFolder(mDelegateListener.get());
    megaApi->getCameraUploadsFolderSecondary(mDelegateListener.get());
    megaApi->getMyChatFilesFolder(mDelegateListener.get());
}

MegaItemModelCloudDrive::~MegaItemModelCloudDrive()
{
}

void MegaItemModelCloudDrive::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api);
    if (e->getErrorCode() != MegaError::API_OK)
    {
        return;
    }
    if(request->getType() == mega::MegaRequest::TYPE_GET_ATTR_USER)
    {
        switch(request->getParamType())
        {
        case mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER:
        case mega::MegaApi::USER_ATTR_MY_CHAT_FILES_FOLDER:
        {
            //TODO EKA: DO IT BY ANOTHER WAY AS THIS LOADS ALL THE TREE
//            QModelIndex idx = findItemByNodeHandle(request->getNodeHandle(), index(0, 0));
//            if(idx.isValid())
//            {
//                if(MegaItem* item = static_cast<MegaItem*>(idx.internalPointer()))
//                {
//                    request->getParamType() == mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER
//                            ? item->setCameraFolder() : item->setChatFilesFolder();
//                }
//            }

            QVector<int> roles;
            roles.append(Qt::DecorationRole);
           // emit dataChanged(idx, idx, roles);
            return;
        }
        default:
            break;
        }
    }
}

QList<MegaItem *> MegaItemModelCloudDrive::getRootItems() const
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    auto root = std::unique_ptr<MegaNode>(megaApi->getRootNode());
    QList<MegaItem*> rootItems;
    rootItems.append(new MegaItem(move(root), mShowFiles));
    return rootItems;
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
    else
    {
        addRootItems();

        //Add the item of the Cloud Drive
        if(canFetchMore(index(0,0)))
        {
            fetchItemChildren(index(0,0));
        }
    }
}

MegaItemModelIncomingShares::MegaItemModelIncomingShares(QObject *parent)
    : MegaItemModel(parent)
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    mSharedNodeList = std::unique_ptr<MegaNodeList>(megaApi->getInShares());
}

void MegaItemModelIncomingShares::onItemInfoUpdated(int role)
{
//    if(MegaItem* item = static_cast<MegaItem*>(sender()))
//    {
//        for(int i = 0; i < rowCount(); ++i)
//        {
//            QModelIndex idx = index(i, COLUMN::USER); //we only update this column because we retrieve the data in async mode
//            if(idx.isValid())                         //so it is possible that we doesn´t have the information from the start
//            {
//                if(MegaItem* chkItem = static_cast<MegaItem*>(idx.internalPointer()))
//                {
//                    if(chkItem == item)
//                    {
//                        QVector<int> roles;
//                        roles.append(role);
//                        emit dataChanged(idx, idx, roles);
//                        return;
//                    }
//                }
//            }
//        }
//    }
}

MegaItemModelIncomingShares::~MegaItemModelIncomingShares()
{
}

QList<MegaItem *> MegaItemModelIncomingShares::getRootItems() const
{
    QList<MegaItem *> ret;

    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    for(int i = 0; i < mSharedNodeList->size(); i++)
    {
        auto node = mSharedNodeList->get(i)->copy();
        auto user = std::unique_ptr<MegaUser>(megaApi->getUserFromInShare(node));
        MegaItem* item = new MegaItem(std::unique_ptr<MegaNode>(node), mShowFiles);
        connect(item, &MegaItem::infoUpdated, this, &MegaItemModelIncomingShares::onItemInfoUpdated);
        item->setOwner(move(user));
        ret.append(item);
    }
    return ret;
}

int MegaItemModelIncomingShares::rootItemsCount() const
{
    return mSharedNodeList->size();
}

void MegaItemModelIncomingShares::fetchMore(const QModelIndex &parent)
{
    //move esto a otra funcion desde nodesready
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
    else
    {
        addRootItems();
        emit rowsAdded(QModelIndex(), rootItemsCount());
    }
}
