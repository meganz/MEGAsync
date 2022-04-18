#include "MegaItem.h"
#include "QMegaMessageBox.h"
#include "MegaApplication.h"
#include "AvatarWidget.h"
#include "SyncModel.h"
#include "MegaApplication.h"
#include "mega/utils.h"

#include <QByteArray>


const int MegaItem::ICON_SIZE = 17;

using namespace mega;

MegaItem::MegaItem(std::unique_ptr<MegaNode> node, MegaItem *parentItem, bool showFiles) :
    QObject(parentItem),
    mShowFiles(showFiles),
    mOwnerFirstName(QString::fromUtf8("")),
    mOwnerLastName(QString::fromUtf8("")),
    mOwnerEmail(QString::fromUtf8("")),
    mOwnerIcon(QPixmap()),
    mStatus(STATUS::NONE),
    mCameraFolder(false),
    mChatFilesFolder(false),
    mChildrenSetted(false),
    mNode(std::move(node)),
    mOwner(nullptr),
    mDelegateListener(mega::make_unique<QTMegaRequestListener>(static_cast<MegaApplication*>(qApp)->getMegaApi(), this))
{ 
    if(isRoot() || mNode->isFile() || mNode->isInShare())
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
        foreach(const QString& folder, SyncModel::instance()->getMegaFolders(SyncModel::AllHandledSyncTypes))
        {
            if(folder.startsWith(parent_item->getOwnerEmail()))
            {
                folderList.append(folder.split(QString::fromUtf8(":")).last().prepend(QString::fromUtf8("/")));
            }
        }
        calculateSyncStatus(folderList);
    }
    ////////////
    else
    {
        calculateSyncStatus(SyncModel::instance()->getMegaFolders(SyncModel::AllHandledSyncTypes));
    }
}


std::shared_ptr<mega::MegaNode> MegaItem::getNode()
{
    return mNode;
}

void MegaItem::setChildren(MegaNodeList *children)
{
    this->mChildrenSetted = true;
    for (int i = 0; i < children->size(); i++)
    {
        auto node = std::unique_ptr<MegaNode>(children->get(i));
        if (!mShowFiles && node->getType() == MegaNode::TYPE_FILE)
        {
            break;
        }
        mChildItems.append(new MegaItem(move(node), this, mShowFiles));
    }
}

bool MegaItem::areChildrenSet()
{
    return mChildrenSetted;
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
    if(!mOwnerFirstName.isEmpty() && !mOwnerLastName.isEmpty())
    {
        return mOwnerFirstName + QString::fromUtf8(" ") + mOwnerLastName;
    }
    return mOwnerEmail;
}

QString MegaItem::getOwnerEmail()
{
    return mOwnerEmail;
}

void MegaItem::setOwner(std::unique_ptr<mega::MegaUser> user)
{
    mOwner = move(user);
    mOwnerEmail = QString::fromUtf8(mOwner->getEmail());
    MegaApi* megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
    if(megaApi)
    {
        megaApi->getUserAttribute(mOwner.get(), mega::MegaApi::USER_ATTR_FIRSTNAME, mDelegateListener.get());
        megaApi->getUserAttribute(mOwner.get(), mega::MegaApi::USER_ATTR_LASTNAME, mDelegateListener.get());
        megaApi->getUserAvatar(mOwner.get(), Utilities::getAvatarPath(mOwnerEmail).toUtf8().constData(), mDelegateListener.get());
    }
    QStringList folderList;
    //Calculating if we have a synced childs.
    foreach(const QString& folder, SyncModel::instance()->getMegaFolders(SyncModel::AllHandledSyncTypes))
    {
        if(folder.startsWith(mOwnerEmail))
        {
            folderList.append(folder.split(QString::fromUtf8(":")).last().prepend(QString::fromUtf8("/")));
        }
    }
    calculateSyncStatus(folderList);
}

QPixmap MegaItem::getOwnerIcon()
{
    return mOwnerIcon;
}

