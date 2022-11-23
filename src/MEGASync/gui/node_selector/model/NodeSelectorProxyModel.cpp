#include "NodeSelectorProxyModel.h"
#include "NodeSelectorModelItem.h"
#include "megaapi.h"
#include "NodeSelectorModel.h"
#include "MegaApplication.h"
#include "QThread"

NodeSelectorProxyModel::NodeSelectorProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent),
    mSortColumn(NodeSelectorModel::NODE),
    mOrder(Qt::AscendingOrder),
    mExpandMapped(true)
{
    mCollator.setCaseSensitivity(Qt::CaseInsensitive);
    mCollator.setNumericMode(true);
    mCollator.setIgnorePunctuation(false);

    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &NodeSelectorProxyModel::onModelSortedFiltered);

}

void NodeSelectorProxyModel::showReadOnlyFolders(bool value)
{
    mFilter.showReadOnly = value;
    invalidateFilter();
}

void NodeSelectorProxyModel::showReadWriteFolders(bool value)
{
    mFilter.showReadWriteFolders = value;
    invalidateFilter();
}

void NodeSelectorProxyModel::sort(int column, Qt::SortOrder order)
{
    mOrder = order;
    mSortColumn = column;

    //If it is already blocked, it is ignored.
    emit getMegaModel()->blockUi(true);
    emit layoutAboutToBeChanged();
    if(mFilterWatcher.isFinished())
    {
        QFuture<void> filtered = QtConcurrent::run([this, column, order](){
            auto itemModel = dynamic_cast<NodeSelectorModel*>(sourceModel());
            if(itemModel)
            {
                blockSignals(true);
                sourceModel()->blockSignals(true);
                invalidateFilter();
                QSortFilterProxyModel::sort(column, order);
                for (auto it = itemsToMap.rbegin(); it != itemsToMap.rend(); ++it)
                {
                    auto proxyIndex = mapFromSource((*it));
                    hasChildren(proxyIndex);
                }
                blockSignals(false);
                sourceModel()->blockSignals(false);
            }
        });
        mFilterWatcher.setFuture(filtered);
    }
}

mega::MegaHandle NodeSelectorProxyModel::getHandle(const QModelIndex &index)
{
    auto node = getNode(index);
    return node ? node->getHandle() : mega::INVALID_HANDLE;
}

QModelIndex NodeSelectorProxyModel::getIndexFromSource(const QModelIndex& index)
{
    return mapToSource(index);
}

QModelIndex NodeSelectorProxyModel::getIndexFromHandle(const mega::MegaHandle& handle)
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

QVector<QModelIndex> NodeSelectorProxyModel::getRelatedModelIndexes(const std::shared_ptr<mega::MegaNode> node)
{
    QVector<QModelIndex> ret;

    if(!node)
    {
        return ret;
    }
    auto parentNodeList = std::shared_ptr<mega::MegaNodeList>(mega::MegaNodeList::createInstance());
    parentNodeList->addNode(node.get());
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    std::shared_ptr<mega::MegaNode> this_node = node;
    while(this_node)
    {
        this_node.reset(megaApi->getParentNode(this_node.get()));
        if(this_node)
        {
            parentNodeList->addNode(this_node.get());
        }
    }
    ret.append(forEach(parentNodeList));

    return ret;
}

std::shared_ptr<mega::MegaNode> NodeSelectorProxyModel::getNode(const QModelIndex &index)
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
    NodeSelectorModelItem *item = static_cast<NodeSelectorModelItem*>(source_idx.internalPointer());
    if (!item)
    {
        return nullptr;
    }
    return item->getNode();
}

void NodeSelectorProxyModel::addNode(std::unique_ptr<mega::MegaNode> node, const QModelIndex &parent)
{
    if(NodeSelectorModel* megaModel = getMegaModel())
    {
        megaModel->addNode(move(node), mapToSource(parent));
    }
}

void NodeSelectorProxyModel::removeNode(const QModelIndex& item)
{
    if(NodeSelectorModel* megaModel = getMegaModel())
    {
        megaModel->removeNode(mapToSource(item));
    }
}

