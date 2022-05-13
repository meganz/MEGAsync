#include "StalledIssuesModel.h"

#include "MegaApplication.h"

StalledIssuesReceiver::StalledIssuesReceiver(QObject *parent) : QObject(parent), mega::MegaRequestListener()
{
}

void StalledIssuesReceiver::processStalledIssues()
{
    StalledIssuesList auxList;

    if(mCacheStalledIssues.size() > 2000)
    {
        for(auto index = 0; index < 2000
            && !mCacheStalledIssues.isEmpty(); ++index)
        {
            auto& firstItem = mCacheStalledIssues.first();            
            auxList.append(firstItem);
            mCacheStalledIssues.removeOne(firstItem);

        }
    }
    else
    {
        auxList = mCacheStalledIssues;
        mCacheStalledIssues.clear();

    }

    emit stalledIssuesReady(auxList);
}

void StalledIssuesReceiver::onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError*)
{
    if (auto stalls = request->getMegaSyncStallList())
    {
        QMutexLocker lock(&mCacheMutex);

        for (int i = 0; i < stalls->size(); ++i)
        {
            auto stall = stalls->get(i);

            StalledIssue d (stall);
            mCacheStalledIssues.append(d);
        }

        processStalledIssues();
    }
}

StalledIssuesModel::StalledIssuesModel(QObject *parent) : QAbstractItemModel(parent)
   , mMegaApi (MegaSyncApp->getMegaApi())
   , mHasStalledIssues(false)
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

void StalledIssuesModel::onProcessStalledIssues(StalledIssuesList stalledIssues)
{
    //Try blocking signals
    //QtConcurrent::run([this, stalledIssues]()
    //{
        reset();

        if(!stalledIssues.isEmpty())
        {
            QMutexLocker lock(&mModelMutex);

            auto totalRows = rowCount(QModelIndex());
            auto rowsToBeInserted(static_cast<int>(stalledIssues.size()));

            beginInsertRows(QModelIndex(), totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = stalledIssues.begin(); it != stalledIssues.end();)
            {
                StalledIssue issue(*it);
                mStalledIssues.append(issue);
                mStalledIssuesByOrder.insert(&issue, rowCount(QModelIndex()) - 1);
                mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason((*it).getReason()))]++;

                it++;
            }

            endInsertRows();
            emit stalledIssuesCountChanged();
        }
        emit stalledIssuesReceived(true);
    //});
}

void StalledIssuesModel::updateStalledIssues()
{
    //For a future, add a Loading Windows here
    reset();

    if (mMegaApi->isSyncStalled())
    {
        mMegaApi->getMegaSyncStallList(nullptr);
    }
    else
    {
        emit stalledIssuesReceived(false);
    }
}

void StalledIssuesModel::onGlobalSyncStateChanged(mega::MegaApi* api)
{
    mHasStalledIssues = api->isSyncStalled();
    emit globalSyncStateChanged(mHasStalledIssues);
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
        auto stalledIssueItem = static_cast<StalledIssue*>(index.internalPointer());
        if (stalledIssueItem)
        {
            return QVariant::fromValue((*stalledIssueItem));
        }
        else
        {
            return QVariant::fromValue(StalledIssue(mStalledIssues.at(index.row())));
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

    auto stalledIssueItem = static_cast<StalledIssue*>(index.internalPointer());
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
    if(parent.isValid())
    {
        auto& stalledIssue = mStalledIssues[parent.row()];
        return createIndex(0, 0, &stalledIssue);
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
        mStalledIssuesByOrder.insert(&item, row);

        mCountByFilterCriterion[static_cast<int>(StalledIssue::getCriterionByReason(item.getReason()))]++;
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

void StalledIssuesModel::reset()
{
    beginResetModel();

    mStalledIssues.clear();
    mStalledIssuesByOrder.clear();
    mCountByFilterCriterion.clear();

    endResetModel();

    emit stalledIssuesCountChanged();
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
