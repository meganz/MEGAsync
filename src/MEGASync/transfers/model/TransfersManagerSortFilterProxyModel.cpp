#include "TransfersManagerSortFilterProxyModel.h"
#include "TransfersModel.h"
#include "MegaApplication.h"
#include "TransferManagerDelegateWidget.h"
#include <megaapi.h>

#include <QMutexLocker>
#include <QElapsedTimer>
#include <QRunnable>
#include <QTimer>

TransfersManagerSortFilterProxyModel::TransfersManagerSortFilterProxyModel(QObject* parent)
    : TransfersSortFilterProxyBaseModel(parent),
      mTransferStates (TransferData::STATE_MASK),
      mTransferTypes (TransferData::TYPE_MASK),
      mFileTypes (~Utilities::FileTypes()),
      mNextTransferStates (mTransferStates),
      mNextTransferTypes (mTransferTypes),
      mNextFileTypes (mFileTypes),
      mSortCriterion (SortCriterion::PRIORITY),
      mThreadPool (ThreadPoolSingleton::getInstance()),
      mIsFiltering(false)
{
    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &TransfersManagerSortFilterProxyModel::onModelSortedFiltered);

    setDynamicSortFilter(false);
    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

TransfersManagerSortFilterProxyModel::~TransfersManagerSortFilterProxyModel()
{
}

void TransfersManagerSortFilterProxyModel::initProxyModel(SortCriterion sortCriterion, Qt::SortOrder order)
{
    mSortOrder = order;
    mSortCriterion = static_cast<SortCriterion>(sortCriterion);
}

void TransfersManagerSortFilterProxyModel::sort(int sortCriterion, Qt::SortOrder order)
{
    auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(true);
    }

    emit modelAboutToBeChanged();
    if (sortCriterion != static_cast<int>(mSortCriterion))
    {
        mSortCriterion = static_cast<SortCriterion>(sortCriterion);
    }

    mSortOrder = order;

    resetTransfersStateCounters();
    invalidateModel();
}

void TransfersManagerSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TransfersManagerSortFilterProxyModel::onRowsAboutToBeRemoved, Qt::DirectConnection);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void TransfersManagerSortFilterProxyModel::setFilterFixedString(const QString& pattern)
{
    mFilterText = pattern;
    refreshFilterFixedString();
}

void TransfersManagerSortFilterProxyModel::refreshFilterFixedString()
{
    updateFilters();

    resetAllCounters();
    emit modelAboutToBeChanged();

    invalidateModel();
}

void TransfersManagerSortFilterProxyModel::textSearchTypeChanged()
{
    updateFilters();
    resetTransfersStateCounters();
    emit modelAboutToBeChanged();

    invalidateModel();
}

void TransfersManagerSortFilterProxyModel::invalidateModel()
{
    if(!dynamicSortFilter())
    {
        setDynamicSortFilter(true);
    }

    QFuture<void> filtered = QtConcurrent::run([this](){
        auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        sourceM->blockSignals(true);
        blockSignals(true);
        mIsFiltering = true;
        invalidate();
        QSortFilterProxyModel::sort(0, mSortOrder);
        blockSignals(false);
        sourceM->blockSignals(false);
        mIsFiltering = false;
        sourceM->lockModelMutex(false);
    });
    mFilterWatcher.setFuture(filtered);
}

void TransfersManagerSortFilterProxyModel::onModelSortedFiltered()
{
    auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(false);
    }

    emit modelChanged();
    emit searchNumbersChanged();
}

void TransfersManagerSortFilterProxyModel::setFilters(const TransferData::TransferTypes transferTypes,
                                               const TransferData::TransferStates transferStates,
                                               const Utilities::FileTypes fileTypes)
{
    mNextTransferTypes = transferTypes ? transferTypes : TransferData::TYPE_MASK;
    mNextTransferStates = transferStates ? transferStates : TransferData::STATE_MASK;
    if (fileTypes)
    {
        mNextFileTypes = fileTypes;
    }
    else
    {
        mNextFileTypes = ~Utilities::FileTypes({});
    }
}