bool NodeSelectorProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool lIsFile = left.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();
    bool rIsFile = right.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)).toBool();

    if(lIsFile && !rIsFile)
    {
        return sortOrder() == Qt::DescendingOrder;
    }
    else if(!lIsFile && rIsFile)
    {
        return sortOrder() != Qt::DescendingOrder;
    }

    if(left.column() == NodeSelectorModel::DATE && right.column() == NodeSelectorModel::DATE)
    {
        return left.data(toInt(NodeSelectorModelRoles::DATE_ROLE)) < right.data(toInt(NodeSelectorModelRoles::DATE_ROLE));
    }
    if(left.column() == NodeSelectorModel::STATUS && right.column() == NodeSelectorModel::STATUS)
    {
      int lStatus = left.data(toInt(NodeSelectorModelRoles::STATUS_ROLE)).toInt();
      int rStatus = right.data(toInt(NodeSelectorModelRoles::STATUS_ROLE)).toInt();
      if(lStatus != rStatus)
      {
        return lStatus < rStatus;
      }
    }

    return mCollator.compare(left.data(Qt::DisplayRole).toString(),
                             right.data(Qt::DisplayRole).toString())<0;
}

void NodeSelectorProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if(auto nodeSelectorModel = dynamic_cast<NodeSelectorModel*>(sourceModel))
    {
        connect(nodeSelectorModel, &NodeSelectorModel::levelsAdded, this, &NodeSelectorProxyModel::invalidateModel);
        nodeSelectorModel->firstLoad();
    }
}

bool NodeSelectorProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if(index.isValid())
    {
        if(NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer()))
        {
            if(std::shared_ptr<mega::MegaNode> node = item->getNode())
            {
               if(node->isInShare())
               {
                   mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();
                   int accs = megaApi->getAccess(node.get());
                    if((accs == mega::MegaShare::ACCESS_READ && !mFilter.showReadOnly)
                       || (accs == mega::MegaShare::ACCESS_READWRITE && !mFilter.showReadWriteFolders))
                    {
                        return false;
                    }
               }
               return true;
            }
        }
    }
    return QSortFilterProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

QVector<QModelIndex> NodeSelectorProxyModel::forEach(std::shared_ptr<mega::MegaNodeList> parentNodeList, QModelIndex parent)
{
    QVector<QModelIndex> ret;

    for(int j = parentNodeList->size()-1; j >= 0; --j)
    {
        auto handle = parentNodeList->get(j)->getHandle();
        for(int i = 0; i < sourceModel()->rowCount(parent); ++i)
        {
            QModelIndex index = sourceModel()->index(i, 0, parent);

            if(NodeSelectorModelItem* item = static_cast<NodeSelectorModelItem*>(index.internalPointer()))
            {
                if(handle == item->getNode()->getHandle())
                {
                    ret.append(mapFromSource(index));

                    auto interList = std::shared_ptr<mega::MegaNodeList>(mega::MegaNodeList::createInstance());
                    for(int k = 0; k < parentNodeList->size(); ++k)
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

QModelIndex NodeSelectorProxyModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> node)
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

    QVector<QModelIndex> indexList = getRelatedModelIndexes(node/*, root_p_node->isInShare()*/);
    if(!indexList.isEmpty())
    {
        return indexList.last();
    }
    return QModelIndex();
}

NodeSelectorModel *NodeSelectorProxyModel::getMegaModel()
{
    return dynamic_cast<NodeSelectorModel*>(sourceModel());
}

bool NodeSelectorProxyModel::isModelProcessing() const
{
    return mFilterWatcher.isRunning();
}

bool NodeSelectorProxyModel::canBeDeleted() const
{
    return dynamic_cast<NodeSelectorModel*>(sourceModel())->canBeDeleted();
}

void NodeSelectorProxyModel::invalidateModel(const QModelIndexList& parents)
{
    itemsToMap = parents;
    sort(mSortColumn, mOrder);
}

void NodeSelectorProxyModel::onModelSortedFiltered()
{
    emit layoutChanged();

    if(mExpandMapped)
    {
        emit expandReady();
    }
    else
    {
        emit navigateReady(itemsToMap.isEmpty() ? QModelIndex() : mapFromSource(itemsToMap.first()));
        if(auto nodeSelectorModel = dynamic_cast<NodeSelectorModel*>(sourceModel()))
        {
            nodeSelectorModel->clearIndexesNodeInfo();
        }
    }

    emit getMegaModel()->blockUi(false);
    itemsToMap.clear();
}
