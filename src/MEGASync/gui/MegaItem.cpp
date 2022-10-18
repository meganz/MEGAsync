#include "MegaItem.h"
#include "QMegaMessageBox.h"
#include "MegaApplication.h"
#include "model/Model.h"
#include "MegaApplication.h"
#include "mega/utils.h"
#include "UserAttributesRequests/FullName.h"
#include "UserAttributesRequests/Avatar.h"

#include <QByteArray>


const int MegaItem::ICON_SIZE = 17;

using namespace mega;

MegaItem::MegaItem(std::unique_ptr<MegaNode> node, bool showFiles, MegaItem *parentItem) :
    QObject(parentItem),
    mOwnerEmail(QString()),
    mStatus(STATUS::NONE),
    mCameraFolder(false),
    mChatFilesFolder(false),
    mChildrenSet(false),
    mRequestingChildren(false),
    mNode(std::move(node)),
    mOwner(nullptr),
    mShowFiles(showFiles)
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
    if(parent_item && parent_item->getNode()->isInShare())
    {
        foreach(const QString& folder, Model::instance()->getCloudDriveSyncMegaFolders(false))
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
        QStringList syncList = Model::instance()->getCloudDriveSyncMegaFolders(true);
        if(isRoot() && !syncList.isEmpty())
        {
            mStatus = STATUS::SYNC_PARENT;
            return;
        }
        calculateSyncStatus(syncList);
    }
}


std::shared_ptr<mega::MegaNode> MegaItem::getNode() const
{
    return mNode;
}

void MegaItem::setChildren(mega::MegaNodeList *nodes)
{
    mChildNodes.reset(nodes);
    mRequestingChildren = false;
}

void MegaItem::fetchChildren()
{
    return;

    if(mNode->isFile())
    {
        return;
    }

    if(!mChildrenSet)
    {
        mChildrenSet = true;
        MegaApi* megaApi = MegaSyncApp->getMegaApi();

        if(!mShowFiles)
        {
            auto childNodes = std::unique_ptr<MegaNodeList>(megaApi->getChildren(mNode.get()));
            mChildNodes = std::unique_ptr<MegaNodeList>(MegaNodeList::createInstance());
            for(int i = 0; i < childNodes->size();++i)
            {
                auto childNode = childNodes->get(i);
                if(childNode->isFile() && !mShowFiles)
                {
                    break;
                }
                mChildNodes->addNode(childNodes->get(i));
            }
        }
        else
        {
            mChildNodes = std::unique_ptr<MegaNodeList>(megaApi->getChildren(mNode.get()));
        }
    }
}

void MegaItem::createChildItems()
{
    if(mNode->isFile())
    {
        return;
    }

    for(int i = 0; i < mChildNodes->size(); i++)
    {
        auto node = std::unique_ptr<MegaNode>(mChildNodes->get(i)->copy());
        mChildItems.append(new MegaItem(move(node), mShowFiles, this));
    }
}

bool MegaItem::childrenAreInit()
{
    return mChildNodes != nullptr;
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

MegaItem *MegaItem::getChild(int i)
{
    if(mChildItems.size() <= i)
    {
        return nullptr;
    }

    return mChildItems.at(i);
}

int MegaItem::getNumChildren()
{
    if(mNode->isFile() || !mChildNodes)
    {
        return 0;
    }
    //qDebug() << mChildNodes->size();
    return mChildNodes->size();
}

int MegaItem::getNumItemChildren()
{
    if(mNode->isFile())
    {
        return 0;
    }
    return mChildItems.size();
}

int MegaItem::indexOf(MegaItem *item)
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
    foreach(const QString& folder, Model::instance()->getMegaFolders())
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
        statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //normal style icon
        statusIcons.addFile(QLatin1String("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
        break;
    }
    case STATUS::SYNC_PARENT:
    {
        statusIcons.addFile(QLatin1String("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //normal style icon
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

QIcon MegaItem::getFolderIcon()
{
    if (getNode()->getType() >= MegaNode::TYPE_FOLDER)
    {
        if(mCameraFolder)
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/node_selector/small-camera-sync.png"), QSize(), QIcon::Normal);
            icon.addFile(QLatin1String("://images/node_selector/small-folder-camera-sync-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if(mChatFilesFolder)
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/node_selector/small-chat-files.png"), QSize(), QIcon::Normal);
            icon.addFile(QLatin1String("://images/node_selector/small-chat-files-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if (getNode()->isInShare())
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/node_selector/small-folder-incoming.png"), QSize(), QIcon::Normal);
            icon.addFile(QLatin1String("://images/node_selector/small-folder-incoming-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if (getNode()->isOutShare())
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/node_selector/small-folder-outgoing.png"), QSize(), QIcon::Normal);
            icon.addFile(QLatin1String("://images/node_selector/small-folder-outgoing_disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if(isRoot())
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/ico-cloud-drive.png"));
            return icon;
        }
        else
        {
            QIcon icon;
            icon.addFile(QLatin1String("://images/small_folder.png"), QSize(), QIcon::Normal);
            icon.addFile(QLatin1String("://images/node_selector/small-folder-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
    }
    else
    {
        return Utilities::getExtensionPixmapSmall(QString::fromUtf8(getNode()->getName()));
    }
}

int MegaItem::getStatus()
{
    return mStatus;
}

bool MegaItem::isSyncable()
{       
    return mStatus != SYNC
            && mStatus != SYNC_PARENT
            && mStatus != SYNC_CHILD;
}

void MegaItem::addNode(std::unique_ptr<MegaNode>node)
{
    mChildItems.append(new MegaItem(move(node), mShowFiles, this));
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

void MegaItem::setCameraFolder()
{
    mCameraFolder = true;
}

void MegaItem::setChatFilesFolder()
{
    mChatFilesFolder = true;
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
    QList<mega::MegaHandle> syncedFolders = Model::instance()->getMegaFolderHandles();
    if(syncedFolders.contains(mNode->getHandle()))
    {
        mStatus = STATUS::SYNC;
        return;
    }

    QString parentFolders;
    std::shared_ptr<MegaNode> n = mNode;
    parentFolders.append(QLatin1Char('/'));
    parentFolders.append(QString::fromUtf8(n->getName()));
    MegaApi* megaApi = MegaSyncApp->getMegaApi();
    while(n->getParentHandle() != INVALID_HANDLE)
    {
        n = std::shared_ptr<MegaNode>(megaApi->getNodeByHandle(n->getParentHandle()));
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