void TransfersManagerSortFilterProxyModel::resetAllFilters()
{
    resetAllCounters();
    setFilters({}, {}, {});
}

int  TransfersManagerSortFilterProxyModel::getNumberOfItems(TransferData::TransferType transferType)
{
    int nb(0);

    if(transferType == TransferData::TransferType::TRANSFER_UPLOAD)
    {
        nb = mUlNumber.size();
    }
    else if(transferType == TransferData::TransferType::TRANSFER_DOWNLOAD)
    {
        nb = mDlNumber.size();
    }

    return nb;
}

void TransfersManagerSortFilterProxyModel::resetAllCounters()
{
    mDlNumber.clear();
    mUlNumber.clear();
    resetTransfersStateCounters();
}

void TransfersManagerSortFilterProxyModel::resetTransfersStateCounters()
{
    mNoSyncTransfers.clear();
    mActiveTransfers.clear();
    mPausedTransfers.clear();
    mCompletedTransfers.clear();
}

TransferBaseDelegateWidget *TransfersManagerSortFilterProxyModel::createTransferManagerItem(QWidget*)
{
    auto item = new TransferManagerDelegateWidget(nullptr);

    connect(item, &TransferManagerDelegateWidget::cancelClearTransfer,
            this, &TransfersManagerSortFilterProxyModel::onCancelClearTransfer);
    connect(item, &TransferManagerDelegateWidget::pauseResumeTransfer,
            this, &TransfersManagerSortFilterProxyModel::onPauseResumeTransfer);
    connect(item, &TransferManagerDelegateWidget::retryTransfer,
             this, &TransfersManagerSortFilterProxyModel::onRetryTransfer);
    connect(item, &TransferManagerDelegateWidget::openTransfer,
             this, &TransfersManagerSortFilterProxyModel::onOpenTransfer);

    return item;
}

void TransfersManagerSortFilterProxyModel::updateFilters()
{
    auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(true);
    }

    mTransferStates = mNextTransferStates;
    mTransferTypes = mNextTransferTypes;
    mFileTypes = mNextFileTypes;
}

bool TransfersManagerSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool accept(false);

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);
    const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

    if(d && d->mTag >= 0)
    {
        accept = (d->getState() & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (toInt(d->mFileType) & mFileTypes);

        if (accept)
        {
            if(!mFilterText.isEmpty())
            {
                accept = d->mFilename.contains(mFilterText,Qt::CaseInsensitive);

                if (accept)
                {
                    if (d->mType & TransferData::TRANSFER_UPLOAD && !mUlNumber.contains(d->mTag))
                    {
                        mUlNumber.insert(d->mTag);
                    }
                    else if (d->mType & TransferData::TRANSFER_DOWNLOAD && !mDlNumber.contains(d->mTag))
                    {
                        mDlNumber.insert(d->mTag);
                    }
                }
            }
        }

        //Not needed to add the logic when the d is a sync transfer, as the sync state is permanent
        if(accept && !d->isSyncTransfer() && !mNoSyncTransfers.contains(d->mTag))
        {
            mNoSyncTransfers.insert(d->mTag);
        }

        //As the active state can change in time, add both logics to add or remove
        if(accept && (d->isActive() && !d->isCompleting()))
        {
            if(!mActiveTransfers.contains(d->mTag))
            {
                mActiveTransfers.insert(d->mTag);
            }
        }
        else
        {
            removeActiveTransferFromCounter(d->mTag);
        }

        if(accept && d->isPaused())
        {
            if(!mPausedTransfers.contains(d->mTag))
            {
                mPausedTransfers.insert(d->mTag);
            }
        }
        else
        {
            removePausedTransferFromCounter(d->mTag);
        }

        if(accept && ((d->isCompleted() && !d->isFailed())))
        {
            if(!mCompletedTransfers.contains(d->mTag))
            {
                mCompletedTransfers.insert(d->mTag);
            }
        }
        else
        {
            removeCompletedTransferFromCounter(d->mTag);
        }

        if(accept && d->isFailed())
        {
            if(!mFailedTransfers.contains(d->mTag))
            {
                mFailedTransfers.insert(d->mTag);
            }
        }
        else
        {
            removeFailedTransferFromCounter(d->mTag);
        }
    }

    return accept;
}

