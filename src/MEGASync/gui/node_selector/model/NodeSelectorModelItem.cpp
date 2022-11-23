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
    mStatus(STATUS::NONE),
    mChildrenSet(false),
    mRequestingChildren(false),
    mShowFiles(showFiles),
    mNode(std::move(node)),
    mOwner(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mIsVault(false)
{ 
    mChildrenCounter = mShowFiles ? MegaSyncApp->getMegaApi()->getNumChildren(mNode.get())
            : MegaSyncApp->getMegaApi()->getNumChildFolders(mNode.get());

    //If it has no children, the item does not need to be init
    mChildrenAreInit = mChildrenCounter > 0 ? false : true;

    if(mNode->isFile() || mNode->isInShare())
    {
        mStatus = STATUS::NONE;
        return;
    }

    //This code is to calculate if a folder which is inside an incoming share folder is a child of a sync
    //////////
    NodeSelectorModelItem *parent_item = getParent();
    while(parent_item && parent_item->getNode()->getParentHandle() != INVALID_HANDLE)
    {
        parent_item = parent_item->getParent();
    }

    QStringList folderList;
    if(isVault() || (parent_item && parent_item->isVault()))
    {
        mStatus = STATUS::BACKUP;
        return;
    }
    if(parent_item && parent_item->getNode()->isInShare())
    {
        foreach(const QString& folder, SyncInfo::instance()->getCloudDriveSyncMegaFolders(false))
        {
            if(folder.startsWith(parent_item->getOwnerEmail()))
            {
                folderList.append(folder.split(QLatin1Char(':')).last().prepend(QLatin1Char('/')));
            }
        }
        calculateSyncStatus(folderList);
    }
    ////////////
    else
    {
        QStringList syncList = SyncInfo::instance()->getCloudDriveSyncMegaFolders(true);
        if(isRoot() && !syncList.isEmpty())
        {
            mStatus = STATUS::SYNC_PARENT;
            return;
        }
        calculateSyncStatus(syncList);
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
        for(int i = 0; i < nodeList->size(); i++)
        {
            auto node = std::unique_ptr<MegaNode>(nodeList->get(i)->copy());
            mChildItems.append(new NodeSelectorModelItem(move(node),mShowFiles, this));
        }

        mRequestingChildren = false;
        mChildrenAreInit = true;
    }
}

bool NodeSelectorModelItem::childrenAreInit()
{
    return mChildrenAreInit;
}

bool NodeSelectorModelItem::canFetchMore()
{
    if(!mChildrenAreInit)
    {
        return true;
    }
    else
    {
        if(mChildrenCounter == 0)
        {
            return false;
        }
        else
        {
            return mChildItems.isEmpty();
        }
    }
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
    else if(!childrenAreInit())
    {
        return mChildrenCounter;
    }
    else
    {
        return mChildItems.size();
    }
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

    QStringList folderList;
    //Calculating if we have a synced childs.
    foreach(const QString& folder, SyncInfo::instance()->getMegaFolders(SyncInfo::AllHandledSyncTypes))
    {
        if(folder.startsWith(mOwnerEmail))
        {
            folderList.append(folder.split(QLatin1Char(':')).last().prepend(QLatin1Char('/')));
        }
    }
    calculateSyncStatus(folderList);
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
    switch(mStatus)
    {
    case STATUS::SYNC:
    {
        statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //selected style icon
        statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
        break;
    }
    case STATUS::SYNC_PARENT:
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

    return statusIcons;
}

int NodeSelectorModelItem::getStatus()
{
    return mStatus;
}

bool NodeSelectorModelItem::isSyncable()
{       
    return mStatus != SYNC
            && mStatus != SYNC_PARENT
            && mStatus != SYNC_CHILD
            && mStatus != BACKUP;
}

QPointer<NodeSelectorModelItem> NodeSelectorModelItem::addNode(std::shared_ptr<MegaNode>node)
{
    auto nodeCopy(node.get()->copy());
    auto item = new NodeSelectorModelItem(std::unique_ptr<MegaNode>(nodeCopy),mShowFiles, this);
    mChildItems.append(item);
    return item;
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

void NodeSelectorModelItem::setAsVaultNode()
{
    mIsVault = true;
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

void NodeSelectorModelItem::calculateSyncStatus(const QStringList &folders)
{
    auto syncedFolders = SyncInfo::instance()->getMegaFolderHandles(SyncInfo::AllHandledSyncTypes);
    if(syncedFolders.contains(mNode->getHandle()))
    {
        mStatus = STATUS::SYNC;
        return;
    }

    QString parentFolders;
    std::shared_ptr<MegaNode> n = mNode;
    parentFolders.append(QLatin1Char('/'));
    parentFolders.append(QString::fromUtf8(n->getName()));
    while(n && n->getParentHandle () != INVALID_HANDLE)
    {
        n = std::shared_ptr<MegaNode>(mMegaApi->getNodeByHandle(n->getParentHandle()));
        if(n->getType() != MegaNode::TYPE_ROOT)
        {
            parentFolders.prepend(QString::fromUtf8(n->getName()));
            parentFolders.prepend(QLatin1Char('/'));
        }
    }

    foreach(const QString& syncFolder, folders)
    {
        if(syncFolder.startsWith(parentFolders))
        {
            mStatus = STATUS::SYNC_PARENT;
            return;
        }
        else if(parentFolders.startsWith(syncFolder))
        {
            mStatus = STATUS::SYNC_CHILD;
            return;
        }
    }
}

bool NodeSelectorModelItem::isRoot()
{
    return mNode->getHandle() == MegaSyncApp->getRootNode()->getHandle();
}

bool NodeSelectorModelItem::isVault()
{
    return mIsVault;
}
