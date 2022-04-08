#include "TransfersManagerSortFilterProxyModel.h"
#include "TransfersModel.h"
#include "MegaApplication.h"
#include "TransferManagerDelegateWidget.h"
#include <megaapi.h>

#include <QMutexLocker>
#include <QElapsedTimer>
#include <QRunnable>


TransfersManagerSortFilterProxyModel::TransfersManagerSortFilterProxyModel(QObject* parent)
    : TransfersSortFilterProxyBaseModel(parent),
      mTransferStates (TransferData::STATE_MASK),
      mTransferTypes (TransferData::TYPE_MASK),
      mFileTypes (~Utilities::FileTypes()),
      mNextTransferStates (mTransferStates),
      mNextTransferTypes (mTransferTypes),
      mNextFileTypes (mFileTypes),
      mSortCriterion (SortCriterion::PRIORITY),
      mThreadPool (ThreadPoolSingleton::getInstance())
{
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");
    qRegisterMetaType<QVector<int>>("QVector<int>");

    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &TransfersManagerSortFilterProxyModel::onModelSortedFiltered);

    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

TransfersManagerSortFilterProxyModel::~TransfersManagerSortFilterProxyModel()
{
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

    QFuture<void> filtered = QtConcurrent::run([this, order, sortCriterion](){
        auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        sourceM->blockSignals(true);
        invalidate();
        QSortFilterProxyModel::sort(0, order);
        sourceM->lockModelMutex(false);
        sourceM->blockSignals(false);
    });
    mFilterWatcher.setFuture(filtered);
}

void TransfersManagerSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TransfersManagerSortFilterProxyModel::onRowsAboutToBeRemoved, Qt::DirectConnection);

    if(auto transferModel = dynamic_cast<TransfersModel*>(sourceModel))
    {
        connect(transferModel, &TransfersModel::unblockUi,
                this, &TransfersManagerSortFilterProxyModel::onModelUnblockedRequest, Qt::DirectConnection);
    }

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void TransfersManagerSortFilterProxyModel::setFilterFixedString(const QString& pattern)
{
    mFilterText = pattern;
    updateFilters();

    resetAllCounters();
    emit modelAboutToBeChanged();

    QFuture<void> filtered = QtConcurrent::run([this](){
        auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        sourceM->blockSignals(true);
        invalidate();
        QSortFilterProxyModel::sort(0,  sortOrder());
        sourceM->lockModelMutex(false);
        sourceM->blockSignals(false);
    });

    mFilterWatcher.setFuture(filtered);
}

