#include "NodeSelectorProxyModel.h"

#include "megaapi.h"
#include "MegaApplication.h"
#include "NodeSelectorModel.h"
#include "QThread"

#include <QCoreApplication>
#include <QDebug>
#include <QScopeGuard>

NodeSelectorProxyModel::NodeSelectorProxyModel(QObject* parent):
    QSortFilterProxyModel(parent),
    mSortColumn(NodeSelectorModel::Column::NODE),
    mOrder(Qt::AscendingOrder),
    mExpandMapped(true),
    mForceInvalidate(false),
    mPendingSortIsLevelLoad(false)
{
    mCollator.setCaseSensitivity(Qt::CaseInsensitive);
    mCollator.setNumericMode(true);
    mCollator.setIgnorePunctuation(false);

    connect(&mFilterWatcher,
            &QFutureWatcher<void>::finished,
            this,
            &NodeSelectorProxyModel::onModelSortedFiltered);
}

NodeSelectorProxyModel::~NodeSelectorProxyModel()
{
    // Defense in depth: ~QFutureWatcher does not wait for its future, so a still
    // running concurrent sort would outlive this proxy if no owner called
    // prepareForDeletion() first (it is idempotent).
    prepareForDeletion();
}

void NodeSelectorProxyModel::prepareForDeletion()
{
    // Already called (e.g. by NodeSelectorTreeViewWidget before our own destructor
    // runs): nothing left to stop.
    if (mTearingDown)
    {
        return;
    }

    // Called during teardown while the source model is still alive. The sort runs on a
    // QtConcurrent thread and dereferences NodeSelectorModelItem objects owned by the source
    // model; if it outlives the model it is a use-after-free (crash in getParent()).
    // Make any later sort() a no-op, stop both re-launch triggers — the finished handler
    // (which could also reattach the view: levelLoaded -> onLevelLoaded -> setModel) and
    // the source model level loads — then block until the running task returns.
    mTearingDown = true;

    disconnect(&mFilterWatcher,
               &QFutureWatcher<void>::finished,
               this,
               &NodeSelectorProxyModel::onModelSortedFiltered);

    // getMegaModel() is null when called from our own destructor after the source
    // model died (QSortFilterProxyModel resets to an empty source model).
    if (auto megaModel = getMegaModel())
    {
        disconnect(megaModel,
                   &NodeSelectorModel::levelsAdded,
                   this,
                   &NodeSelectorProxyModel::invalidateModel);
    }

    if (mFilterWatcher.isRunning())
    {
        mFilterWatcher.waitForFinished();
    }
}

void NodeSelectorProxyModel::sort(int column, Qt::SortOrder order)
{
    // Teardown barrier: after prepareForDeletion() a new concurrent sort would
    // dereference a source model that is being (or has been) destroyed.
    if (mTearingDown)
    {
        return;
    }

    mOrder = order;
    mSortColumn = column;

    // If it is already blocked, it is ignored.
    getMegaModel()->sendBlockUiSignal(true);

    emit layoutAboutToBeChanged();
    if (mFilterWatcher.isFinished())
    {
        QFuture<void> filtered = QtConcurrent::run(
            [this, column, order]()
            {
                auto itemModel = dynamic_cast<NodeSelectorModel*>(sourceModel());
                if (itemModel)
                {
                    // This job runs on a QtConcurrent pool thread and reads the source model
                    // (rowCount/index/parent/data) across many calls to build the mapping. Hold
                    // the source data mutex for the whole job so the NodeRequester worker cannot
                    // insert/remove/reallocate items between those reads: without cross-call
                    // consistency the mapping ends up referencing source rows that no longer
                    // match, and a later QTreeView::drawTree faults in proxy_to_source().
                    itemModel->lockDataMutex(true);
                    blockSignals(true);
                    sourceModel()->blockSignals(true);
                    // Release the mutex and restore signals on every exit path, including an
                    // exception (e.g. std::bad_alloc while rebuilding the mapping): otherwise the
                    // recursive mutex would stay locked forever and deadlock the worker and GUI
                    // threads on their next access, and the source model would stay silent.
                    auto restore = qScopeGuard(
                        [this, itemModel]()
                        {
                            blockSignals(false);
                            sourceModel()->blockSignals(false);
                            itemModel->lockDataMutex(false);
                        });
                    invalidateFilter();
                    QSortFilterProxyModel::sort(column, order);
                    for (auto it = mItemsToMap.crbegin(); it != mItemsToMap.crend(); ++it)
                    {
                        auto proxyIndex = mapFromSource((*it));
                        hasChildren(proxyIndex);
                    }
                    mItemsToMap.clear();
                    if (mForceInvalidate)
                    {
                        invalidate();
                    }
                }
            });
        mFilterWatcher.setFuture(filtered);
    }
}

void NodeSelectorProxyModel::onSortIndicatorChanged(int column, Qt::SortOrder order)
{
    // QHeaderView::restoreState() re-emits sortIndicatorChanged unconditionally with the
    // column/order already applied. Re-sorting here would launch a concurrent sort job in
    // the middle of the loading-scene view reattach. Genuine header clicks always change
    // the column or toggle the order, so no-change notifications are safe to ignore.
    if (column == mSortColumn && order == mOrder)
    {
        return;
    }

    sort(column, order);
}