bool TransfersManagerSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftItem (qvariant_cast<TransferItem>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem>(right.data()).getTransferData());

    if(leftItem && rightItem)
    {
        switch (mSortCriterion)
        {
        case SortCriterion::PRIORITY:
        {
            return leftItem->mPriority > rightItem->mPriority;
        }
        case SortCriterion::TOTAL_SIZE:
        {
            return leftItem->mTotalSize < rightItem->mTotalSize;
        }
        case SortCriterion::NAME:
        {
            return QString::compare(leftItem->mFilename, rightItem->mFilename, Qt::CaseInsensitive) < 0;
        }
        case SortCriterion::SPEED:
        {
            return leftItem->mSpeed < rightItem->mSpeed;
        }
        case SortCriterion::TIME:
        {
            if(leftItem->isProcessing() || rightItem->isProcessing())
            {
                return leftItem->mRemainingTime < rightItem->mRemainingTime;
            }
            else if(leftItem->isFinished() && rightItem->isFinished())
            {
                return leftItem->getRawFinishedTime() < rightItem->getRawFinishedTime();
            }
        }
        default:
            break;
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}


//It is called from a QtConcurrent thread
void TransfersManagerSortFilterProxyModel::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
   bool searchRowsRemoved(false);

   for(int row = first; row <= last; ++row)
   {
       QModelIndex index = sourceModel()->index(row, 0, parent);

       const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

       if(d && d->mTag >= 0)
       {
           searchRowsRemoved |= updateTransfersCounterFromTag(d);
       }
   }

   if(searchRowsRemoved)
   {
       searchNumbersChanged();
   }
}

bool TransfersManagerSortFilterProxyModel::updateTransfersCounterFromTag(QExplicitlySharedDataPointer<TransferData> transfer) const
{
    bool searchRowsRemoved(false);

    if(!mFilterText.isEmpty())
    {
        if (transfer->mType & TransferData::TRANSFER_UPLOAD)
        {
            mUlNumber.remove(transfer->mTag);
        }
        else
        {
            mDlNumber.remove(transfer->mTag);
        }

        searchRowsRemoved = true;
    }

    if(!transfer->isSyncTransfer())
    {
       removeNonSyncedTransferFromCounter(transfer->mTag);
    }

    if(transfer->isCompleted())
    {
        removeCompletedTransferFromCounter(transfer->mTag);
    }
    else if(transfer->isFailed())
    {
        removeFailedTransferFromCounter(transfer->mTag);
    }
    else
    {
        removeActiveTransferFromCounter(transfer->mTag);
        removePausedTransferFromCounter(transfer->mTag);
    }

    return searchRowsRemoved;
}

void TransfersManagerSortFilterProxyModel::removeActiveTransferFromCounter(TransferTag tag) const
{
    if(mActiveTransfers.contains(tag))
    {
        mActiveTransfers.remove(tag);
    }
}

void TransfersManagerSortFilterProxyModel::removePausedTransferFromCounter(TransferTag tag) const
{
    if(mPausedTransfers.contains(tag))
    {
        mPausedTransfers.remove(tag);
    }
}

void TransfersManagerSortFilterProxyModel::removeNonSyncedTransferFromCounter(TransferTag tag) const
{
    if(mNoSyncTransfers.contains(tag))
    {
        mNoSyncTransfers.remove(tag);
    }
}

void TransfersManagerSortFilterProxyModel::removeCompletedTransferFromCounter(TransferTag tag) const
{
    if(mCompletedTransfers.contains(tag))
    {
        mCompletedTransfers.remove(tag);
    }
}

void TransfersManagerSortFilterProxyModel::removeFailedTransferFromCounter(TransferTag tag) const
{
    if(mFailedTransfers.contains(tag))
    {
        mFailedTransfers.remove(tag);
    }
}

