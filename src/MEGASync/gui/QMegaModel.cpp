#include "QMegaModel.h"
#include "MegaApplication.h"
#include "control/Utilities.h"
#include "control/SyncController.h"

#include <QBrush>
#include <QApplication>

#include <memory>

QMegaModel::QMegaModel(mega::MegaApi *megaApi, QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi (megaApi),
    mRootNode (MegaSyncApp->getRootNode()),
    mRootItem (new MegaItem(mRootNode)),
    mRequiredRights (mega::MegaShare::ACCESS_READ),
    mDisplayFiles (false),
    mDisableFolders (false),
    mDisableBackups (false),
    mMyBackupsRootDirHandle (mega::INVALID_HANDLE),
    mDeviceId (QString::fromUtf8(mMegaApi->getDeviceId()))
{
    std::unique_ptr<mega::MegaUserList> contacts (megaApi->getContacts());
    for (int i = 0; i < contacts->size(); i++)
    {
        mega::MegaUser* contact (contacts->get(i));
        std::unique_ptr<mega::MegaNodeList> folders (megaApi->getInShares(contact));
        for (int j = 0; j < folders->size(); j++)
        {
            std::shared_ptr<mega::MegaNode> folder (folders->get(j)->copy());
            mOwnNodes.append(folder);
            mInshareItems.append(new MegaItem(folder));
            mInshareOwners.append(QString::fromUtf8(contact->getEmail()));
        }
    }

    connect(&mSyncController, &SyncController::myBackupsHandle,
            this, &QMegaModel::onMyBackupsRootDir);
    mSyncController.getMyBackupsHandle();
}

int QMegaModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant QMegaModel::data(const QModelIndex& index, int role) const
{
    if (!index.isValid())
    {
        return QVariant();
    }

    MegaItem* item = static_cast<MegaItem*>(index.internalPointer());
    if (!item->getNode())
    {
        return QVariant();
    }

    switch(role)
    {
        case Qt::DecorationRole:
        {
            auto node (item->getNode());
            if (node->getType() >= mega::MegaNode::TYPE_FOLDER)
            {
                if (node->isInShare())
                {
                    static const QIcon incomingFolderIcon (QIcon(QLatin1String("://images/small_folder_incoming.png")));
                    return incomingFolderIcon;
                }
                else if (node->isOutShare())
                {
                    static const QIcon outgoingFolderIcon (QIcon(QLatin1String("://images/small_folder_outgoing.png")));
                    return outgoingFolderIcon;
                }
                else if (node->getHandle() == mMyBackupsRootDirHandle)
                {
                    static const QIcon myBackupsRootIcon (QIcon(QLatin1String("://images/backup_mono.png")));
                    return myBackupsRootIcon;
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
                                if (nodeDeviceId == mDeviceId)
                                {
#ifdef Q_OS_WINDOWS
static const QIcon thisDeviceIcon (QIcon(QLatin1String("://images/icons/pc/pc-win_24.png")));
#elif defined(Q_OS_MACOS)
static const QIcon thisDeviceIcon (QIcon(QLatin1String("://images/icons/pc/pc-mac_24.png")));
#elif defined(Q_OS_LINUX)
static const QIcon thisDeviceIcon (QIcon(QLatin1String("://images/icons/pc/pc-linux_24.png")));
#endif
                                    return thisDeviceIcon;
                                }
                                else
                                {
                                    static const QIcon otherDeviceIcon (QIcon(QLatin1String("://images/icons/pc/pc_24.png")));
                                    return otherDeviceIcon;
                                }
                            }
                        }
                    }
                }
                static const QIcon defaultFolderIcon (QIcon(QLatin1String("://images/small_folder.png")));
                return defaultFolderIcon;
            }

            return Utilities::getExtensionPixmapSmall(QString::fromUtf8(node->getName()));
        }
        case Qt::ForegroundRole:
        {
            auto node (item->getNode());
            QString nodeDeviceId (QString::fromUtf8(node->getDeviceId()));
            int access = mMegaApi->getAccess(node.get());
            if (access < mRequiredRights || (mDisableFolders && node->isFolder())
                    || (mDisableBackups && (!nodeDeviceId.isEmpty()
                                            || node->getHandle() == mMyBackupsRootDirHandle)))
            {
                static const QBrush disabledBrush (QBrush(QColor(170, 170, 170, 127)));
                return QVariant(disabledBrush);
            }
            return QVariant();
        }
        case Qt::DisplayRole:
        {
            if (item->getParent())
            {
                return QVariant(QString::fromUtf8(item->getNode()->getName()));
            }
            else if (item->getNode()->getType() == mega::MegaNode::TYPE_ROOT)
            {
                return QVariant(QCoreApplication::translate("MegaNodeNames",
                                                            item->getNode()->getName()));
            }

            int inshareIndex = index.row() - 1;
            return QVariant(QString::fromUtf8("%1 (%2)").arg(
                                QString::fromUtf8(mInshareItems.at(inshareIndex)->getNode()->getName()),
                                mInshareOwners.at(inshareIndex)));
        }
        case Qt::UserRole:
        {
            auto node (item->getNode());
            QString nodeDeviceId (QString::fromUtf8(node->getDeviceId()));
            int access = mMegaApi->getAccess(node.get());
            if (access < mRequiredRights || (mDisableFolders && node->isFolder())
                    || (mDisableBackups && (!nodeDeviceId.isEmpty()
                                            || node->getHandle() == mMyBackupsRootDirHandle)))
            {
                return false;
            }
            return true;
        }
    }
    return QVariant();
}

