#include "MegaItemProxyModel.h"
#include "MegaItem.h"
#include "megaapi.h"
#include "MegaItemModel.h"
#include "MegaApplication.h"

MegaItemProxyModel::MegaItemProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent)
{
    setSortCaseSensitivity(Qt::CaseInsensitive);
}

void MegaItemProxyModel::setFilter(const Filter &f)
{
    mFilter.showCloudDrive = f.showCloudDrive;
    mFilter.showInShares = f.showInShares;
    mFilter.showReadOnly = f.showReadOnly;
}

void MegaItemProxyModel::showOnlyVault()
{
    mFilter.showOnlyVault();
    invalidateFilter();
}

void MegaItemProxyModel::showOnlyCloudDrive()
{
    mFilter.showOnlyCloudDrive();
    invalidateFilter();
}

void MegaItemProxyModel::showOnlyInShares()
{
    mFilter.showOnlyInShares();
    invalidateFilter();
}

void MegaItemProxyModel::showReadOnlyFolders(bool value)
{
    mFilter.showReadOnly = value;
    invalidateFilter();
}

void MegaItemProxyModel::showReadWriteFolders(bool value)
{
    mFilter.showReadWriteFolders = value;
    invalidateFilter();
}

mega::MegaHandle MegaItemProxyModel::getHandle(const QModelIndex &index)
{
    auto node = getNode(index);
    return node ? node->getHandle() : mega::INVALID_HANDLE;
}

QModelIndex MegaItemProxyModel::getIndexFromSource(const QModelIndex& index)
{
    return mapToSource(index);
}

QModelIndex MegaItemProxyModel::getIndexFromHandle(const mega::MegaHandle& handle)
{
    if(handle == mega::INVALID_HANDLE)
    {
        return QModelIndex();
    }
    auto megaApi = MegaSyncApp->getMegaApi();
    auto node = std::shared_ptr<mega::MegaNode>(megaApi->getNodeByHandle(handle));
    QModelIndex ret = getIndexFromNode(node);

    return ret;
}

QVector<QModelIndex> MegaItemProxyModel::getRelatedModelIndexes(const std::shared_ptr<mega::MegaNode> node, bool isInShare)
{
    QVector<QModelIndex> ret;

    if(!node)
    {
        return ret;
    }
    auto parentNodeList = std::shared_ptr<mega::MegaNodeList>(mega::MegaNodeList::createInstance());
    parentNodeList->addNode(node.get());
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
    auto  rootNodeHandle = MegaSyncApp->getRootNode()->getHandle();

    std::shared_ptr<mega::MegaNode> this_node = node;
    while(this_node)
    {
        this_node.reset(megaApi->getParentNode(this_node.get()));
        if(this_node && (!isInShare || this_node->getHandle() != rootNodeHandle))
        {
            parentNodeList->addNode(this_node.get());
        }
    }
    ret.append(forEach(parentNodeList));

    return ret;
}

std::shared_ptr<mega::MegaNode> MegaItemProxyModel::getNode(const QModelIndex &index)
{
    if(!index.isValid())
    {
        return nullptr;
    }
    QModelIndex source_idx = mapToSource(index);
    if(!source_idx.isValid())
    {
        return nullptr;
    }
    MegaItem *item = static_cast<MegaItem*>(source_idx.internalPointer());
    if (!item)
    {
        return nullptr;
    }
    return item->getNode();
}

QModelIndex MegaItemProxyModel::insertNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex &parent)
{
    if(MegaItemModel* megaModel = getMegaModel())
    {
        QModelIndex source_idx = megaModel->insertNode(move(node), mapToSource(parent));
        return mapFromSource(source_idx);
    }
    return QModelIndex();
}

void MegaItemProxyModel::removeNode(const QModelIndex& item)
{
    if(MegaItemModel* megaModel = getMegaModel())
    {
        megaModel->removeNode(mapToSource(item));
    }
}

bool MegaItemProxyModel::isShowOnlyInShares()
{
    return mFilter.isShowOnlyInShares();
}

bool MegaItemProxyModel::isShowOnlyCloudDrive()
{
    return mFilter.isShowOnlyCloudDrive();
}

bool MegaItemProxyModel::isShowOnlyVault()
{
    return mFilter.isShowOnlyVault();
}

