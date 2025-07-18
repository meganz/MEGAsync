#include "NodeSelectorModelItem.h"

#include "Avatar.h"
#include "FullName.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "ViewLoadingScene.h"

const int NodeSelectorModelItem::ICON_SIZE = 17;
const int UPDATE_ACCESS_THRESHOLD_MS = 50;

using namespace mega;

NodeSelectorModelItem::NodeSelectorModelItem(std::unique_ptr<MegaNode> node,
                                             bool showFiles,
                                             NodeSelectorModelItem* parentItem):
    QObject(parentItem),
    mOwnerEmail(QString()),
    mStatus(Status::NONE),
    mRequestingChildren(false),
    mShowFiles(showFiles),
    mNodeAccess(mega::MegaShare::ACCESS_OWNER),
    mNodeAccessLastUpdate(0),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mNode(std::move(node)),
    mOwner(nullptr)
{
    // In case we don´t have a valid node (which is an error), just use a goofy node
    if (!mNode)
    {
        mNode = std::make_shared<mega::MegaNode>();
    }

    resetChildrenCounter();

    if(mNode->isFile() || mNode->isInShare())
    {
        mStatus = Status::NONE;
    }
}

NodeSelectorModelItem::~NodeSelectorModelItem()
{
    qDeleteAll(mChildItems);
    mChildItems.clear();
}

bool NodeSelectorModelItem::isValid() const
{
    return mNode && mNode->getHandle() != mega::INVALID_HANDLE;
}

std::shared_ptr<mega::MegaNode> NodeSelectorModelItem::getNode() const
{
    return mNode;
}

bool NodeSelectorModelItem::isSpecialNode() const
{
    return (isCloudDrive() || isVault() || isRubbishBin());
}

bool NodeSelectorModelItem::canBeRenamed() const
{
    if (isCloudDrive() || isVault() || isRubbishBin() || isInRubbishBin() ||
        (mMegaApi->isInVault(mNode.get())) || (getNodeAccess() < mega::MegaShare::ACCESS_FULL))
    {
        return false;
    }

    return true;
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
            auto child = createModelItem(std::move(node), mShowFiles, this);
            if (child->isValid())
            {
                connect(child,
                        &NodeSelectorModelItem::destroyed,
                        this,
                        &NodeSelectorModelItem::onChildDestroyed);
                mChildItems.append(child);
            }
            else
            {
                child->deleteLater();
            }
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

void NodeSelectorModelItem::resetChildrenCounter()
{
    mChildrenCounter = mShowFiles ? MegaSyncApp->getMegaApi()->getNumChildren(mNode.get()) :
                                    MegaSyncApp->getMegaApi()->getNumChildFolders(mNode.get());

    // If it has no children, the item does not need to be init
    mChildrenAreInit = mChildrenCounter > 0 ? false : true;
}

int NodeSelectorModelItem::getNodeAccess() const
{
    auto currentTimestamp(QDateTime::currentMSecsSinceEpoch());
    if ((currentTimestamp - mNodeAccessLastUpdate) > UPDATE_ACCESS_THRESHOLD_MS)
    {
        mNodeAccess = Utilities::getNodeAccess(mNode.get());
        mNodeAccessLastUpdate = currentTimestamp;
    }

    return mNodeAccess;
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

void NodeSelectorModelItem::onChildDestroyed()
{
    mChildrenCounter--;
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
    return !isInRubbishBin() && mStatus != Status::SYNC && mStatus != Status::SYNC_PARENT &&
           mStatus != Status::SYNC_CHILD && mStatus != Status::BACKUP &&
           getNodeAccess() >= mega::MegaShare::ACCESS_FULL;
}

QList<QPointer<NodeSelectorModelItem>> NodeSelectorModelItem::addNodes(QList<std::shared_ptr<MegaNode>> nodes)
{
    QList<QPointer<NodeSelectorModelItem>> items;
    foreach(auto& node, nodes)
    {
        auto child = createModelItem(std::unique_ptr<MegaNode>(node->copy()), mShowFiles, this);
        if (child->isValid())
        {
            items.append(child);
            connect(child,
                    &NodeSelectorModelItem::destroyed,
                    this,
                    &NodeSelectorModelItem::onChildDestroyed);
            mChildItems.append(child);
            mChildrenCounter++;
        }
        else
        {
            child->deleteLater();
        }
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
}

bool NodeSelectorModelItem::isCloudDrive() const
{
    auto rootNode(MegaSyncApp->getRootNode());
    return rootNode && mNode->getHandle() == rootNode->getHandle();
}

bool NodeSelectorModelItem::isRubbishBin() const
{
    return mNode->getHandle() == MegaSyncApp->getRubbishNode()->getHandle();
}

bool NodeSelectorModelItem::isInRubbishBin() const
{
    return mNode && mMegaApi->isInRubbish(mNode.get());
}

bool NodeSelectorModelItem::isVault() const
{
    return false;
}

bool NodeSelectorModelItem::isVaultDevice() const
{
    return false;
}

bool NodeSelectorModelItem::isInShare() const
{
    return mNode->isInShare();
}

bool NodeSelectorModelItem::isInVault() const
{
    return MegaSyncApp->getMegaApi()->isInVault(mNode.get());
}

NodeSelectorModelItemSearch::NodeSelectorModelItemSearch(std::unique_ptr<mega::MegaNode> node, Types type, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), false, parentItem),
      mType(type)
{
    if(mType & NodeSelectorModelItemSearch::Type::INCOMING_SHARE)
    {
        auto user = std::unique_ptr<mega::MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(mNode.get(), true));
        setOwner(std::move(user));
    }

    calculateSyncStatus();

    qRegisterMetaType<Types>("Types");
}

NodeSelectorModelItemSearch::~NodeSelectorModelItemSearch()
{

}

void NodeSelectorModelItemSearch::setType(Types type)
{
    if (mType != type)
    {
        mType = type;
        emit typeChanged(type);
    }
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
        auto user = std::unique_ptr<mega::MegaUser>(MegaSyncApp->getMegaApi()->getUserFromInShare(mNode.get()));
        setOwner(std::move(user));
    }
    calculateSyncStatus();
}

NodeSelectorModelItemIncomingShare::~NodeSelectorModelItemIncomingShare()
{

}

NodeSelectorModelItem *NodeSelectorModelItemIncomingShare::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemIncomingShare(std::move(node), showFiles, parentItem);
}

NodeSelectorModelItemBackup::NodeSelectorModelItemBackup(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
    : NodeSelectorModelItem(std::move(node), showFiles, parentItem)
{
    mStatus = Status::BACKUP;
}

NodeSelectorModelItemBackup::~NodeSelectorModelItemBackup()
{

}

bool NodeSelectorModelItemBackup::isSyncable()
{
    return false;
}

bool NodeSelectorModelItemBackup::isVault() const
{
    //if it is a backup item and it doesn´t have parent it is the root node in backups tree
    return parent() == nullptr;
}

bool NodeSelectorModelItemBackup::isVaultDevice() const
{
    return parent() && parent()->parent() == nullptr;
}

NodeSelectorModelItem *NodeSelectorModelItemBackup::createModelItem(std::unique_ptr<mega::MegaNode> node, bool showFiles, NodeSelectorModelItem *parentItem)
{
    return new NodeSelectorModelItemBackup(std::move(node), showFiles, parentItem);
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
    return new NodeSelectorModelItemCloudDrive(std::move(node), showFiles, parentItem);
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
    return new NodeSelectorModelItemRubbish(std::move(node), showFiles, parentItem);
}
