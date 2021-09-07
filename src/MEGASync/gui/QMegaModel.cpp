#include "QMegaModel.h"
#include "MegaApplication.h"
#include "control/Utilities.h"

#include <QBrush>
#include <QApplication>

using namespace mega;

QMegaModel::QMegaModel(mega::MegaApi *megaApi, QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi (megaApi),
    mRootNode (MegaSyncApp->getRootNode()),
    mRootItem (new MegaItem(mRootNode.get())),
    mFolderIcon (QIcon(QLatin1String("://images/small_folder.png"))),
    mRequiredRights (MegaShare::ACCESS_READ),
    mDisplayFiles (false),
    mDisableFolders (false)
{
    std::unique_ptr<MegaUserList> contacts (megaApi->getContacts());
    for (int i = 0; i < contacts->size(); i++)
    {
        MegaUser* contact (contacts->get(i));
        std::unique_ptr<MegaNodeList> folders (megaApi->getInShares(contact));
        for (int j = 0; j < folders->size(); j++)
        {
            mega::MegaNode* folder (folders->get(j)->copy());
            mOwnNodes.append(folder);
            mInshareItems.append(new MegaItem(folder));
            mInshareOwners.append(QString::fromUtf8(contact->getEmail()));
        }
    }
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
            MegaNode* node = item->getNode();
            if (node->getType() >= MegaNode::TYPE_FOLDER)
            {
                if (node->isInShare())
                {
                    return QIcon(QLatin1String("://images/small_folder_incoming.png"));;
                }
                else if (node->isOutShare())
                {
                    return QIcon(QLatin1String("://images/small_folder_outgoing.png"));;
                }

                return mFolderIcon;
            }

            return Utilities::getExtensionPixmapSmall(QString::fromUtf8(node->getName()));
        }
        case Qt::ForegroundRole:
        {
            int access = mMegaApi->getAccess(item->getNode());
            if (access < mRequiredRights || (mDisableFolders && item->getNode()->isFolder()))
            {
                return QVariant(QBrush(QColor(170, 170, 170, 127)));
            }

            return QVariant();
        }
        case Qt::DisplayRole:
        {
            if (item->getParent())
            {
                return QVariant(QString::fromUtf8(item->getNode()->getName()));
            }
            else if (item->getNode()->getType() == MegaNode::TYPE_ROOT)
            {
                return QVariant(QCoreApplication::translate("MegaNodeNames",
                                                            item->getNode()->getName()));
            }

            int inshareIndex = index.row() - 1;
            return QVariant(QString::fromUtf8("%1 (%2)").arg(
                                QString::fromUtf8(mInshareItems.at(inshareIndex)->getNode()->getName()),
                                mInshareOwners.at(inshareIndex)));
        }
        default:
        {
            return QVariant();
        }
    }
#ifndef WIN32
    return QVariant(); // warning C4702: unreachable code
#endif
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
            item->setChildren(mMegaApi->getChildren(item->getNode()));
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
        MegaItem*item = static_cast<MegaItem*>(parent.internalPointer());
        if (!item->areChildrenSet())
        {
            item->setChildren(mMegaApi->getChildren(item->getNode()));
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

void QMegaModel::showFiles(bool show)
{
    mDisplayFiles = show;
    mRootItem->displayFiles(show);
    for (int i = 0; i < mInshareItems.size(); i++)
    {
        mInshareItems.at(i)->displayFiles(show);
    }
}

QModelIndex QMegaModel::insertNode(MegaNode* node, const QModelIndex& parent)
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
    MegaNode* node = (static_cast<MegaItem*>(item.internalPointer()))->getNode();
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

MegaNode* QMegaModel::getNode(const QModelIndex& index)
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
    qDeleteAll(mOwnNodes);
    qDeleteAll(mInshareItems);
}
