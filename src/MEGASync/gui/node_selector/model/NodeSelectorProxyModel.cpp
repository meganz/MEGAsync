#include "NodeSelectorProxyModel.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "NodeSelectorModel.h"
#include "QThread"

#include <QDebug>

NodeSelectorProxyModel::NodeSelectorProxyModel(QObject* parent) :
    QSortFilterProxyModel(parent),
    mSortColumn(NodeSelectorModel::NODE),
    mOrder(Qt::AscendingOrder),
    mExpandMapped(true),
    mForceInvalidate(false)
{
    mCollator.setCaseSensitivity(Qt::CaseInsensitive);
    mCollator.setNumericMode(true);
    mCollator.setIgnorePunctuation(false);

    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &NodeSelectorProxyModel::onModelSortedFiltered);
}

NodeSelectorProxyModel::~NodeSelectorProxyModel()
{

}

void NodeSelectorProxyModel::sort(int column, Qt::SortOrder order)
{
    mOrder = order;
    mSortColumn = column;

    //If it is already blocked, it is ignored.
    getMegaModel()->sendBlockUiSignal(true);

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
                for (auto it = mItemsToMap.crbegin(); it != mItemsToMap.crend(); ++it)
                {
                    auto proxyIndex = mapFromSource((*it));
                    hasChildren(proxyIndex);
                }
                mItemsToMap.clear();
                if(mForceInvalidate)
                {
                    invalidate();
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

std::shared_ptr<mega::MegaNode> NodeSelectorProxyModel::getNode(const QModelIndex &index)
{
    if(!index.isValid())
    {
        return nullptr;
    }
    return qvariant_cast<std::shared_ptr<mega::MegaNode>>(index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
}

void NodeSelectorProxyModel::deleteNode(const QModelIndex& item)
{
    if(NodeSelectorModel* megaModel = getMegaModel())
    {
        megaModel->deleteNodeFromModel(mapToSource(item));
    }
}

bool NodeSelectorProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    auto lIsFileVar(left.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)));
    auto rIsFileVar(right.data(toInt(NodeSelectorModelRoles::IS_FILE_ROLE)));

    // Logic to put the empty space always at the bottom
    {
        if (!lIsFileVar.isValid())
        {
            return sortOrder() == Qt::DescendingOrder;
        }

        if (!rIsFileVar.isValid())
        {
            return sortOrder() != Qt::DescendingOrder;
        }
    }

    bool lIsFile = lIsFileVar.toBool();
    bool rIsFile = rIsFileVar.toBool();

    auto result(false);

    if(lIsFile && !rIsFile)
    {
        result = sortOrder() == Qt::DescendingOrder;
    }
    else if(!lIsFile && rIsFile)
    {
        result = sortOrder() != Qt::DescendingOrder;
    }
    else
    {
        if(left.column() == NodeSelectorModel::DATE && right.column() == NodeSelectorModel::DATE)
        {
            result = left.data(toInt(NodeSelectorModelRoles::DATE_ROLE)).value<int64_t>() < right.data(toInt(NodeSelectorModelRoles::DATE_ROLE)).value<int64_t>();
        }
        else
        {
            int lStatus(0);
            int rStatus(0);

            if(left.column() == NodeSelectorModel::STATUS && right.column() == NodeSelectorModel::STATUS)
            {
                lStatus = left.data(toInt(NodeSelectorModelRoles::STATUS_ROLE)).toInt();
                rStatus = right.data(toInt(NodeSelectorModelRoles::STATUS_ROLE)).toInt();
            }

            if(lStatus != rStatus)
            {
                result = lStatus < rStatus;
            }
            else if(left.column() == NodeSelectorModel::USER && right.column() == NodeSelectorModel::USER)
            {
                result = mCollator.compare(left.data(Qt::ToolTipRole).toString(),
                                           right.data(Qt::ToolTipRole).toString()) < 0;
            }
            else if (left.column() == NodeSelectorModel::ACCESS &&
                     right.column() == NodeSelectorModel::ACCESS)
            {
                result = left.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt() <
                         right.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt();
            }
            else
            {
                result = mCollator.compare(left.data(Qt::DisplayRole).toString(),
                                           right.data(Qt::DisplayRole).toString())<0;
            }
        }

    }

    return result;
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

QModelIndex NodeSelectorProxyModel::findIndexInParentList(mega::MegaNode* NodeToFind,
                                                          QModelIndex sourceModelParent)
{
    auto handle = NodeToFind->getHandle();

    for (int i = 0; i < sourceModel()->rowCount(sourceModelParent); ++i)
    {
        QModelIndex index = sourceModel()->index(i, 0, sourceModelParent);

        if (NodeSelectorModelItem* item =
                static_cast<NodeSelectorModelItem*>(index.internalPointer()))
        {
            if (handle == item->getNode()->getHandle())
            {
                return mapFromSource(index);
            }
        }
    }

    return QModelIndex();
}

QModelIndex
    NodeSelectorProxyModel::getIndexFromNode(const std::shared_ptr<mega::MegaNode> nodeToFind)
{
    if (!nodeToFind)
    {
        return QModelIndex();
    }

    auto parentNodeList = std::shared_ptr<mega::MegaNodeList>(mega::MegaNodeList::createInstance());
    parentNodeList->addNode(nodeToFind.get());
    mega::MegaApi* megaApi = MegaSyncApp->getMegaApi();

    std::shared_ptr<mega::MegaNode> this_node = nodeToFind;
    while (this_node)
    {
        this_node.reset(megaApi->getParentNode(this_node.get()));
        if (this_node)
        {
            parentNodeList->addNode(this_node.get());
        }
    }

    QModelIndex foundIndex;

    // Start from top parent to last child
    for (int i = parentNodeList->size() - 1; i >= 0; --i)
    {
        auto nodeFromList(parentNodeList->get(i));
        foundIndex = findIndexInParentList(nodeFromList, mapToSource(foundIndex));
        if (foundIndex.isValid() && nodeFromList->getHandle() == nodeToFind->getHandle())
        {
            return foundIndex;
        }
    }

    return QModelIndex();
}

NodeSelectorModel* NodeSelectorProxyModel::getMegaModel() const
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

bool NodeSelectorProxyModel::hasContextMenuOptions(const QModelIndexList& indexes) const
{
    for (const auto& index: indexes)
    {
        auto indexItem(getMegaModel()->getItemByIndex(mapToSource(index)));
        if (indexItem && indexItem->getNode())
        {
            if (indexItem->isRubbishBin() || indexItem->isVault() || indexItem->isVaultDevice())
            {
                return false;
            }
        }
    }

    return true;
}

void NodeSelectorProxyModel::invalidateModel(const QList<QPair<mega::MegaHandle,QModelIndex>>& parents, bool force)
{
    foreach(auto parent, parents)
    {
        mItemsToMap.append(parent.second);
    }
    mForceInvalidate = force;
    sort(mSortColumn, mOrder);
}

void NodeSelectorProxyModel::onModelSortedFiltered()
{
    if(mForceInvalidate)
    {
        if(auto nodeSelectorModel = dynamic_cast<NodeSelectorModel*>(sourceModel()))
        {
            nodeSelectorModel->proxyInvalidateFinished();
        }
    }

    mForceInvalidate = false;

    emit layoutChanged();

    if(mExpandMapped)
    {
        emit expandReady();
    }
    else
    {
        emit navigateReady(mItemsToMap.isEmpty() ? QModelIndex() :
                                                   mapFromSource(mItemsToMap.first()));
        mExpandMapped = true;
    }

    emit modelSorted();

    getMegaModel()->sendBlockUiSignal(false);
    mItemsToMap.clear();
}

NodeSelectorProxyModelSearch::NodeSelectorProxyModelSearch(QObject *parent)
    : NodeSelectorProxyModel(parent), mMode(NodeSelectorModelItemSearch::Type::NONE)
{

}

void NodeSelectorProxyModelSearch::setMode(NodeSelectorModelItemSearch::Types mode)
{
    if(mMode == mode)
    {
        return;
    }

    getMegaModel()->sendBlockUiSignal(true);
    mMode = mode;
    invalidateFilter();
    getMegaModel()->sendBlockUiSignal(false);
    if (rowCount() == 0)
    {
        emit modeEmpty();
    }
}

bool NodeSelectorProxyModelSearch::canBeDeleted() const
{
    if (mMode & NodeSelectorModelItemSearch::Type::BACKUP)
    {
        return false;
    }
    return NodeSelectorProxyModel::canBeDeleted();
}

bool NodeSelectorProxyModelSearch::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    if(mMode == static_cast<int>(NodeSelectorModelItemSearch::Type::NONE))
    {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if(index.isValid())
    {
        if(NodeSelectorModelItemSearch* item = static_cast<NodeSelectorModelItemSearch*>(index.internalPointer()))
        {
            return mMode & item->getType();
        }
    }

    return NodeSelectorProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}

