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
      mActivityMutex (new QMutex(QMutex::Recursive))
{
    // Allow only one thread to sort/filter at a time
    connect(this, &TransfersSortFilterProxyModel::layoutAboutToBeChanged,
            this, [this]
    {
        emit modelAboutToBeSorted();
        mActivityMutex->lock();
    });
    connect(this, &TransfersSortFilterProxyModel::layoutChanged,
            this, [this]
    {
        mActivityMutex->unlock();
        emit modelSorted();
    });

//    connect(this, &TransfersSortFilterProxyModel::modelAboutToBeReset,
//            this, [this]
//    {
//        mSortingMutex->lock();
//    });
//    connect(this, &TransfersSortFilterProxyModel::modelReset,
//            this, [this]
//    {
//        mSortingMutex->unlock();
//    });

//    connect(this, &TransfersSortFilterProxyModel::modelAboutToBeFiltered,
//            this, [this]
//    {
//        mSortingMutex->lock();
//    }, Qt::QueuedConnection);
//    connect(this, &TransfersSortFilterProxyModel::modelFiltered,
//            this, [this]
//    {
//        mSortingMutex->unlock();
//    }, Qt::QueuedConnection);
}

TransfersSortFilterProxyModel::~TransfersSortFilterProxyModel()
{
    delete mFilterMutex;
    delete mActivityMutex;
}

void TransfersSortFilterProxyModel::sort(int, Qt::SortOrder order)
{
    sort(mSortCriterion, order);
}

void TransfersSortFilterProxyModel::sort(SortCriterion column, Qt::SortOrder order)
{
    QtConcurrent::run([=]
    {
        emit modelAboutToBeSorted();
        auto transferModel (static_cast<QTransfersModel*> (sourceModel()));
        transferModel->lockModelMutex(true);
        QMutexLocker lockSortingMutex (mActivityMutex);
        if (column != mSortCriterion)
        {
            QSortFilterProxyModel::sort(-1, order);
            mSortCriterion = column;
        }
        QSortFilterProxyModel::sort(0, order);
        transferModel->lockModelMutex(false);
        emit modelSorted();
    });
}

void TransfersSortFilterProxyModel::invalidate()
{
    resetNumberOfItems();
    QSortFilterProxyModel::invalidate();
}

void TransfersSortFilterProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &TransfersSortFilterProxyModel::onRowsRemoved);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void TransfersSortFilterProxyModel::setFilterFixedString(const QString& pattern)
{
    QtConcurrent::run([=]
    {
        emit modelAboutToBeFiltered();
        auto transferModel (static_cast<QTransfersModel*> (sourceModel()));
        transferModel->lockModelMutex(true);
        QMutexLocker lockSortingMutex (mActivityMutex);
        QSortFilterProxyModel::setFilterFixedString(pattern);
        transferModel->lockModelMutex(false);
        emit searchNumbersChanged();
    });
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
}

void TransfersSortFilterProxyModel::resetAllFilters(bool invalidate)
{
    setFilters({}, {}, {});
    applyFilters(invalidate);
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
    resetNumberOfItems();

    QtConcurrent::run([=]
    {
        emit modelAboutToBeFiltered();
        auto transferModel (static_cast<QTransfersModel*> (sourceModel()));
        transferModel->lockModelMutex(true);
        QMutexLocker lockCallingThread (mActivityMutex);
        mTransferStates = mNextTransferStates;
        mTransferTypes = mNextTransferTypes;
        mFileTypes = mNextFileTypes;
        if (invalidate)
        {
            invalidateFilter();
        }
        transferModel->lockModelMutex(false);
        emit searchNumbersChanged();
    });
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QMutexLocker lock (mActivityMutex);
    bool accept (false);

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());

    if(d->mTag >= 0)
    {
        accept = (d->mState & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (d->mFileType & mFileTypes);

        if (accept && !filterRegExp().isEmpty())
        {
            accept = d->mFilename.contains(filterRegExp());

            if (accept)
            {
                if (d->mType & TransferData::TRANSFER_UPLOAD)
                {
                    mUlNumber.append(d->mTag);
                }
                else
                {
                    mDlNumber.append(d->mTag);
                }
            }
        }
    }

    return accept;
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

bool TransfersSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    QMutexLocker lock (mActivityMutex);
    const auto leftItem (qvariant_cast<TransferItem>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem>(right.data()).getTransferData());

    switch (mSortCriterion)
    {
        case SortCriterion::PRIORITY:
        {
            return leftItem->mPriority > rightItem->mPriority;
            break;
        }
        case SortCriterion::TOTAL_SIZE:
        {
            return leftItem->mTotalSize < rightItem->mTotalSize;
            break;
        }
        case SortCriterion::NAME:
        {
            return QString::compare(leftItem->mFilename, rightItem->mFilename, Qt::CaseInsensitive) < 0;
            break;
        }
    }
    return false;
}


bool TransfersSortFilterProxyModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
              const QModelIndex &destinationParent, int destinationChild)
{
    bool moveOk(true);
    int row(sourceRow);
    QMutexLocker lock (mActivityMutex);
    while (moveOk && row < (sourceRow+count))
    {
        auto sourceIndex(mapToSource(index(sourceRow, 0, sourceParent)));
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
        sourModel->pauseResumeTransferByTag(tag,
                                            delegateWidget->getData()->mState != TransferData::TransferState::TRANSFER_PAUSED);
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
