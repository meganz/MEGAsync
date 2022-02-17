#include "TransfersSortFilterProxyModel.h"
#include "QTransfersModel.h"
#include "TransferManagerDelegateWidget.h"
#include <megaapi.h>
#include <QElapsedTimer>


TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      mTransferStates (TransferData::STATE_MASK),
      mTransferTypes (TransferData::TYPE_MASK),
      mFileTypes (~TransferData::FileTypes()),
      mNextTransferStates (mTransferStates),
      mNextTransferTypes (mTransferTypes),
      mNextFileTypes (mFileTypes),
      mSortCriterion (SortCriterion::PRIORITY),
      mFilterMutex (new QMutex(QMutex::Recursive)),
      mActivityMutex (new QMutex(QMutex::Recursive)),
      mSearchCountersOn(false),
      mThreadPool (ThreadPoolSingleton::getInstance())
{
    // Allow only one thread to sort/filter at a time
    connect(this, &TransfersSortFilterProxyModel::modelAboutToBeChanged,
            this, [this]
    {
    });
    connect(this, &TransfersSortFilterProxyModel::modelChanged,
            this, [this]
    {
        //auto transferModel (static_cast<QTransfersModel*> (sourceModel()));
        //transferModel->pauseModelProcessing(false);
    });

    setFilterCaseSensitivity(Qt::CaseInsensitive);
}

TransfersSortFilterProxyModel::~TransfersSortFilterProxyModel()
{
    delete mFilterMutex;
    delete mActivityMutex;
}

void TransfersSortFilterProxyModel::sort(int column, Qt::SortOrder order)
{
    sort(mSortCriterion, order);

    if(column == 0 && !dynamicSortFilter())
    {
        setDynamicSortFilter(true);
    }

}

void TransfersSortFilterProxyModel::sort(SortCriterion column, Qt::SortOrder order)
{
    emit modelAboutToBeChanged();

    if (column != mSortCriterion)
    {
        QSortFilterProxyModel::sort(-1, order);
        mSortCriterion = column;
    }
    QSortFilterProxyModel::sort(0, order);

    emit modelChanged();
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
            this, &TransfersSortFilterProxyModel::onRowsRemoved);

    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeInserted,
            this, &TransfersSortFilterProxyModel::onRowsAboutToBeInserted);

    connect(sourceModel, &QAbstractItemModel::rowsInserted,
            this, &TransfersSortFilterProxyModel::onRowsInserted);

    connect(sourceModel, &QAbstractItemModel::rowsInserted,
            this, &TransfersSortFilterProxyModel::onRowsInserted);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void TransfersSortFilterProxyModel::setFilterFixedString(const QString& pattern)
{
    mSearchCountersOn = true;
    QSortFilterProxyModel::setFilterFixedString(pattern);
    mSearchCountersOn = false;

    emit searchNumbersChanged();
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

    applyFilters(true);
}

void TransfersSortFilterProxyModel::resetAllFilters(bool invalidate)
{
    resetNumberOfItems();
    setFilters({}, {}, {});
}

int  TransfersSortFilterProxyModel::getNumberOfItems(TransferData::TransferType transferType)
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

void TransfersSortFilterProxyModel::resetNumberOfItems()
{
    mDlNumber.clear();
    mUlNumber.clear();
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

void TransfersSortFilterProxyModel::applyFilters(bool invalidate)
{
    emit modelAboutToBeChanged();

    mTransferStates = mNextTransferStates;
    mTransferTypes = mNextTransferTypes;
    mFileTypes = mNextFileTypes;

    if (invalidate)
    {
        invalidateFilter();
    }

    emit modelChanged();
    emit searchNumbersChanged();
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
            if(!filterRegExp().isEmpty())
            {
                accept = d->mFilename.contains(filterRegExp());

                if (accept && mSearchCountersOn)
                {
                    if (d->mType & TransferData::TRANSFER_UPLOAD)
                    {
                        if(!mUlNumber.contains(d->mTag))
                        {
                            mUlNumber.append(d->mTag);
                        }
                    }
                    else if (d->mType & TransferData::TRANSFER_DOWNLOAD)
                    {
                        if(!mDlNumber.contains(d->mTag))
                        {
                            mDlNumber.append(d->mTag);
                        }
                    }
                }
            }

        }

        return accept;
    }

    return false;
}

void TransfersSortFilterProxyModel::onRowsRemoved(const QModelIndex &parent, int first, int last)
{
   if(filterRegExp().isEmpty())
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
               mUlNumber.removeOne(d->mTag);
           }
           else
           {
               mDlNumber.removeOne(d->mTag);
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
    if(!mSearchCountersOn && !filterRegExp().isEmpty())
    {
        mSearchCountersOn = true;
    }
}

void TransfersSortFilterProxyModel::onRowsInserted(const QModelIndex &, int , int )
{
    if(mSearchCountersOn && !filterRegExp().isEmpty())
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

int TransfersSortFilterProxyModel::areAllPaused() const
{
    auto paused(0);
    auto last = rowCount();

    for(int row = 0; row < last; ++row)
    {
        QModelIndex proxyIndex = index(row, 0);

        const auto d (qvariant_cast<TransferItem>(proxyIndex.data()).getTransferData());

        if(d->mState & TransferData::TransferState::TRANSFER_PAUSED)
        {
            paused++;
        }
    }

    return (last - paused);
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
