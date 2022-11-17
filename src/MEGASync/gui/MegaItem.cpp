#include "MegaItem.h"
#include "QMegaMessageBox.h"
#include "MegaApplication.h"
#include "syncs/control/SyncInfo.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"

#include "mega/utils.h"

const int MegaItem::ICON_SIZE = 17;

using namespace mega;

MegaItem::MegaItem(std::unique_ptr<MegaNode> node, bool showFiles, MegaItem *parentItem) :
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
    if(mShowFiles)
    {
        mChildrenCounter = MegaSyncApp->getMegaApi()->getNumChildren(mNode.get());
    }
    else
    {
        mChildrenCounter = MegaSyncApp->getMegaApi()->getNumChildFolders(mNode.get());
    }

    //If it has no children, the item does not need to be init
    mChildrenAreInit = mChildrenCounter > 0 ? false : true;

    if(mNode->isFile() || mNode->isInShare())
    {
        mStatus = STATUS::NONE;
        return;
    }

    //This code is to calculate if a folder which is inside an incoming share folder is a child of a sync
    //////////
    MegaItem *parent_item = getParent();
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

MegaItem::~MegaItem()
{
    qDeleteAll(mChildItems);
    mChildItems.clear();
}

std::shared_ptr<mega::MegaNode> MegaItem::getNode() const
{
    return mNode;
}

void MegaItem::createChildItems(std::unique_ptr<mega::MegaNodeList> nodeList)
{
    if(mNode->isFile())
    {
        return;
    }

    for(int i = 0; i < nodeList->size(); i++)
    {
        auto node = std::unique_ptr<MegaNode>(nodeList->get(i)->copy());
        mChildItems.append(new MegaItem(move(node),mShowFiles, this));
    }

    mRequestingChildren = false;
    mChildrenAreInit = true;
}

bool MegaItem::childrenAreInit()
{
    return mChildrenAreInit;
}

bool MegaItem::canFetchMore()
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

bool MegaItem::requestingChildren() const
{
    return mRequestingChildren;
}

void MegaItem::setRequestingChildren(bool newRequestingChildren)
{
    mRequestingChildren = newRequestingChildren;
}

MegaItem *MegaItem::getParent()
{
    return dynamic_cast<MegaItem*>(parent());
}

MegaItem* MegaItem::getChild(int i)
{
    if(mChildItems.size() <= i)
    {
        return nullptr;
    }

    return mChildItems.at(i);
}

int MegaItem::getNumChildren()
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
    //return mChildNodes->size();
}

int MegaItem::getNumItemChildren()
{
    if(mNode->isFile())
    {
        return 0;
    }
    return mChildItems.size();
}

int MegaItem::indexOf(MegaItem* item)
{
    return mChildItems.indexOf(item);
}

QString MegaItem::getOwnerName()
{
    if(mFullNameAttribute && mFullNameAttribute->isAttributeReady())
    {
        return mFullNameAttribute->getFullName();
    }

    return mOwnerEmail;
}

QString MegaItem::getOwnerEmail()
{
    return mOwnerEmail;
}

void MegaItem::setOwner(std::unique_ptr<mega::MegaUser> user)
{
    mOwner = std::move(user);
    mOwnerEmail = QString::fromUtf8(mOwner->getEmail());
    mFullNameAttribute = UserAttributes::FullName::requestFullName(mOwner->getEmail());
    if(mFullNameAttribute)
    {
        connect(mFullNameAttribute.get(), &UserAttributes::FullName::attributeReady, this, &MegaItem::onFullNameAttributeReady);
        if(mFullNameAttribute->isAttributeReady())
        {
            onFullNameAttributeReady();
        }
    }
    mAvatarAttribute = UserAttributes::Avatar::requestAvatar(mOwner->getEmail());
    if(mAvatarAttribute)
    {
        connect(mAvatarAttribute.get(), &UserAttributes::Avatar::attributeReady, this, &MegaItem::onAvatarAttributeReady);
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

void MegaItem::onFullNameAttributeReady()
{
    emit infoUpdated(Qt::DisplayRole);
}

void MegaItem::onAvatarAttributeReady()
{
    emit infoUpdated(Qt::DecorationRole);
}

QPixmap MegaItem::getOwnerIcon()
{
    if(mAvatarAttribute)
    {
        return mAvatarAttribute->getPixmap(ICON_SIZE);
    }

    return QPixmap();
}

QIcon MegaItem::getStatusIcons()
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

int MegaItem::getStatus()
{
    return mStatus;
}

bool MegaItem::isSyncable()
{       
    return mStatus != SYNC
            && mStatus != SYNC_PARENT
            && mStatus != SYNC_CHILD
            && mStatus != BACKUP;
}

MegaItem* MegaItem::addNode(std::shared_ptr<MegaNode>node)
{
    auto nodeCopy(node.get()->copy());
    auto item = new MegaItem(std::unique_ptr<MegaNode>(nodeCopy),mShowFiles, this);
    mChildItems.append(item);
    return item;
}

MegaItem* MegaItem::removeNode(std::shared_ptr<MegaNode> node)
{
    if (!node)
    {
        return nullptr;
    }

    for (int i = 0; i < mChildItems.size(); i++)
    {
        if (mChildItems[i]->getNode()->getHandle() == node->getHandle())
        {
            MegaItem* item = mChildItems.takeAt(i);
            return item;
        }
    }
}

void MegaItem::displayFiles(bool enable)
{
    mShowFiles = enable;
}

void MegaItem::setAsVaultNode()
{
    mIsVault = true;
}

int MegaItem::row()
{
    if (MegaItem* parent = getParent())
    {
        return parent->mChildItems.indexOf(const_cast<MegaItem*>(this));
    }
    return 0;
}

void MegaItem::calculateSyncStatus(const QStringList &folders)
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

bool MegaItem::isRoot()
{
    return mNode->getHandle() == MegaSyncApp->getRootNode()->getHandle();
}

bool MegaItem::isVault()
{
    return mIsVault;
}
