#include "TransfersSortFilterProxyModel.h"
#include "QTransfersModel.h"
#include "TransferManagerDelegateWidget.h"
#include <megaapi.h>
#include <QElapsedTimer>


TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : TransfersSortFilterProxyModelBase(parent),
      mTransferStates (TransferData::STATE_MASK),
      mTransferTypes (TransferData::TYPE_MASK),
      mFileTypes (~TransferData::FileTypes()),
      mNextTransferStates (mTransferStates),
      mNextTransferTypes (mTransferTypes),
      mNextFileTypes (mFileTypes),
      mSortCriterion (SortCriterion::PRIORITY),
      mSearchCountersOn(false),
      mThreadPool (ThreadPoolSingleton::getInstance())
{
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");

    connect(&mFilterWatcher, &QFutureWatcher<void>::finished,
            this, &TransfersSortFilterProxyModel::onModelSortedFiltered);

    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

TransfersSortFilterProxyModel::~TransfersSortFilterProxyModel()
{
}

void TransfersSortFilterProxyModel::sort(int sortCriterion, Qt::SortOrder order)
{
    if(!dynamicSortFilter())
    {
        setDynamicSortFilter(true);
    }

    auto sourceM = qobject_cast<QTransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(false);
    }

    emit modelAboutToBeChanged();
    if (sortCriterion != static_cast<int>(mSortCriterion))
    {
        mSortCriterion = static_cast<SortCriterion>(sortCriterion);
    }

    QFuture<void> filtered = QtConcurrent::run([this, order, sortCriterion](){
        auto sourceM = qobject_cast<QTransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        QSortFilterProxyModel::sort(sortCriterion, order);
        sourceM->lockModelMutex(false);
    });
    mFilterWatcher.setFuture(filtered);
}

void TransfersSortFilterProxyModel::invalidate()
{
    emit modelAboutToBeChanged();

    mSearchCountersOn = false;
    QSortFilterProxyModel::invalidate();
    mSearchCountersOn = true;

    emit modelChanged();
    emit searchNumbersChanged();
}

void TransfersSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TransfersSortFilterProxyModel::onRowsAboutToBeRemoved, Qt::DirectConnection);

    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
            this, &TransfersSortFilterProxyModel::onRowsAboutToBeInserted);

    connect(sourceModel, &QAbstractItemModel::rowsInserted,
            this, &TransfersSortFilterProxyModel::onRowsInserted);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void TransfersSortFilterProxyModel::setFilterFixedString(const QString& pattern)
{
    mSearchCountersOn = true;
    mFilterText = pattern;
    updateFilters();

    emit modelAboutToBeChanged();

    QFuture<void> filtered = QtConcurrent::run([this, &pattern](){
        auto sourceM = qobject_cast<QTransfersModel*>(sourceModel());
        sourceM->lockModelMutex(true);
        invalidateFilter();
        sourceM->lockModelMutex(false);
    });
    mFilterWatcher.setFuture(filtered);
}

void TransfersSortFilterProxyModel::onModelSortedFiltered()
{
    auto sourceM = qobject_cast<QTransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(false);
    }

    emit modelChanged();
    emit searchNumbersChanged();

    mSearchCountersOn = false;
}

void TransfersSortFilterProxyModel::setFilters(const TransferData::TransferTypes transferTypes,
                                               const TransferData::TransferStates transferStates,
                                               const TransferData::FileTypes fileTypes)
{
    mNextTransferTypes = transferTypes ? transferTypes : TransferData::TYPE_MASK;
    mNextTransferStates = transferStates ? transferStates : TransferData::STATE_MASK;
    if (fileTypes)
    {
        mNextFileTypes = fileTypes;
    }
    else
    {
        mNextFileTypes = ~TransferData::FileTypes({});
    }

    //applyFilters();
}

void TransfersSortFilterProxyModel::resetAllFilters()
{
    resetNumberOfItems();
    setFilters({}, {}, {});
}

int  TransfersSortFilterProxyModel::getNumberOfItems(TransferData::TransferType transferType)
{
    int nb(0);

    if(transferType == TransferData::TransferType::TRANSFER_UPLOAD)
    {
        nb = mUlNumber;
    }
    else if(transferType == TransferData::TransferType::TRANSFER_DOWNLOAD)
    {
        nb = mDlNumber;
    }

    return nb;
}

void TransfersSortFilterProxyModel::resetNumberOfItems()
{
    mDlNumber = 0;
    mUlNumber = 0;
}

