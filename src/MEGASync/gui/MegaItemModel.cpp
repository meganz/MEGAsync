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

const int MegaItemModel::ROW_HEIGHT = 20;

MegaItemModel::MegaItemModel(QObject *parent) :
    QAbstractItemModel(parent),
    mRequiredRights(MegaShare::ACCESS_READ),
    mDisplayFiles(false),
    mSyncSetupMode(false),
    mMegaApi(MegaSyncApp->getMegaApi())
{
   mCameraFolderAttribute = UserAttributes::CameraUploadFolder::requestCameraUploadFolder();
   mMyChatFilesFolderAttribute = UserAttributes::MyChatFilesFolder::requestMyChatFilesFolder();

   //cloud drive
   auto root = std::unique_ptr<MegaNode>(mMegaApi->getRootNode());
   mRootItems.append(new MegaItem(move(root)));

   //incoming shares
   auto folders = std::unique_ptr<MegaNodeList>(mMegaApi->getInShares());
   for (int j = 0; j < folders->size(); j++)
   {
       auto folder = std::unique_ptr<MegaNode>(folders->get(j)->copy());
       auto user = std::unique_ptr<MegaUser>(mMegaApi->getUserFromInShare(folder.get()));
       MegaItem* item = new MegaItem(move(folder));
       item->setOwner(move(user));
       connect(item, &MegaItem::infoUpdated, this, &MegaItemModel::onItemInfoUpdated);
       mRootItems.append(item);
   }

   // Get "My Backups" handle to localize the name
   auto myBackupsHandle = UserAttributes::MyBackupsHandle::requestMyBackupsHandle();
   connect(myBackupsHandle.get(), &UserAttributes::MyBackupsHandle::attributeReady,
           this, &MegaItemModel::onMyBackupsFolderHandleSet);
   onMyBackupsFolderHandleSet(myBackupsHandle->getMyBackupsHandle());
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
            return item->isRoot() || item->isVault()? -10 : 0;
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
        if (!item->areChildrenSet())
        {
            auto children = std::shared_ptr<MegaNodeList>(mMegaApi->getChildren(item->getNode().get()));
            item->setChildren(children);
        }
        return createIndex(row, column, item->getChild(row));
    }
    else
    {
        return createIndex(row, column, mRootItems.at(row));
    }
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
        if (!item->areChildrenSet())
        {
            auto children = std::shared_ptr<MegaNodeList>(mMegaApi->getChildren(item->getNode().get()));
            item->setChildren(children);
        }
        return item->getNumChildren();
    }
    return mRootItems.size();
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

void MegaItemModel::setSyncSetupMode(bool value)
{
    mSyncSetupMode = value;
}

void MegaItemModel::showFiles(bool show)
{
    mDisplayFiles = show;
    for(QList<MegaItem*>::iterator it = mRootItems.begin(); it != mRootItems.end();)
    {
        if((*it)->getNode()->isFile() && !show)
        {
            mRootItems.removeOne(*it);
            continue;
        }
        (*it)->displayFiles(show);
        ++it;
    }
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

MegaItemModel::~MegaItemModel()
{
    qDeleteAll(mRootItems);
    mRootItems.clear();
}

void MegaItemModel::onItemInfoUpdated(int role)
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
                        return;
                    }
                }
            }
        }
    }
}

void MegaItemModel::onMyBackupsFolderHandleSet(mega::MegaHandle h)
{
    if (h != INVALID_HANDLE)
    {
     auto node = mMegaApi->getNodeByHandle(h);
     auto megaItem = new MegaItem(std::unique_ptr<MegaNode>(node));
     megaItem->setAsVaultNode();
     beginInsertRows(QModelIndex(), rowCount(), rowCount());
     mRootItems.append(move(megaItem));
     endInsertRows();
    }
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

QIcon MegaItemModel::getFolderIcon(MegaItem *item) const
{
    if(!item)
    {
        return QIcon();
    }
    auto node = item->getNode();

    if(!node)
    {
        return QIcon();
    }
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
                std::unique_ptr<mega::MegaNode> parent (mMegaApi->getNodeByHandle(node->getParentHandle()));
                if (parent)
                {
                    QString parentDeviceId (QString::fromUtf8(parent->getDeviceId()));
                    if (parentDeviceId.isEmpty())
                    {
                        // TODO, future: choose icon according to host OS
                        if (nodeDeviceId == QString::fromUtf8(mMegaApi->getDeviceId()))
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
