#include "NodeSelectorModelItem.h"
#include "QMegaMessageBox.h"
#include "MegaApplication.h"
#include "syncs/control/SyncInfo.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"

#include "mega/utils.h"

const int NodeSelectorModelItem::ICON_SIZE = 17;

using namespace mega;

NodeSelectorModelItem::NodeSelectorModelItem(std::unique_ptr<MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem) :
    QObject(parentItem),
    mOwnerEmail(QString()),
    mStatus(Status::NONE),
    mRequestingChildren(false),
    mShowFiles(showFiles),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mNode(std::move(node)),
    mOwner(nullptr)
{ 
    mChildrenCounter = mShowFiles ? MegaSyncApp->getMegaApi()->getNumChildren(mNode.get())
            : MegaSyncApp->getMegaApi()->getNumChildFolders(mNode.get());

    //If it has no children, the item does not need to be init
    mChildrenAreInit = mChildrenCounter > 0 ? false : true;

    if(mNode->isFile() || mNode->isInShare())
    {
        mStatus = Status::NONE;
        return;
    }
}

NodeSelectorModelItem::~NodeSelectorModelItem()
{
    qDeleteAll(mChildItems);
    mChildItems.clear();
}

std::shared_ptr<mega::MegaNode> NodeSelectorModelItem::getNode() const
{
    return mNode;
}

void NodeSelectorModelItem::createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList)
{
    if(!mNode->isFile())
    {
        auto info = std::make_shared<MessageInfo>();
        info->message = QLatin1String("Creating nodes");
        info->total = nodeList->size();

        for(int i = 0; i < nodeList->size(); i++)
        {
            if(i % 1000 == 0 ||
               i == (nodeList->size() -1))
            {
                info->count = i + 1;
                emit updateLoadingMessage(info);
            }

            auto node = std::unique_ptr<MegaNode>(nodeList->get(i)->copy());
            mChildItems.append(createModelItem(move(node), mShowFiles, this));
        }

        mRequestingChildren = false;
        mChildrenAreInit = true;
    }
}

bool NodeSelectorModelItem::areChildrenInitialized() const
{
    return mChildrenAreInit;
}

bool NodeSelectorModelItem::canFetchMore()
{
    if(!mChildrenAreInit)
    {
        return true;
    }
    else if(mChildrenCounter == 0)
    {
        return false;
    }

    return mChildItems.isEmpty();
}

bool NodeSelectorModelItem::requestingChildren() const
{
    return mRequestingChildren;
}