Qt::ItemFlags NodeSelectorProxyModel::flags(const QModelIndex& index) const
{
    auto flags = Qt::ItemFlags();
    if (sourceModel())
    {
        flags = sourceModel()->flags(mapToSource(index));
    }

    applyProxyModelFlags(flags, index);

    return flags;
}

mega::MegaHandle NodeSelectorProxyModel::getHandle(const QModelIndex& index)
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
    if (handle == mega::INVALID_HANDLE)
    {
        return QModelIndex();
    }
    auto megaApi = MegaSyncApp->getMegaApi();
    auto node = std::shared_ptr<mega::MegaNode>(megaApi->getNodeByHandle(handle));
    QModelIndex ret = getIndexFromNode(node);

    return ret;
}

std::shared_ptr<mega::MegaNode> NodeSelectorProxyModel::getNode(const QModelIndex& index)
{
    if (!index.isValid())
    {
        return nullptr;
    }
    return qvariant_cast<std::shared_ptr<mega::MegaNode>>(
        index.data(toInt(NodeSelectorModelRoles::NODE_ROLE)));
}

void NodeSelectorProxyModel::deleteNode(const QModelIndex& item)
{
    if (NodeSelectorModel* megaModel = getMegaModel())
    {
        megaModel->deleteNodeFromModel(mapToSource(item));
    }
}

bool NodeSelectorProxyModel::lessThan(const QModelIndex& left, const QModelIndex& right) const
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

    if (lIsFile && !rIsFile)
    {
        result = sortOrder() == Qt::DescendingOrder;
    }
    else if (!lIsFile && rIsFile)
    {
        result = sortOrder() != Qt::DescendingOrder;
    }
    else
    {
        if ((left.column() == NodeSelectorModel::Column::ADDED_DATE &&
             right.column() == NodeSelectorModel::Column::ADDED_DATE) ||
            (left.column() == NodeSelectorModel::Column::LAST_MODIFIED_DATE &&
             right.column() == NodeSelectorModel::Column::LAST_MODIFIED_DATE))
        {
            result = left.data(toInt(NodeSelectorModelRoles::DATE_ROLE)).value<int64_t>() <
                     right.data(toInt(NodeSelectorModelRoles::DATE_ROLE)).value<int64_t>();
        }
        else
        {
            int lStatus(0);
            int rStatus(0);

            if (lStatus != rStatus)
            {
                result = lStatus < rStatus;
            }
            else if (left.column() == NodeSelectorModel::Column::USER &&
                     right.column() == NodeSelectorModel::Column::USER)
            {
                result = mCollator.compare(left.data(Qt::ToolTipRole).toString(),
                                           right.data(Qt::ToolTipRole).toString()) < 0;
            }
            else if (left.column() == NodeSelectorModel::Column::ACCESS &&
                     right.column() == NodeSelectorModel::Column::ACCESS)
            {
                result = left.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt() <
                         right.data(toInt(NodeSelectorModelRoles::ACCESS_ROLE)).toInt();
            }
            else if (left.column() == NodeSelectorModel::Column::LABEL &&
                     right.column() == NodeSelectorModel::Column::LABEL)
            {
                result = left.data(toInt(NodeSelectorModelRoles::LABEL_ORDER_ROLE)).toInt() <
                         right.data(toInt(NodeSelectorModelRoles::LABEL_ORDER_ROLE)).toInt();
            }
            else
            {
                result = mCollator.compare(left.data(Qt::DisplayRole).toString(),
                                           right.data(Qt::DisplayRole).toString()) < 0;
            }
        }
    }

    return result;
}

void NodeSelectorProxyModel::setSourceModel(QAbstractItemModel* sourceModel)
{
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if (auto nodeSelectorModel = dynamic_cast<NodeSelectorModel*>(sourceModel))
    {
        connect(nodeSelectorModel,
                &NodeSelectorModel::levelsAdded,
                this,
                &NodeSelectorProxyModel::invalidateModel,
                Qt::UniqueConnection);
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

QModelIndex NodeSelectorProxyModel::getTopRootIndex()
{
    auto model(dynamic_cast<NodeSelectorModel*>(sourceModel()));
    if (model)
    {
        return mapFromSource(model->getTopRootIndex());
    }

    return QModelIndex();
}

NodeSelectorModel* NodeSelectorProxyModel::getMegaModel() const
{
    return dynamic_cast<NodeSelectorModel*>(sourceModel());
}

bool NodeSelectorProxyModel::isWorking() const
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
            if (indexItem->isRubbishBin() || indexItem->isMyBackupsFolder() ||
                indexItem->isDeviceFolder())
            {
                return false;
            }
        }
    }

    return true;
}

void NodeSelectorProxyModel::invalidateModel(
    const QList<QPair<mega::MegaHandle, QModelIndex>>& parents,
    bool force)
{
    mPendingSortIsLevelLoad = true;

    foreach(auto parent, parents)
    {
        mItemsToMap.append(parent.second);
    }
    mForceInvalidate = force;
    sort(mSortColumn, mOrder);
}

