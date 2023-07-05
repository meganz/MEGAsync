#include "StalledIssuesModel.h"

#include "MegaApplication.h"
#include <StalledIssuesDelegateWidgetsCache.h>
#include <NameConflictStalledIssue.h>

#include "ThreadPool.h"
#include "mega/types.h"

#include <QSortFilterProxyModel>

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaRequestListener()
{
    qRegisterMetaType<StalledIssuesReceived>("StalledIssuesReceived");
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError*)
{
    if (request->getType() == ::mega::MegaRequest::TYPE_GET_SYNC_STALL_LIST)
    {
        QMutexLocker lock(&mCacheMutex);
        mCacheStalledIssues.clear();

        if (auto stalls = request->getMegaSyncStallList())
        {
            for (size_t i = 0; i < stalls->size(); ++i)
            {
                auto stall = stalls->get(i);

                if(stall->reason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                {
                    auto d = std::make_shared<NameConflictedStalledIssue>(stall);
                    auto variant = std::make_shared<StalledIssueVariant>(d);
                    if(!d->isSolved())
                    {
                        mCacheStalledIssues.stalledIssues.append(variant);
                    }
                }
                else
                {
                    auto d = std::make_shared<StalledIssue>(stall);
                    auto variant = std::make_shared<StalledIssueVariant>(d);
                    mCacheStalledIssues.stalledIssues.append(variant);
                }
            }
        }
        emit stalledIssuesReady(mCacheStalledIssues);
    }
}

const int StalledIssuesModel::ADAPTATIVE_HEIGHT_ROLE = Qt::UserRole;

StalledIssuesModel::StalledIssuesModel(QObject *parent)
    : QAbstractItemModel(parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mHasStalledIssues(false),
    mUpdateWhenGlobalStateChanges(false),
    mThreadPool(mega::make_unique<ThreadPool>(1))
{
    mStalledIssuesThread = new QThread();
    mStalledIssuedReceiver = new StalledIssuesReceiver();

    mRequestListener = new mega::QTMegaRequestListener(mMegaApi, mStalledIssuedReceiver);
    mStalledIssuedReceiver->moveToThread(mStalledIssuesThread);
    mRequestListener->moveToThread(mStalledIssuesThread);
    mMegaApi->addRequestListener(mRequestListener);

    mGlobalListener = new mega::QTMegaGlobalListener(mMegaApi,this);
    mMegaApi->addGlobalListener(mGlobalListener);

    mStalledIssuesThread->start();

    connect(mStalledIssuedReceiver, &StalledIssuesReceiver::stalledIssuesReady,
            this, &StalledIssuesModel::onProcessStalledIssues,
            Qt::QueuedConnection);
}

StalledIssuesModel::~StalledIssuesModel()
{
    mMegaApi->removeRequestListener(mRequestListener);
    mMegaApi->removeGlobalListener(mGlobalListener);

    mStalledIssuesThread->quit();
    mStalledIssuesThread->deleteLater();
    mStalledIssuedReceiver->deleteLater();

    mRequestListener->deleteLater();
}

void StalledIssuesModel::onProcessStalledIssues(StalledIssuesReceiver::StalledIssuesReceived issuesReceived)
{
    reset();

    mThreadPool->push([this, issuesReceived]()
    {
        mModelMutex.lock();
        blockSignals(true);

        auto totalRows = rowCount(QModelIndex());
        auto rowsToBeInserted(static_cast<int>(issuesReceived.stalledIssues.size()));

        if(rowsToBeInserted > 0)
        {
            beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = issuesReceived.stalledIssues.begin(); it != issuesReceived.stalledIssues.end();)
            {
                if(mThreadPool->isThreadInterrupted())
                {
                    return;
                }

                std::shared_ptr<StalledIssueVariant> issue(*it);
                mStalledIssues.append(issue);
                mStalledIssuesByOrder.insert(issue.get(), rowCount(QModelIndex()) - 1);
                mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason((*it)->consultData()->getReason()))]++;

                it++;
            }

            endInsertRows();
        }

        blockSignals(false);
        mModelMutex.unlock();

        emit stalledIssuesCountChanged();
        emit stalledIssuesReceived(true);
    });
}