void TransfersManagerSortFilterProxyModel::textSearchTypeChanged()
{
    updateFilters();

    resetTransfersStateCounters();
    emit modelAboutToBeChanged();

    QFuture<void> filtered = QtConcurrent::run([this](){
        auto sourceM = qobject_cast<TransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        sourceM->blockSignals(true);
        invalidate();
        QSortFilterProxyModel::sort(0, sortOrder());
        sourceM->lockModelMutex(false);
        sourceM->blockSignals(false);
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

//When the central model request an ui unblock, it´s because an action in other thread has finished completely (canceling or failing)
//We invalidate the proxy model to refresh it, as the action in the other thread has the signals blocked to speed up things.
void TransfersManagerSortFilterProxyModel::onModelUnblockedRequest()
{
    invalidate();
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
}

TransferBaseDelegateWidget *TransfersManagerSortFilterProxyModel::createTransferManagerItem(QWidget* parent)
{
    auto item = new TransferManagerDelegateWidget(parent);

    //All are UniqueConnection to avoid reconnecting if thw item already exists in cache and it is not a new item
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
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

    if(d && d->mTag >= 0)
    {
        auto accept = (d->mState & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (toInt(d->mFileType) & mFileTypes);

        if (accept)
        {
            QMutexLocker lock(&mMutex);

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

            //Not needed to add the logic when the d is a sync transfer, as the sync state is permanent
            if(accept && !d->isSyncTransfer() && !mNoSyncTransfers.contains(d->mTag))
            {
                auto wasEmpty(mNoSyncTransfers.isEmpty());

                mNoSyncTransfers.insert(d->mTag);

                if(wasEmpty)
                {
                    emit cancelableTransfersChanged(true);
                }
            }

            //As the active state can change in time, add both logics to add or remove
            if(accept && d->isActive())
            {
                if(!mActiveTransfers.contains(d->mTag))
                {
                    auto wasEmpty(mActiveTransfers.isEmpty());

                    mActiveTransfers.insert(d->mTag);

                    if(wasEmpty)
                    {
                        emit activeTransfersChanged(true);
                    }
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
                    bool wasEmpty(mPausedTransfers.isEmpty());

                    mPausedTransfers.insert(d->mTag);

                    if(wasEmpty)
                    {
                        emit pausedTransfersChanged(true);
                    }
                }
            }
            else
            {
                removePausedTransferFromCounter(d->mTag);
            }
        }

        return accept;
    }

    return false;
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
           if(!mFilterText.isEmpty())
           {
               if (d->mType & TransferData::TRANSFER_UPLOAD)
               {
                   mUlNumber.remove(d->mTag);
               }
               else
               {
                   mDlNumber.remove(d->mTag);
               }

               searchRowsRemoved = true;
           }

           if(!d->isSyncTransfer())
           {
              removeNonSyncedTransferFromCounter(d->mTag);
           }

           removeActiveTransferFromCounter(d->mTag);
           removePausedTransferFromCounter(d->mTag);

       }
   }

   if(searchRowsRemoved)
   {
       searchNumbersChanged();
   }
}

void TransfersManagerSortFilterProxyModel::removeActiveTransferFromCounter(TransferTag tag) const
{
    if(mActiveTransfers.contains(tag))
    {
        auto wasEmpty(mActiveTransfers.isEmpty());

        mActiveTransfers.remove(tag);

        if(!wasEmpty && mActiveTransfers.isEmpty())
        {
            emit activeTransfersChanged(false);
        }
    }
}

void TransfersManagerSortFilterProxyModel::removePausedTransferFromCounter(TransferTag tag) const
{
    if(mPausedTransfers.contains(tag))
    {
        auto wasEmpty(mPausedTransfers.isEmpty());

        mPausedTransfers.remove(tag);

        if(!wasEmpty && mPausedTransfers.isEmpty())
        {
            emit pausedTransfersChanged(false);
        }
    }
}

void TransfersManagerSortFilterProxyModel::removeNonSyncedTransferFromCounter(TransferTag tag) const
{
    if(mNoSyncTransfers.contains(tag))
    {
        auto wasEmpty(mNoSyncTransfers.isEmpty());

        mNoSyncTransfers.remove(tag);

        if(!wasEmpty && mNoSyncTransfers.isEmpty())
        {
            emit cancelableTransfersChanged(false);
        }
    }
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
        }
    }

    return QSortFilterProxyModel::lessThan(left, right);
}

int TransfersManagerSortFilterProxyModel::getPausedTransfers() const
{
    return mPausedTransfers.size();
}

bool TransfersManagerSortFilterProxyModel::areAllPaused() const
{
    return mPausedTransfers.size() == rowCount();
}

bool TransfersManagerSortFilterProxyModel::isAnyPaused() const
{
    return !mPausedTransfers.isEmpty();
}

bool TransfersManagerSortFilterProxyModel::isAnyCancelable() const
{
    return !mNoSyncTransfers.isEmpty();
}

bool TransfersManagerSortFilterProxyModel::isAnyActive() const
{
    return !mActiveTransfers.isEmpty();
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

    for(int row = 0; row < rowCount(); ++row)
    {
        auto sourceIndex(mapToSource(index(row, 0)));
    }

    while (moveOk && row < (proxyRow+count))
    {
        auto sourceIndex(mapToSource(index(proxyRow, 0, proxyParent)));
        int destRow;
        if (destinationChild == rowCount())
        {
            destRow = sourceModel()->rowCount();
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
        auto pause = delegateWidget->getData()->mState != TransferData::TransferState::TRANSFER_PAUSED;
        emit transferPauseResume(pause);
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
                    || data->mState & TransferData::FINISHED_STATES_MASK)
            {
                sourModel->openFolderByTag(tag);
            }
        }
    }
}
