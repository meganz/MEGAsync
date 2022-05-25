#include "StalledIssuesModel.h"

#include "MegaApplication.h"

#include <QSortFilterProxyModel>

#include <QDebug>

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaRequestListener()
{
    qRegisterMetaType<StalledIssuesReceived>("StalledIssuesReceived");
}

bool StalledIssuesReceiver::setCurrentStalledIssuesToCompare(const StalledIssuesVariantList& currentIssues)
{
    if(mCacheMutex.tryLock())
    {
        mCurrentStalledIssues = currentIssues;
        mCacheMutex.unlock();
        return true;
    }

    return false;
}

void StalledIssuesReceiver::processStalledIssues()
{
    emit stalledIssuesReady(mCacheStalledIssues);
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError*)
{
    if (auto stalls = request->getMegaSyncStallList())
    {
        QMutexLocker lock(&mCacheMutex);

        mCacheStalledIssues.clear();

        for (int i = 0; i < stalls->size(); ++i)
        {
            auto stall = stalls->get(i);

            //Compare with the currentIssues
            auto existingUpdate = std::find_if(mCurrentStalledIssues.begin(), mCurrentStalledIssues.end(), [stall](const std::shared_ptr<StalledIssueVariant>& check) ->bool{
                if(stall->reason() == check->consultData()->getReason())
                {
                    if(stall->reason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                    {
                        if(auto nameConflict = std::dynamic_pointer_cast<const NameConflictedStalledIssue>(check->consultData()))
                        {
                            auto cloudNames = NameConflictedStalledIssue::convertConflictedNames(true, stall);
                            auto localNames = NameConflictedStalledIssue::convertConflictedNames(false, stall);

                            if(!nameConflict->getNameConflictCloudData().isEmpty() && !cloudNames.isEmpty())
                            {
                                foreach(auto name, cloudNames)
                                {
                                    if(nameConflict->getNameConflictCloudData().conflictedNames.contains(name))
                                    {
                                        return true;
                                    }
                                }
                            }

                            if(!nameConflict->getNameConflictLocalData().isEmpty() && !localNames.isEmpty())
                            {
                                foreach(auto name, localNames)
                                {
                                    if(nameConflict->getNameConflictLocalData().conflictedNames.contains(name))
                                    {
                                        return true;
                                    }
                                }
                            }
                        }
                    }
                    else
                    {
                        auto result(true);
                        auto issue = check->consultData();

                        if(issue->consultCloudData())
                        {
                            result &= issue->consultCloudData()->isEqual(stall);
                        }

                        if(issue->consultLocalData())
                        {
                            result &= issue->consultLocalData()->isEqual(stall);
                        }

                        return result;
                    }
                }

                return false;
            });

            if(!mCurrentStalledIssues.isEmpty() && existingUpdate != mCurrentStalledIssues.end())
            {
                (*existingUpdate)->updateData(stall);
                mCacheStalledIssues.mUpdateStalledIssues.append((*existingUpdate));
                mCurrentStalledIssues.removeOne(*existingUpdate);
            }
            else
            {
                if(stall->reason() == mega::MegaSyncStall::SyncStallReason::NamesWouldClashWhenSynced)
                {
                    auto d = std::make_shared<NameConflictedStalledIssue>(stall);
                    auto variant = std::make_shared<StalledIssueVariant>(d);
                    mCacheStalledIssues.mNewStalledIssues.append(variant);
                }
                else
                {
                    auto d = std::make_shared<StalledIssue>(stall);
                    auto variant = std::make_shared<StalledIssueVariant>(d);
                    mCacheStalledIssues.mNewStalledIssues.append(variant);
                }
            }
        }

        mCacheStalledIssues.mDeleteStalledIssues.append(mCurrentStalledIssues);

        processStalledIssues();
    }
}

StalledIssuesModel::StalledIssuesModel(QObject *parent)
    : QAbstractItemModel(parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mHasStalledIssues(false),
    mUpdateWhenGlobalStateChanges(false)
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
    if(!issuesReceived.isEmpty())
    {
        QMutexLocker lock(&mModelMutex);

        for (auto it = issuesReceived.mDeleteStalledIssues.begin(); it != issuesReceived.mDeleteStalledIssues.end(); ++it)
        {
            auto index = mStalledIssuesByOrder.value((*it).get(),-1);
            if(index >= 0)
            {
                removeRows(index, 1);
            }

            updateStalledIssuedByOrder();
        }

        for (auto it = issuesReceived.mUpdateStalledIssues.begin(); it != issuesReceived.mUpdateStalledIssues.end(); ++it)
        {
            auto row = mStalledIssuesByOrder.value((*it).get(),-1);
            if(row >= 0)
            {
                auto rootIndex(index(row,0));
                emit dataChanged(rootIndex,rootIndex);

                auto childIndex(index(0,0,rootIndex));
                if(childIndex.isValid())
                {
                    emit dataChanged(childIndex,childIndex);
                }
            }
        }

        auto totalRows = rowCount(QModelIndex());
        auto rowsToBeInserted(static_cast<int>(issuesReceived.mNewStalledIssues.size()));

        if(rowsToBeInserted > 0)
        {
            beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = issuesReceived.mNewStalledIssues.begin(); it != issuesReceived.mNewStalledIssues.end();)
            {
                std::shared_ptr<StalledIssueVariant> issue(*it);
                mStalledIssues.append(issue);
                mStalledIssuesByOrder.insert(issue.get(), rowCount(QModelIndex()) - 1);
                mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason((*it)->consultData()->getReason()))]++;

                it++;
            }

            endInsertRows();
        }

        emit stalledIssuesCountChanged();
        emit stalledIssuesReceived(true);
        emit layoutAboutToBeChanged();

        layoutChanged();
    }
}

void StalledIssuesModel::updateStalledIssues()
{
    blockUi();
    reset();

    if (mMegaApi->isSyncStalled())
    {
        //At the moment, we reset and refil the whole model
        //if(mStalledIssuedReceiver->setCurrentStalledIssuesToCompare(mStalledIssues))
        //{
            mMegaApi->getMegaSyncStallList(nullptr);
        //}
    }
    else
    {
        emit stalledIssuesReceived(false);
    }
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
void StalledIssuesModel::solveLocalConflictedNameByRemove(const QString &name, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        nameConflict->solveLocalConflictedName(name, NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE);
    }
}

void StalledIssuesModel::solveLocalConflictedNameByRename(const QString &name, const QString &renameTo, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        nameConflict->solveLocalConflictedNameByRename(name, renameTo);
    }
}

void StalledIssuesModel::solveCloudConflictedNameByRemove(const QString &name, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        nameConflict->solveCloudConflictedName(name, NameConflictedStalledIssue::ConflictedNameInfo::SolvedType::REMOVE);
    }
}

void StalledIssuesModel::solveCloudConflictedNameByRename(const QString &name, const QString& renameTo, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    if(auto nameConflict = std::dynamic_pointer_cast<NameConflictedStalledIssue>(mStalledIssues.at(potentialIndex.row())->getData()))
    {
        nameConflict->solveCloudConflictedNameByRename(name, renameTo);
    }
}

void StalledIssuesModel::solveIssue(bool isCloud, const QModelIndex &index)
{
    auto potentialIndex = getSolveIssueIndex(index);

    mStalledIssues.at(potentialIndex.row())->getData()->setIsSolved(isCloud);
}