void NodeSelectorProxyModel::onModelSortedFiltered()
{
    // Consume the flag atomically so a re-entry while emitting signals
    // cannot leave it stuck in an inconsistent state.
    const bool sortFromLevelLoad = mPendingSortIsLevelLoad;
    mPendingSortIsLevelLoad = false;

    if (mForceInvalidate)
    {
        if (auto nodeSelectorModel = dynamic_cast<NodeSelectorModel*>(sourceModel()))
        {
            nodeSelectorModel->proxyInvalidateFinished();
        }
    }

    mForceInvalidate = false;

    emit layoutChanged();

    if (mExpandMapped)
    {
        emit expandReady();
    }
    else
    {
        emit navigateReady(mItemsToMap.isEmpty() ? QModelIndex() :
                                                   mapFromSource(mItemsToMap.first()));
        mExpandMapped = true;
    }

    // If the sort was caused by a new level loaded
    if (sortFromLevelLoad)
    {
        emit levelLoaded();
    }
    // If the sort was caused by a filter or a column sort
    else
    {
        emit modelSorted();
    }

    getMegaModel()->sendBlockUiSignal(false);
    mItemsToMap.clear();
}

NodeSelectorProxyModelSync::NodeSelectorProxyModelSync(QObject* parent):
    NodeSelectorProxyModel(parent)
{}

void NodeSelectorProxyModelSync::applyProxyModelFlags(Qt::ItemFlags& flags,
                                                      const QModelIndex& index) const
{
    NodeSelectorProxyModel::applyProxyModelFlags(flags, index);

    if (index.isValid() &&
        !index.data(toInt(NodeSelectorModelRoles::IS_SYNCABLE_FOLDER_ROLE)).toBool())
    {
        flags &= ~(Qt::ItemIsSelectable | Qt::ItemIsEnabled);
    }
}

QVariant NodeSelectorProxyModelSync::data(const QModelIndex& index, int role) const
{
    // Whole-row tooltip for incoming-share folders that cannot be synced because the user does not
    // have full access. Overrides the base per-cell ToolTipRole on every column so the styled
    // tooltip covers the whole row (owner/takedown tooltips still apply to full-access folders).
    if (role == Qt::ToolTipRole && index.isValid())
    {
        if (auto* item = NodeSelectorModel::getItemByIndex(index))
        {
            const auto node = item->getNode();
            const int access = item->getNodeAccess();
            if (node && node->isFolder() && access < mega::MegaShare::ACCESS_FULL)
            {
                return access == mega::MegaShare::ACCESS_READWRITE ?
                           QCoreApplication::translate(
                               "NodeSelectorTreeViewWidget",
                               "This folder is read and write. Ask for full access to sync") :
                           QCoreApplication::translate(
                               "NodeSelectorTreeViewWidget",
                               "This folder is read-only. Ask for full access to sync");
            }
        }
    }

    return NodeSelectorProxyModel::data(index, role);
}

NodeSelectorProxyModelSearch::NodeSelectorProxyModelSearch(
    std::shared_ptr<NodeSelectorProxyModel> mainProxyModel,
    QObject* parent):
    NodeSelectorProxyModel(parent),
    mMode(TabType::NONE),
    mMainProxyModel(mainProxyModel)
{}

void NodeSelectorProxyModelSearch::setMode(TabTypes mode, bool forceFilter)
{
    if (mMode == mode)
    {
        return;
    }

    mMode = mode;
    // Only block/invalidate when actually re-filtering. Doing it unconditionally emitted a
    // blockUi(false) during the initial-mode setup (forceFilter=false), which reached the view
    // before its model/header were attached and crashed. Invalidate first so rowCount() below
    // reflects the new mode when deciding whether the result set is empty.
    if (forceFilter)
    {
        getMegaModel()->sendBlockUiSignal(true);
        invalidateFilter();
        getMegaModel()->sendBlockUiSignal(false);
    }
    if (rowCount() == 0)
    {
        emit modeEmpty();
    }
}

bool NodeSelectorProxyModelSearch::canBeDeleted() const
{
    if (mMode & TabType::BACKUP)
    {
        return false;
    }
    return NodeSelectorProxyModel::canBeDeleted();
}

Qt::ItemFlags NodeSelectorProxyModelSearch::flags(const QModelIndex& index) const
{
    auto flags = NodeSelectorProxyModel::flags(index);

    if (mMainProxyModel)
    {
        mMainProxyModel->applyProxyModelFlags(flags, index);
    }

    return flags;
}

bool NodeSelectorProxyModelSearch::filterAcceptsRow(int sourceRow,
                                                    const QModelIndex& sourceParent) const
{
    if (!mMode)
    {
        return true;
    }

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if (index.isValid())
    {
        if (NodeSelectorModelItemSearch* item =
                static_cast<NodeSelectorModelItemSearch*>(index.internalPointer()))
        {
            return mMode & item->getType();
        }
    }

    return NodeSelectorProxyModel::filterAcceptsRow(sourceRow, sourceParent);
}