void NodeSelectorModelItem::setRequestingChildren(bool newRequestingChildren)
{
    mRequestingChildren = newRequestingChildren;
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::getParent()
{
    return dynamic_cast<NodeSelectorModelItem*>(parent());
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::getChild(int i)
{
    if(mChildItems.size() <= i)
    {
        return nullptr;
    }

    return mChildItems.at(i);
}

int NodeSelectorModelItem::getNumChildren()
{
    if(mNode->isFile())
    {
        return 0;
    }
    else if(!areChildrenInitialized())
    {
        return mChildrenCounter;
    }

    return mChildItems.size();
}

int NodeSelectorModelItem::indexOf(NodeSelectorModelItem* item)
{
    return mChildItems.indexOf(item);
}

QString NodeSelectorModelItem::getOwnerName()
{
    if(mFullNameAttribute && mFullNameAttribute->isAttributeReady())
    {
        return mFullNameAttribute->getFullName();
    }

    return mOwnerEmail;
}

QString NodeSelectorModelItem::getOwnerEmail()
{
    return mOwnerEmail;
}

void NodeSelectorModelItem::setOwner(std::unique_ptr<mega::MegaUser> user)
{
    if(!user)
    {
        return;
    }

    mOwner = std::move(user);
    mOwnerEmail = QString::fromUtf8(mOwner->getEmail());
    mFullNameAttribute = UserAttributes::FullName::requestFullName(mOwner->getEmail());
    if(mFullNameAttribute)
    {
        connect(mFullNameAttribute.get(), &UserAttributes::FullName::fullNameReady, this, &NodeSelectorModelItem::onFullNameAttributeReady);
        if(mFullNameAttribute->isAttributeReady())
        {
            onFullNameAttributeReady();
        }
    }
    mAvatarAttribute = UserAttributes::Avatar::requestAvatar(mOwner->getEmail());
    if(mAvatarAttribute)
    {
        connect(mAvatarAttribute.get(), &UserAttributes::Avatar::attributeReady, this, &NodeSelectorModelItem::onAvatarAttributeReady);
        if(mAvatarAttribute->isAttributeReady())
        {
            onAvatarAttributeReady();
        }
    }
}

void NodeSelectorModelItem::onFullNameAttributeReady()
{
    emit infoUpdated(Qt::DisplayRole);
}

void NodeSelectorModelItem::onAvatarAttributeReady()
{
    emit infoUpdated(Qt::DecorationRole);
}

QPixmap NodeSelectorModelItem::getOwnerIcon()
{
    if(mAvatarAttribute)
    {
        return mAvatarAttribute->getPixmap(ICON_SIZE);
    }

    return QPixmap();
}

QIcon NodeSelectorModelItem::getStatusIcons()
{
    QIcon statusIcons; //first is selected state icon / second is normal state icon

    if (mNode && !mNode->isNodeKeyDecrypted())
    {
        statusIcons.addFile(QLatin1String("://images/node_selector/alert-circle-hover.png"), QSize(), QIcon::Selected); //selected style icon
        statusIcons.addFile(QLatin1String("://images/node_selector/alert-circle-default.png"), QSize(), QIcon::Normal); //normal style icon
    }
    else
    {
        switch(mStatus)
        {
            case Status::SYNC:
            {
                statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //selected style icon
                statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
                break;
            }
            case Status::SYNC_PARENT:
            {
                statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //selected style icon
                statusIcons.addFile(QLatin1String("://images/node_selector/icon-small-sync-disabled.png"), QSize(), QIcon::Normal); //normal style icon
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return statusIcons;
}

NodeSelectorModelItem::Status NodeSelectorModelItem::getStatus() const
{
    return mStatus;
}

bool NodeSelectorModelItem::isSyncable()
{       
    return mStatus != Status::SYNC
            && mStatus != Status::SYNC_PARENT
            && mStatus != Status::SYNC_CHILD
            && mStatus != Status::BACKUP;
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::addNode(std::shared_ptr<MegaNode>node)
{
    auto item = createModelItem(std::unique_ptr<MegaNode>(node->copy()), mShowFiles, this);
    mChildItems.append(item);
    return item;
}

QList<QPointer<NodeSelectorModelItem>> NodeSelectorModelItem::addNodes(QList<std::shared_ptr<MegaNode>> nodes)
{
    QList<QPointer<NodeSelectorModelItem>> items;
    foreach(auto& node, nodes)
    {
        auto item = createModelItem(std::unique_ptr<MegaNode>(node->copy()), mShowFiles, this);
        items.append(item);
        mChildItems.append(item);
    }
    return items;
}


QPointer<NodeSelectorModelItem> NodeSelectorModelItem::findChildNode(std::shared_ptr<MegaNode> node)
{
    NodeSelectorModelItem* returnNode(nullptr);

    if (node)
    {
        for (int i = 0; i < mChildItems.size(); i++)
        {
            if (mChildItems[i]->getNode()->getHandle() == node->getHandle())
            {
                returnNode = mChildItems.takeAt(i);
                break;
            }
        }
    }

    return returnNode;
}

void NodeSelectorModelItem::displayFiles(bool enable)
{
    mShowFiles = enable;
}

int NodeSelectorModelItem::row()
{
    if (NodeSelectorModelItem* parent = getParent())
    {
        return parent->mChildItems.indexOf(const_cast<NodeSelectorModelItem*>(this));
    }
    return 0;
}

void NodeSelectorModelItem::updateNode(std::shared_ptr<mega::MegaNode> node)
{
    mNode = node;
}

void NodeSelectorModelItem::calculateSyncStatus()
{
    if(mNode->isFile())
    {
        return;
    }

    mStatus = Status::NONE;

    //if current item has a parent and the parent is already a sync or a sync_child, current item is also a sync_child
    //if not, continue checking. This avoid to block the mutex in the megaapi call below.
    if(parent())
    {
        if(auto parent_item = qobject_cast<NodeSelectorModelItem*>(parent()))
        {
            switch(parent_item->getStatus())
            {
            case Status::SYNC:
            case Status::SYNC_CHILD:
            {
                mStatus = Status::SYNC_CHILD;
            }
            default:
                break;
            }
        }
    }

    if(mStatus == Status::NONE)
    {
        std::unique_ptr<MegaError> err (MegaSyncApp->getMegaApi()->isNodeSyncableWithError(mNode.get()));
        switch(err->getSyncError())
        {
        case mega::MegaSync::Error::ACTIVE_SYNC_ABOVE_PATH:
        {
            mStatus = Status::SYNC_CHILD;
            break;
        }
        case mega::MegaSync::Error::ACTIVE_SYNC_BELOW_PATH:
        {
            mStatus = Status::SYNC_PARENT;
            break;
        }
        case mega::MegaSync::Error::ACTIVE_SYNC_SAME_PATH:
        {
            mStatus = Status::SYNC;
            break;
        }
        }
    }

//    if(mStatus == Status::NONE)
//    {
//        auto syncedFolders = SyncInfo::instance()->getMegaFolderHandles(SyncInfo::AllHandledSyncTypes);
//        if(syncedFolders.contains(mNode->getHandle()))
//        {
//            mStatus = Status::SYNC;
//        }
//    }
}

bool NodeSelectorModelItem::isCloudDrive() const
{
    return mNode->getHandle() == MegaSyncApp->getRootNode()->getHandle();
}

bool NodeSelectorModelItem::isRubbishBin() const
{
    return mNode->getHandle() == MegaSyncApp->getRubbishNode()->getHandle();
}

bool NodeSelectorModelItem::isVault()
{
    return false;
}

NodeSelectorModelItemSearch::NodeSelectorModelItemSearch(std::unique_ptr<mega::MegaNode> node, Types type, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), false, parentItem),
      mType(type)
{
    if(mType & NodeSelectorModelItemSearch::Type::INCOMING_SHARE)
    {
        auto user = std::unique_ptr<mega::MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(mNode.get(), true));
        setOwner(move(user));
    }

    calculateSyncStatus();
}

NodeSelectorModelItemSearch::~NodeSelectorModelItemSearch()
{

}

int NodeSelectorModelItemSearch::getNumChildren()
{
    return 0;
}

NodeSelectorModelItem *NodeSelectorModelItemSearch::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    Q_UNUSED(showFiles)
    Q_UNUSED(parentItem)
    return nullptr;
}

NodeSelectorModelItemIncomingShare::NodeSelectorModelItemIncomingShare(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    if(!parentItem)
    {
        auto user = std::unique_ptr<mega::MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(node.get()));
        setOwner(move(user));
    }
    calculateSyncStatus();
}

NodeSelectorModelItemIncomingShare::~NodeSelectorModelItemIncomingShare()
{

}

NodeSelectorModelItem *NodeSelectorModelItemIncomingShare::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemIncomingShare(move(node), showFiles, parentItem);
}

NodeSelectorModelItemBackup::NodeSelectorModelItemBackup(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{

}

NodeSelectorModelItemBackup::~NodeSelectorModelItemBackup()
{

}

bool NodeSelectorModelItemBackup::isSyncable()
{
    return false;
}

bool NodeSelectorModelItemBackup::isVault()
{
    //if it is a backup item and it doesn´t have parent it is the root node in backups tree
    return parent() == nullptr;
}

NodeSelectorModelItem *NodeSelectorModelItemBackup::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemBackup(move(node), showFiles, parentItem);
}

NodeSelectorModelItemCloudDrive::NodeSelectorModelItemCloudDrive(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    calculateSyncStatus();
}

NodeSelectorModelItemCloudDrive::~NodeSelectorModelItemCloudDrive()
{

}

NodeSelectorModelItem *NodeSelectorModelItemCloudDrive::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemCloudDrive(move(node), showFiles, parentItem);
}

////////////////
NodeSelectorModelItemRubbish::NodeSelectorModelItemRubbish(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
}

NodeSelectorModelItemRubbish::~NodeSelectorModelItemRubbish()
{

}

NodeSelectorModelItem *NodeSelectorModelItemRubbish::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemRubbish(move(node), showFiles, parentItem);
}
