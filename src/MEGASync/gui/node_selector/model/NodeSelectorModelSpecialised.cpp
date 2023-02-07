#include "NodeSelectorModelSpecialised.h"
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

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelCloudDrive::NodeSelectorModelCloudDrive(QObject *parent)
    : NodeSelectorModel(parent)
{
}

void NodeSelectorModelCloudDrive::createRootNodes()
{    
    emit requestCloudDriveRootCreation();
}

int NodeSelectorModelCloudDrive::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelCloudDrive::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelCloudDrive::firstLoad()
{
    connect(this, &NodeSelectorModelCloudDrive::requestCloudDriveRootCreation, mNodeRequesterWorker, &NodeRequester::createCloudDriveRootItem);
    connect(mNodeRequesterWorker, &NodeRequester::megaCloudDriveRootItemCreated, this, &NodeSelectorModelCloudDrive::onRootItemCreated, Qt::QueuedConnection);

    addRootItems();
}

void NodeSelectorModelCloudDrive::onRootItemCreated(NodeSelectorModelItem *item)
{
    Q_UNUSED(item)
    rootItemsLoaded();

    //Add the item of the Cloud Drive
    auto rootIndex(index(0,0));
    if(canFetchMore(rootIndex))
    {
        fetchItemChildren(rootIndex);
    }
}

///////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelIncomingShares::NodeSelectorModelIncomingShares(QObject *parent)
    : NodeSelectorModel(parent)
{
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    mSharedNodeList = std::unique_ptr<MegaNodeList>(megaApi->getInShares());
}