QModelIndex QMegaModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!hasIndex(row, column, parent))
    {
        return QModelIndex();
    }

    if (parent.isValid())
    {
        MegaItem* item = nullptr;
        item = static_cast<MegaItem*>(parent.internalPointer());

        if (!item->areChildrenSet())
        {
            item->setChildren(std::shared_ptr<mega::MegaNodeList>(mMegaApi->getChildren(item->getNode().get())));
        }

        return createIndex(row, column, item->getChild(row));
    }

    if (row == 0)
    {
        return createIndex(row, column, mRootItem.get());
    }

    return createIndex(row, column, mInshareItems.at(row - 1));
}

QModelIndex QMegaModel::parent(const QModelIndex& index) const
{
    MegaItem* item = static_cast<MegaItem*>(index.internalPointer());
    if (!item)
    {
        return QModelIndex();
    }

    MegaItem* parent = item->getParent();
    if (!parent)
    {
        return QModelIndex();
    }

    MegaItem* grandparent = parent->getParent();
    if (!grandparent)
    {
        if (parent == mRootItem.get())
        {
            return createIndex(0, 0, parent);
        }

        return createIndex(1 + mInshareItems.indexOf(parent), 0, parent);
    }

    return createIndex(grandparent->indexOf(parent), 0, parent);
}

int QMegaModel::rowCount(const QModelIndex& parent) const
{
    if (parent.isValid())
    {
        MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
        if (!item->areChildrenSet())
        {
            item->setChildren(std::shared_ptr<mega::MegaNodeList>(mMegaApi->getChildren(item->getNode().get())));
        }

        return item->getNumChildren();
    }

    return mInshareItems.size() + 1;
}

void QMegaModel::setRequiredRights(int requiredRights)
{
    mRequiredRights = requiredRights;
}

void QMegaModel::setDisableFolders(bool option)
{
    mDisableFolders = option;
}

void QMegaModel::setDisableBackups(bool option)
{
    mDisableBackups = option;
}

void QMegaModel::showFiles(bool show)
{
    mDisplayFiles = show;
    mRootItem->displayFiles(show);
    for (int i = 0; i < mInshareItems.size(); i++)
    {
        mInshareItems.at(i)->displayFiles(show);
    }
}

QModelIndex QMegaModel::insertNode(std::shared_ptr<mega::MegaNode> node, const QModelIndex& parent)
{
    MegaItem* item = static_cast<MegaItem*>(parent.internalPointer());
    int idx = item->insertPosition(node);

    beginInsertRows(parent, idx, idx);
    item->insertNode(node, idx);
    endInsertRows();

    return index(idx, 0, parent);
}

void QMegaModel::removeNode(QModelIndex& item)
{
    auto node = (static_cast<MegaItem*>(item.internalPointer()))->getNode();
    MegaItem* parent = static_cast<MegaItem*>(item.parent().internalPointer());
    if (!node || !parent)
    {
        return;
    }
    int index = parent->indexOf(static_cast<MegaItem*>(item.internalPointer()));

    beginRemoveRows(item.parent(), index, index);
    parent->removeNode(node);
    endRemoveRows();
}

std::shared_ptr<mega::MegaNode> QMegaModel::getNode(const QModelIndex& index)
{
    MegaItem* item = static_cast<MegaItem*>(index.internalPointer());
    if (!item)
    {
        return nullptr;
    }
    return item->getNode();
}

QMegaModel::~QMegaModel()
{
    qDeleteAll(mInshareItems);
}

void QMegaModel::onMyBackupsRootDir(mega::MegaHandle handle)
{
    mMyBackupsRootDirHandle = handle;
}