TransferBaseDelegateWidget *TransfersSortFilterProxyModel::createTransferManagerItem(QWidget* parent)
{
    auto item = new TransferManagerDelegateWidget(parent);

    //All are UniqueConnection to avoid reconnecting if thw item already exists in cache and it is not a new item
    connect(item, &TransferManagerDelegateWidget::cancelTransfer,
            this, &TransfersSortFilterProxyModel::onCancelClearTransfer);
    connect(item, &TransferManagerDelegateWidget::pauseResumeTransfer,
            this, &TransfersSortFilterProxyModel::onPauseResumeTransfer);
    connect(item, &TransferManagerDelegateWidget::retryTransfer,
             this, &TransfersSortFilterProxyModel::onRetryTransfer);

    return item;
}

void TransfersSortFilterProxyModel::updateFilters()
{
    auto sourceM = qobject_cast<QTransfersModel*>(sourceModel());
    if(sourceM)
    {
        sourceM->pauseModelProcessing(false);
    }

    mTransferStates = mNextTransferStates;
    mTransferTypes = mNextTransferTypes;
    mFileTypes = mNextFileTypes;
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

    if(d->mTag >= 0)
    {
        auto accept = (d->mState & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (d->mFileType & mFileTypes);

        if (accept)
        {
            if(!mFilterText.isEmpty())
            {
                accept = d->mFilename.contains(mFilterText);

                if (accept && mSearchCountersOn)
                {
                    if (d->mType & TransferData::TRANSFER_UPLOAD)
                    {
                        mUlNumber++;
                    }
                    else if (d->mType & TransferData::TRANSFER_DOWNLOAD)
                    {
                        mDlNumber++;
                    }
                }
            }

        }

        return accept;
    }

    return false;
}

//It is called in the main thread, from a QtConcurrent thread
void TransfersSortFilterProxyModel::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
   if(mFilterText.isEmpty())
   {
       return;
   }

   bool RowsRemoved(false);

   for(int row = first; row <= last; ++row)
   {
       QModelIndex index = sourceModel()->index(row, 0, parent);

       const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

       if(d->mTag >= 0)
       {
           if (d->mType & TransferData::TRANSFER_UPLOAD)
           {
               mUlNumber--;
           }
           else
           {
               mDlNumber--;
           }

           RowsRemoved = true;
       }
   }

   if(RowsRemoved)
   {
       searchNumbersChanged();
   }
}

void TransfersSortFilterProxyModel::onRowsAboutToBeInserted(const QModelIndex &, int , int )
{
    if(!mSearchCountersOn && !mFilterText.isEmpty())
    {
        mSearchCountersOn = true;
    }
}

void TransfersSortFilterProxyModel::onRowsInserted(const QModelIndex &, int , int )
{
    if(mSearchCountersOn && !mFilterText.isEmpty())
    {
        mSearchCountersOn = false;
    }
}

bool TransfersSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftItem (qvariant_cast<TransferItem>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem>(right.data()).getTransferData());

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
    }

    return false;
}

int TransfersSortFilterProxyModel::getPausedTransfers() const
{
    auto paused(0);

    for(int row = 0; row < rowCount(); ++row)
    {
        QModelIndex proxyIndex = index(row, 0);

        const auto d (qvariant_cast<TransferItem>(proxyIndex.data()).getTransferData());

        if(d->mState & TransferData::TransferState::TRANSFER_PAUSED)
        {
            paused++;
        }
    }

    return paused;
}

bool TransfersSortFilterProxyModel::isAnyPaused() const
{
    auto paused(false);
    auto last = rowCount();

    for(int row = 0; row < last; ++row)
    {
        QModelIndex proxyIndex = index(row, 0);

        const auto d (qvariant_cast<TransferItem>(proxyIndex.data()).getTransferData());

        if(d->mState & TransferData::TransferState::TRANSFER_PAUSED)
        {
            paused = true;
            break;
        }
    }

    return paused;
}

bool TransfersSortFilterProxyModel::moveRows(const QModelIndex &proxyParent, int proxyRow, int count,
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

void TransfersSortFilterProxyModel::onCancelClearTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<QTransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        QModelIndexList indexes;
        auto index = delegateWidget->getCurrentIndex();
        index = mapToSource(index);
        indexes.append(index);
        sourModel->cancelClearTransfers(indexes, false);
    }
}

void TransfersSortFilterProxyModel::onPauseResumeTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<QTransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        auto tag = delegateWidget->getData()->mTag;
        auto pause = delegateWidget->getData()->mState != TransferData::TransferState::TRANSFER_PAUSED;
        sourModel->pauseResumeTransferByTag(tag,
                                            pause);
        emit transferPauseResume(pause);
    }
}

void TransfersSortFilterProxyModel::onRetryTransfer()
{
    auto delegateWidget = dynamic_cast<TransferManagerDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<QTransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        auto tag = delegateWidget->getData()->mTag;
        sourModel->onRetryTransfer(tag);
    }
}