void NodeSelectorModelIncomingShares::onItemInfoUpdated(int role)
{
    if(NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(sender()))
    {
        for(int i = 0; i < rowCount(); ++i)
        {
            QModelIndex idx = index(i, COLUMN::USER); //we only update this column because we retrieve the data in async mode
            if(idx.isValid())                         //so it is possible that we doesn´t have the information from the start
            {
                if(NodeSelectorModelItem* chkItem = static_cast<NodeSelectorModelItem*>(idx.internalPointer()))
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

void NodeSelectorModelIncomingShares::onRootItemsCreated(QList<NodeSelectorModelItem *> items)
{
    Q_UNUSED(items)
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

void NodeSelectorModelIncomingShares::createRootNodes()
{
    emit requestIncomingSharesRootCreation(mSharedNodeList);
}

int NodeSelectorModelIncomingShares::rootItemsCount() const
{
    return mSharedNodeList->size();
}

void NodeSelectorModelIncomingShares::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelIncomingShares::firstLoad()
{
    connect(this, &NodeSelectorModelIncomingShares::requestIncomingSharesRootCreation, mNodeRequesterWorker, &NodeRequester::createIncomingSharesRootItems);
    connect(mNodeRequesterWorker, &NodeRequester::megaIncomingSharesRootItemsCreated, this, &NodeSelectorModelIncomingShares::onRootItemsCreated, Qt::QueuedConnection);

    addRootItems();
}

///////////////////////////////////////////////////////////////////////////////////////////////
NodeSelectorModelBackups::NodeSelectorModelBackups(QObject *parent)
    : NodeSelectorModel(parent)
    , mBackupDevicesSize(0)
{
}

NodeSelectorModelBackups::~NodeSelectorModelBackups()
{
}

void NodeSelectorModelBackups::createRootNodes()
{
    emit requestBackupsRootCreation(mBackupsHandle);
}

int NodeSelectorModelBackups::rootItemsCount() const
{
    return 1;
}

void NodeSelectorModelBackups::fetchMore(const QModelIndex &parent)
{
    if (parent.isValid())
    {
        fetchItemChildren(parent);
    }
}

void NodeSelectorModelBackups::firstLoad()
{
    connect(this, &NodeSelectorModelBackups::requestBackupsRootCreation, mNodeRequesterWorker, &NodeRequester::createBackupRootItems);
    connect(mNodeRequesterWorker, &NodeRequester::megaBackupRootItemsCreated, this, &NodeSelectorModelBackups::onRootItemCreated, Qt::QueuedConnection);

    auto backupsRequest = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
    mBackupsHandle = backupsRequest->getMyBackupsHandle();
    connect(backupsRequest.get(), &UserAttributes::MyBackupsHandle::attributeReady,
            this, &NodeSelectorModelBackups::onMyBackupsHandleReceived);

    if(backupsRequest->isAttributeReady())
    {
        onMyBackupsHandleReceived(backupsRequest->getMyBackupsHandle());
    }
    else
    {
        addRootItems();
    }
}

bool NodeSelectorModelBackups::canBeDeleted() const
{
    return false;
}

void NodeSelectorModelBackups::onMyBackupsHandleReceived(mega::MegaHandle handle)
{
    mBackupsHandle = handle;
    if (mBackupsHandle != INVALID_HANDLE)
    {
        addRootItems();
    }
}

bool NodeSelectorModelBackups::addToLoadingList(const std::shared_ptr<MegaNode> node)
{
    return node && node->getType() != mega::MegaNode::TYPE_VAULT;
}

void NodeSelectorModelBackups::loadLevelFinished()
{
    if(mIndexesActionInfo.indexesToBeExpanded.size() == 1 && mIndexesActionInfo.indexesToBeExpanded.at(0) == index(0, 0))
    {
        QModelIndex rootIndex(index(0, 0));
        int rowcount = rowCount(rootIndex);
        mBackupDevicesSize = rowcount;
        for(int i = 0 ; i < rowcount; i++)
        {
            auto idx = index(i, 0, rootIndex);
            if(canFetchMore(idx))
            {
                fetchItemChildren(idx);
            }
        }
    }
    else
    {
        if(mBackupDevicesSize > 0)
        {
            mBackupDevicesSize--;
        }
        if(mBackupDevicesSize == 0)
        {
            emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded);
        }
    }
}

void NodeSelectorModelBackups::onRootItemCreated(NodeSelectorModelItem *item)
{
    rootItemsLoaded();

    if(!item)
    {
        loadLevelFinished();
    }
    else
    {

        QModelIndex rootIndex(index(0, 0));
        //Add the item of the Backups Drive
        if(canFetchMore(rootIndex))
        {
            fetchItemChildren(rootIndex);
        }
    }
}

NodeSelectorModelSearch::NodeSelectorModelSearch(NodeSelectorModelItemSearch::Types allowedTypes, QObject *parent)
    : NodeSelectorModel(parent),
      mAllowedTypes(allowedTypes)
{

}

NodeSelectorModelSearch::~NodeSelectorModelSearch()
{

}

void NodeSelectorModelSearch::firstLoad()
{
    connect(this, &NodeSelectorModelSearch::searchNodes, mNodeRequesterWorker, &NodeRequester::search);
    connect(mNodeRequesterWorker, &NodeRequester::searchItemsCreated, this, &NodeSelectorModelSearch::onRootItemsCreated, Qt::QueuedConnection);
}

void NodeSelectorModelSearch::createRootNodes()
{
    //pure virtual function in the base class, in first stage this model is empty so not need to put any code here.
}

void NodeSelectorModelSearch::searchByText(const QString &text)
{
    mNodeRequesterWorker->restartSearch();
    addRootItems();
    emit searchNodes(text, mAllowedTypes);
}

void NodeSelectorModelSearch::stopSearch()
{
    mNodeRequesterWorker->restartSearch();
}

int NodeSelectorModelSearch::rootItemsCount() const
{
    return 0;
}

bool NodeSelectorModelSearch::canFetchMore(const QModelIndex &parent) const
{
    Q_UNUSED(parent)
    return false;
}

QVariant NodeSelectorModelSearch::data(const QModelIndex &index, int role) const
{
    if(!index.isValid())
    {
        return QVariant();
    }
    if(index.column() == NODE)
    {
        switch(role)
        {
        case toInt(NodeRowDelegateRoles::INDENT_ROLE):
        {
            return -15;
        }
        case toInt(NodeRowDelegateRoles::SMALL_ICON_ROLE):
        {
            return false;
        }
        }
    }
    else if(index.column() == STATUS && role == Qt::DecorationRole)
    {
        NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer());
        if(item->getStatus() == NodeSelectorModelItem::Status::SYNC_CHILD)
        {
            QIcon statusIcons; //first is selected state icon / second is normal state icon
            statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //selected style icon
            statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
            return statusIcons;
        }
    }
    return NodeSelectorModel::data(index, role);
}

void NodeSelectorModelSearch::proxyInvalidateFinished()
{
    mNodeRequesterWorker->lockSearchMutex(false);
}

void NodeSelectorModelSearch::onRootItemsCreated(QList<NodeSelectorModelItem *> items, NodeSelectorModelItemSearch::Types searchedTypes)
{
    Q_UNUSED(items)
    if(mNodeRequesterWorker->trySearchLock())
    {
        mSearchedTypes = searchedTypes;
        rootItemsLoaded();
        emit levelsAdded(mIndexesActionInfo.indexesToBeExpanded, true);
    }
}

const NodeSelectorModelItemSearch::Types &NodeSelectorModelSearch::searchedTypes() const
{
    return mSearchedTypes;
}
