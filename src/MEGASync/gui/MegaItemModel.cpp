#include "MegaItemModel.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "model/SyncModel.h"
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
    mMegaApi(MegaSyncApp->getMegaApi()),
    mDelegateListener(mega::make_unique<QTMegaRequestListener>(mMegaApi, this))
{
   auto root = std::unique_ptr<MegaNode>(mMegaApi->getRootNode());

   //cloud drive
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

   //backups vault
   if(auto vaultNode = std::unique_ptr<MegaNode>(mMegaApi->getVaultNode()))
       mRootItems.append(new MegaItem(move(vaultNode)));

   mMegaApi->getCameraUploadsFolder(mDelegateListener.get());
   mMegaApi->getCameraUploadsFolderSecondary(mDelegateListener.get());
   mMegaApi->getMyChatFilesFolder(mDelegateListener.get());

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
                return item->isSyncable();

            return true;
        }
        case toInt(NodeRowDelegateRoles::INDENT_ROLE):
        {
            return item->isRoot() || item->isVault()? -10: 0;
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

QModelIndex MegaItemModel::insertNode(std::unique_ptr<MegaNode> node, const QModelIndex &parent)
{
    if(!parent.isValid())
    {
        int index = insertPosition(node);
        MegaItem *item = new MegaItem(move(node));
        beginInsertRows(QModelIndex(), index, index);
        mRootItems.insert(index, item);
        endInsertRows();

        return this->index(index, 0, QModelIndex());
    }

    MegaItem *parentItem = static_cast<MegaItem*>(parent.internalPointer());
    int index = parentItem->insertPosition(node);
    beginInsertRows(parent, index, index);
    parentItem->insertNode(move(node), index);
    endInsertRows();

    return this->index(index, 0, parent);
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
            //To remove when SDK returns correct names for this nodes
            if(item->isVault())
                return QCoreApplication::translate("MegaNodeNames", "Backups");

            if(item->isRoot())
                return QApplication::translate("MegaNodeNames", item->getNode()->getName());
                
            return QVariant(QString::fromUtf8(item->getNode()->getName()));
        }
        case COLUMN::DATE:
        {
            if(item->isRoot() || item->isVault())
                return QVariant();

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

void MegaItemModel::onRequestFinish(mega::MegaApi *api, mega::MegaRequest *request, mega::MegaError *e)
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
            QModelIndex rootIndex = index(0,0);
            for(int i = 0; i < rowCount(rootIndex); ++i)
            {
                //we only update this column because we retrieve the data in async mode
                //so it is possible that we doesn´t have the information from the start
                QModelIndex idx = index(i, COLUMN::NODE, rootIndex);
                if(idx.isValid())
                {
                    if(MegaItem* chkItem = static_cast<MegaItem*>(idx.internalPointer()))
                    {
                        if(chkItem->getNode()->getHandle() == request->getNodeHandle())
                        {
                            if(request->getParamType() == mega::MegaApi::USER_ATTR_CAMERA_UPLOADS_FOLDER)
                            {
                                chkItem->setCameraFolder();
                            }
                            else
                            {
                                chkItem->setChatFilesFolder();
                            }

                            QVector<int> roles;
                            roles.append(Qt::DecorationRole);
                            emit dataChanged(idx, idx, roles);
                            return;
                        }
                    }
                }
            }
            break;
        }
        default:
            break;
        }
    }
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