QMimeData *TransfersManagerSortFilterProxyModel::mimeData(const QModelIndexList &indexes) const
{
    //sorted in inverse order to guarantee that the original order is preserved
    auto sortedIndexes(indexes);
    std::sort(sortedIndexes.begin(), sortedIndexes.end(),[](const QModelIndex& index1, const QModelIndex& index2){
        return index1.row() > index2.row();
    });

    return TransfersSortFilterProxyBaseModel::mimeData(sortedIndexes);
}

int TransfersManagerSortFilterProxyModel::getPausedTransfers() const
{
    return mPausedTransfers.size();
}

bool TransfersManagerSortFilterProxyModel::areAllPaused() const
{
    return mPausedTransfers.size() == mActiveTransfers.size();
}

bool TransfersManagerSortFilterProxyModel::isAnyActive() const
{
    return !mActiveTransfers.isEmpty() ;
}

bool TransfersManagerSortFilterProxyModel::areAllActive() const
{
    return mActiveTransfers.size() > 0 && (mPausedTransfers.isEmpty() && mCompletedTransfers.isEmpty() && mFailedTransfers.isEmpty());
}

bool TransfersManagerSortFilterProxyModel::areAllSync() const
{
    return !isEmpty() && mNoSyncTransfers.isEmpty();
}

bool TransfersManagerSortFilterProxyModel::areAllCompleted() const
{
    return mCompletedTransfers.size() > 0 && (mPausedTransfers.isEmpty() && mActiveTransfers.isEmpty() && mFailedTransfers.isEmpty());
}

bool TransfersManagerSortFilterProxyModel::isAnyCompleted() const
{
    return !mCompletedTransfers.isEmpty();
}

bool TransfersManagerSortFilterProxyModel::isEmpty() const
{
    return mCompletedTransfers.isEmpty() && mPausedTransfers.isEmpty() && mActiveTransfers.isEmpty() && mFailedTransfers.isEmpty();
}

int TransfersManagerSortFilterProxyModel::activeTransfers() const
{
    return mActiveTransfers.size();
}

bool TransfersManagerSortFilterProxyModel::isModelProcessing() const
{
    return mFilterWatcher.isRunning();
}

bool TransfersManagerSortFilterProxyModel::moveRows(const QModelIndex &proxyParent, int proxyRow, int count,
              const QModelIndex &destinationParent, int destinationChild)
{
    bool moveOk(true);
    int row(proxyRow);
    auto totalRows(rowCount());

    auto sourceTotalRows(sourceModel()->rowCount());

    while (moveOk && row < (proxyRow+count))
    {
        auto sourceIndex(mapToSource(index(proxyRow, 0, proxyParent)));
        int destRow;
        if (destinationChild == totalRows)
        {
            destRow = sourceTotalRows;
        }
        else
        {
            destRow = mapToSource(index(destinationChild, 0, destinationParent)).row();
        }
        moveOk = sourceModel()->moveRows(sourceIndex.parent(), sourceIndex.row(), 1,
                                         sourceIndex.parent(), destRow);
        row++;
    }

    return moveOk;
}

void TransfersManagerSortFilterProxyModel::onCancelClearTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        emit transferCancelClear();
    }
}

void TransfersManagerSortFilterProxyModel::onPauseResumeTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        auto pause = delegateWidget->getData()->getState() != TransferData::TransferState::TRANSFER_PAUSED;
        emit pauseResumeTransfer(pause);
    }
}

void TransfersManagerSortFilterProxyModel::onRetryTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        emit transferRetry();
    }
}

void TransfersManagerSortFilterProxyModel::onOpenTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        auto data = delegateWidget->getData();
        if(data)
        {
            auto tag = data->mTag;
            //If the transfer is an upload (already on the local drive)
            //Or if it is an download but already finished
            if(data->mType & TransferData::TRANSFER_UPLOAD
                    || data->getState() & TransferData::FINISHED_STATES_MASK)
            {
                sourModel->openFolderByTag(tag);
            }
        }
    }
}