void StalledIssuesModel::updateStalledIssues()
{
    blockUi();

    mMegaApi->getMegaSyncStallList(nullptr);
}

void StalledIssuesModel::updateStalledIssuesWhenReady()
{
    mUpdateWhenGlobalStateChanges = true;
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi* api)
{
    if(mHasStalledIssues != api->isSyncStalled())
    {
        mHasStalledIssues = api->isSyncStalled();
        emit globalSyncStateChanged(mHasStalledIssues);
    }
}

Qt::DropActions StalledIssuesModel::supportedDropActions() const
{
    return Qt::IgnoreAction;
}

bool StalledIssuesModel::hasChildren(const QModelIndex &parent) const
{
    auto stalledIssueItem = static_cast<StalledIssue*>(parent.internalPointer());
    if (stalledIssueItem)
    {
        return false;
    }

    return true;
}

int StalledIssuesModel::rowCount(const QModelIndex &parent) const
{
   if(!parent.isValid())
   {
       return mStalledIssues.size();
   }
   else
   {
       return 1;
   }
}

int StalledIssuesModel::columnCount(const QModelIndex &) const
{
   return 1;
}

QVariant StalledIssuesModel::data(const QModelIndex &index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        auto stalledIssueItem = static_cast<StalledIssueVariant*>(index.internalPointer());
        if (stalledIssueItem)
        {
            return QVariant::fromValue((*stalledIssueItem));
        }
        else
        {
            return QVariant::fromValue(StalledIssueVariant(*(mStalledIssues.at(index.row()).get())));
        }
    }
    else if(role == ADAPTATIVE_HEIGHT_ROLE)
    {
        auto issue = mStalledIssues.at(index.row());
        return StalledIssuesDelegateWidgetsCache::adaptativeHeight(issue->getData()->getReason());
    }

    return QVariant();
}

QModelIndex StalledIssuesModel::parent(const QModelIndex &index) const
{
    if(!index.isValid())
    {
        return QModelIndex();
    }

    auto stalledIssueItem = static_cast<StalledIssueVariant*>(index.internalPointer());
    if (!stalledIssueItem)
    {
        return QModelIndex();
    }

    auto row = mStalledIssuesByOrder.value(&(*stalledIssueItem),-1);
    if(row >= 0)
    {
        return createIndex(row, 0);
    }

    return QModelIndex();
}

QModelIndex StalledIssuesModel::index(int row, int column, const QModelIndex &parent) const
{
    if(parent.isValid() && mStalledIssues.size() > parent.row())
    {
        auto& stalledIssue = mStalledIssues[parent.row()];
        return createIndex(0, 0, stalledIssue.get());
    }
    else
    {
        return (row < rowCount(QModelIndex())) ?  createIndex(row, column) : QModelIndex();
    }
}

Qt::ItemFlags StalledIssuesModel::flags(const QModelIndex &index) const
{
    return QAbstractItemModel::flags(index) | Qt::ItemIsEnabled | Qt::ItemIsEditable;
}

void StalledIssuesModel::finishStalledIssues(const QModelIndexList &indexes)
{
    auto indexesToFinish(indexes);
    removeRows(indexesToFinish);
}

