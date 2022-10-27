#include "MegaItem.h"
#include "QMegaMessageBox.h"
#include "MegaApplication.h"
#include "model/SyncModel.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"

#include "mega/utils.h"

const int MegaItem::ICON_SIZE = 17;

using namespace mega;

MegaItem::MegaItem(std::unique_ptr<MegaNode> node, MegaItem *parentItem, bool showFiles) :
    QObject(parentItem),
    mShowFiles(showFiles),
    mOwnerEmail(QString()),
    mStatus(STATUS::NONE),
    mChildrenSet(false),
    mNode(std::move(node)),
    mOwner(nullptr),
    mMegaApi(MegaSyncApp->getMegaApi()),
    mIsVault(false)
{ 
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
        foreach(const QString& folder, SyncModel::instance()->getCloudDriveSyncMegaFolders(false))
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
        QStringList syncList = SyncModel::instance()->getCloudDriveSyncMegaFolders(true);
        if(isRoot() && !syncList.isEmpty())
        {
            mStatus = STATUS::SYNC_PARENT;
            return;
        }
        calculateSyncStatus(syncList);
    }
}


std::shared_ptr<mega::MegaNode> MegaItem::getNode()
{
    return mNode;
}

void MegaItem::setChildren(std::shared_ptr<MegaNodeList> children)
{
    mChildrenSet = true;
    for (int i = 0; i < children->size(); i++)
    {
        auto node = std::unique_ptr<MegaNode>(children->get(i)->copy());
        if (!mShowFiles && node->getType() == MegaNode::TYPE_FILE)
        {
            break;
        }
        mChildItems.append(new MegaItem(move(node), this, mShowFiles));
    }
}

bool MegaItem::areChildrenSet()
{
    return mChildrenSet;
}

MegaItem *MegaItem::getParent()
{
    return dynamic_cast<MegaItem*>(parent());
}

MegaItem* MegaItem::getChild(int i)
{
    return mChildItems.at(i);
}

int MegaItem::getNumChildren()
{
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
    foreach(const QString& folder, SyncModel::instance()->getMegaFolders(SyncModel::AllHandledSyncTypes))
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

void MegaItem::addNode(std::unique_ptr<MegaNode>node)
{
    mChildItems.append(new MegaItem(move(node), this, mShowFiles));
}

void MegaItem::removeNode(std::shared_ptr<MegaNode> node)
{
    if (!node)
    {
        return;
    }

    for (int i = 0; i < mChildItems.size(); i++)
    {
        if (mChildItems[i]->getNode()->getHandle() == node->getHandle())
        {
            MegaItem* item = mChildItems.takeAt(i);
            delete item;
            return;
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

MegaItem::~MegaItem()
{
    qDeleteAll(mChildItems);
    mChildItems.clear();
}

void MegaItem::calculateSyncStatus(const QStringList &folders)
{
    auto syncedFolders = SyncModel::instance()->getMegaFolderHandles(SyncModel::AllHandledSyncTypes);
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