QIcon MegaItem::getStatusIcons()
{
    QIcon statusIcons; //first is selected state icon / second is normal state icon
    switch(mStatus)
    {
    case STATUS::SYNC:
    {
        statusIcons.addFile(QString::fromAscii("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //normal style icon
        statusIcons.addFile(QString::fromAscii("://images/Item-sync-rest.png"), QSize(), QIcon::Normal); //normal style icon
        break;
    }
    case STATUS::SYNC_PARENT:
    {
        statusIcons.addFile(QString::fromAscii("://images/Item-sync-press.png"), QSize(), QIcon::Selected); //normal style icon
        statusIcons.addFile(QString::fromAscii("://images/node_selector/icon-small-sync-disabled.png"), QSize(), QIcon::Normal); //normal style icon
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
            icon.addFile(QString::fromAscii("://images/node_selector/small-camera-sync.png"), QSize(), QIcon::Normal);
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-camera-sync-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if(mChatFilesFolder)
        {
            QIcon icon;
            icon.addFile(QString::fromAscii("://images/node_selector/small-chat-files.png"), QSize(), QIcon::Normal);
            icon.addFile(QString::fromAscii("://images/node_selector/small-chat-files-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if (getNode()->isInShare())
        {
            QIcon icon;
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-incoming.png"), QSize(), QIcon::Normal);
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-incoming-disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if (getNode()->isOutShare())
        {
            QIcon icon;
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-outgoing.png"), QSize(), QIcon::Normal);
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-outgoing_disabled.png"), QSize(), QIcon::Disabled);
            return icon;
        }
        else if(isRoot())
        {
            QIcon icon;
            icon.addFile(QString::fromAscii("://images/ico-cloud-drive.png"));
            return icon;
        }
        else
        {
            QIcon icon;
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder.png"), QSize(), QIcon::Normal);
            icon.addFile(QString::fromAscii("://images/node_selector/small-folder-disabled.png"), QSize(), QIcon::Disabled);
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

int MegaItem::insertPosition(const std::unique_ptr<MegaNode>& node)
{
    int type = node->getType();

    int i;
    for (i = 0; i < mChildItems.size(); i++)
    {
        std::shared_ptr<MegaNode> n = mChildItems.at(i)->getNode();
        int nodeType = n->getType();
        if (type < nodeType)
        {
            continue;
        }

        if (qstricmp(node->getName(), n->getName()) <= 0)
        {
            break;
        }
    }

    return i;
}

void MegaItem::insertNode(std::unique_ptr<MegaNode>node, int index)
{
    mChildItems.insert(index, new MegaItem(move(node), this, mShowFiles));
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
            item = nullptr;
            return;
        }
    }
}

void MegaItem::displayFiles(bool enable)
{
    this->mShowFiles = enable;
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

void MegaItem::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
{
    Q_UNUSED(api);
    if (e->getErrorCode() != MegaError::API_OK && e->getErrorCode() != MegaError::API_ENOENT)
    {
        QMegaMessageBox::critical(nullptr, QString::fromUtf8("MEGAsync"), tr("Error") +
                                  QString::fromUtf8(": ") + QCoreApplication::translate("MegaError", e->getErrorString()));
        return;
    }
    if(request->getType() == mega::MegaRequest::TYPE_GET_ATTR_USER)
    {
        switch(request->getParamType())
        {
        case mega::MegaApi::USER_ATTR_FIRSTNAME:
        {
            mOwnerFirstName = QString::fromUtf8(request->getText());
            emit infoUpdated(Qt::DisplayRole);
            break;
        }
        case mega::MegaApi::USER_ATTR_LASTNAME:
        {
            mOwnerLastName = QString::fromUtf8(request->getText());
            emit infoUpdated(Qt::DisplayRole);
            break;
        }
        case mega::MegaApi::USER_ATTR_AVATAR:
        {
            QString fileRoute = QString::fromUtf8(request->getFile());
            #ifdef WIN32
            if (fileRoute.startsWith(QString::fromUtf8("\\\\?\\")))
            {
                fileRoute = fileRoute.mid(4);
            }
            #endif

            if(e->getErrorCode() != MegaError::API_ENOENT)
            {
                mOwnerIcon = AvatarPixmap::maskFromImagePath(fileRoute, ICON_SIZE);
            }
            else
            {
                QFile::remove(fileRoute);
                const char* color = nullptr;
                if(mega::MegaApi* megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi())
                {
                    color = megaApi->getUserAvatarColor(mOwner.get());
                }
                QColor avatarColor(color);
                delete [] color;
                QLinearGradient gradient(ICON_SIZE, ICON_SIZE, ICON_SIZE, ICON_SIZE);
                gradient.setColorAt(1.0, avatarColor.lighter(130));
                gradient.setColorAt(0.0, avatarColor);
                mOwnerIcon = AvatarPixmap::createFromLetter(getOwnerName().at(0), gradient, ICON_SIZE);
            }
            emit infoUpdated(Qt::DecorationRole);
            break;
        }
        default:
            break;
        }
    }
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
    parentFolders.append(QString::fromUtf8("/"));
    parentFolders.append(QString::fromUtf8(n->getName()));
    MegaApi* megaApi = static_cast<MegaApplication*>(qApp)->getMegaApi();
    while(n->getParentHandle () != INVALID_HANDLE)
    {
        n = std::shared_ptr<MegaNode>(megaApi->getNodeByHandle(n->getParentHandle()));
        if(n->getType() != MegaNode::TYPE_ROOT)
        {
            parentFolders.prepend(QString::fromUtf8(n->getName()));
            parentFolders.prepend(QString::fromUtf8("/"));
        }
    }

    foreach(const QString& syncFolder, folders)
    {
        if(syncFolder.startsWith(parentFolders))
        {
            mStatus = STATUS::SYNC_PARENT;
        }
        else if(parentFolders.startsWith(syncFolder))
        {
            mStatus = STATUS::SYNC_CHILD;
        }
    }
}


bool MegaItem::isRoot()
{
    return getNode()->getHandle() == static_cast<MegaApplication*>(qApp)->getRootNode()->getHandle();
}