void StalledIssuesModel::removeRows(QModelIndexList &indexesToRemove)
{
    std::sort(indexesToRemove.begin(), indexesToRemove.end(),[](QModelIndex check1, QModelIndex check2){
        return check1.row() > check2.row();
    });

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (indexesToRemove.last().row());
    for (auto index : indexesToRemove)
    {
        // Init row with row of first tag
        if (count == 0)
        {
            row = index.row();
        }

        // If rows are non-contiguous, flush and start from item
        if (row != index.row())
        {
            removeRows(row + 1, count, QModelIndex());
            count = 0;
            row = index.row();
        }

        // We have at least one row
        count++;
        row--;
    }
    // Flush pooled rows (start at row + 1).
    // This happens when the last item processed is in a finished state.
    if (count > 0)
    {
        removeRows(row + 1, count, QModelIndex());
    }

    updateStalledIssuedByOrder();
}

bool StalledIssuesModel::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent == QModelIndex() && count > 0 && row >= 0)
    {
        beginRemoveRows(parent, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            mStalledIssues.removeAt(i);
        }

        endRemoveRows();

        return true;
    }
    else
    {
        return false;
    }
}

void StalledIssuesModel::updateStalledIssuedByOrder()
{
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();

    //Recalculate rest of items
    for(int row = 0; row < rowCount(QModelIndex()); ++row)
    {
        auto item = mStalledIssues.at(row);
        mStalledIssuesByOrder.insert(item.get(), row);

        mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(item->consultData()->getReason()))]++;
    }

    emit stalledIssuesCountChanged();
}

bool StalledIssuesModel::hasStalledIssues() const
{
    return mHasStalledIssues;
}

void StalledIssuesModel::lockModelMutex(bool lock)
{
    if (lock)
    {
        mModelMutex.lock();
    }
    else
    {
        mModelMutex.unlock();
    }
}

void StalledIssuesModel::blockUi()
{
    emit uiBlocked();
}

void StalledIssuesModel::unBlockUi()
{
    emit uiUnblocked();
}

void StalledIssuesModel::updateIndex(const QModelIndex &index)
{
    emit dataChanged(index, index);
}

void StalledIssuesModel::reset()
{
    beginResetModel();

    mStalledIssues.clear();
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();

    endResetModel();

    emit stalledIssuesCountChanged();
}

QModelIndex StalledIssuesModel::getSolveIssueIndex(const QModelIndex &index)
{
    auto indexParent(index.parent());
    auto potentialIndex(indexParent.isValid() ? indexParent : index);

    if(potentialIndex.model() != this)
    {
        if(auto proxyIndex = dynamic_cast<const QSortFilterProxyModel*>(index.model()))
        {
            potentialIndex = proxyIndex->mapToSource(potentialIndex);
        }
    }

    return potentialIndex;
}

int StalledIssuesModel::getCountByFilterCriterion(StalledIssueFilterCriterion criterion)
{
    if(criterion == StalledIssueFilterCriterion::ALL_ISSUES)
    {
        return rowCount(QModelIndex());
    }
    else
    {
        return mCountByFilterCriterion.value((int)criterion,0);
    }
}

//METHODS TO MODIFY DATA
bool StalledIssuesModel::solveLocalConflictedNameByRemove(int conflictIndex, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        return nameConflict->solveLocalConflictedNameByRemove(conflictIndex);
    }

    return false;
}

bool StalledIssuesModel::solveLocalConflictedNameByRename(const QString &renameTo, int conflictIndex, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        return nameConflict->solveLocalConflictedNameByRename(conflictIndex, renameTo);
    }

    return false;
}

bool StalledIssuesModel::solveCloudConflictedNameByRemove(int conflictIndex, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        return nameConflict-> solveCloudConflictedNameByRemove(conflictIndex);
    }

    return false;
}

bool StalledIssuesModel::solveCloudConflictedNameByRename(const QString& renameTo, int conflictIndex, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        return nameConflict->solveCloudConflictedNameByRename(conflictIndex, renameTo);
    }

    return false;
}

void StalledIssuesModel::solveIssue(bool isCloud, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    mStalledIssues.at(potentialIndex.row())->getData()->setIsSolved(isCloud);
}

void StalledIssuesModel::solveDuplicatedIssues(const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        nameConflict->solveIssue();
    }
}