bool MegaItemProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool lIsFile = left.data(toInt(MegaItemModelRoles::IS_FILE_ROLE)).toBool();
    bool rIsFile = right.data(toInt(MegaItemModelRoles::IS_FILE_ROLE)).toBool();

    if(lIsFile && !rIsFile)
    {
        return false;
    }
    else if(!lIsFile && rIsFile)
    {
        return true;
    }
    if(left.column() == MegaItemModel::DATE && right.column() == MegaItemModel::DATE)
    {
        return left.data(toInt(MegaItemModelRoles::DATE_ROLE)) > right.data(toInt(MegaItemModelRoles::DATE_ROLE));
    }
    if(left.column() == MegaItemModel::STATUS && right.column() == MegaItemModel::STATUS)
    {
      int lStatus = left.data(toInt(MegaItemModelRoles::STATUS_ROLE)).toInt();
      int rStatus = right.data(toInt(MegaItemModelRoles::STATUS_ROLE)).toInt();
      if(lStatus != rStatus)
      {
        return lStatus < rStatus;
      }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


bool MegaItemProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    if(index.isValid())
    {
        if(MegaItem* megaItem = static_cast<MegaItem*>(index.internalPointer()))
        {
            if(std::shared_ptr<mega::MegaNode> node = megaItem->getNode())
            {
               QModelIndex parentIndex = index.parent();
               if(parentIndex.isValid())
               {
                   return filterAcceptsRow(index.row(), index);
               }
               mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
               int accs = megaApi->getAccess(node.get());
               if(node->isInShare())
               {
                    if((accs == mega::MegaShare::ACCESS_READ && !mFilter.showReadOnly)
                       || (accs == mega::MegaShare::ACCESS_READWRITE && !mFilter.showReadWriteFolders))
                    {
                        return false;
                    }
               }
               else if(megaItem->isVault())
               {
                    return !mFilter.showInShares && !mFilter.showCloudDrive;
               }
               return ((node->isInShare() && mFilter.showInShares)
                       || (!node->isInShare() && mFilter.showCloudDrive));
            }
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

bool MegaItemProxyModel::filterAcceptsColumn(int sourceColumn, const QModelIndex &sourceParent) const
{
    Q_UNUSED(sourceParent);
    switch(sourceColumn)
    { // Fallthrough
    case MegaItemModel::COLUMN::NODE:
    case MegaItemModel::COLUMN::STATUS:
    case MegaItemModel::COLUMN::DATE:
        return true;
    case MegaItemModel::COLUMN::USER:
        return mFilter.showInShares;
    }

    return false;
}

QVector<QModelIndex> MegaItemProxyModel::forEach(std::shared_ptr<mega::MegaNodeList> parentNodeList, QModelIndex parent)
{
    QVector<QModelIndex> ret;

    for(int j = parentNodeList->size()-1; j >= 0; --j)
    {
        auto handle = parentNodeList->get(j)->getHandle();
        for(int i = 0; i < sourceModel()->rowCount(parent); ++i)
        {
            QModelIndex index = sourceModel()->index(i, 0, parent);

            if(MegaItem* megaItem = static_cast<MegaItem*>(index.internalPointer()))
            {
                if(handle == megaItem->getNode()->getHandle())
                {
                    ret.append(mapFromSource(index));

                    auto interList = std::shared_ptr<mega::MegaNodeList>(mega::MegaNodeList::createInstance());
                    for(int k = 0; k < parentNodeList->size() - 1; ++k)
                    {
                        interList->addNode(parentNodeList->get(k));
                    }
                    ret.append(forEach(interList, index));
                    break;
                }
            }
        }
    }

    return ret;
}

QModelIndex MegaItemProxyModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> node)
{
    if(!node)
    {
        return QModelIndex();
    }
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    std::shared_ptr<mega::MegaNode> root_p_node = node;
    auto p_node = std::unique_ptr<mega::MegaNode>(megaApi->getParentNode(root_p_node.get()));
    while(p_node)
    {
        root_p_node = std::move(p_node);
        p_node.reset(megaApi->getParentNode(root_p_node.get()));
    }

    QVector<QModelIndex> indexList = getRelatedModelIndexes(node, root_p_node->isInShare());
    if(!indexList.isEmpty())
    {
        return indexList.last();
    }
    return QModelIndex();
}

MegaItemModel *MegaItemProxyModel::getMegaModel()
{
    return dynamic_cast<MegaItemModel*>(sourceModel());
}
