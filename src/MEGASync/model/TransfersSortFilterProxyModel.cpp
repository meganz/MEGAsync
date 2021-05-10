#include "TransfersSortFilterProxyModel.h"
#include "QTransfersModel2.h"
#include <megaapi.h>


TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      mTransferStates (TransferData::STATE_MASK),
      mTransferTypes (TransferData::TYPE_MASK),
      mFileTypes (~TransferData::FileTypes()),
      mNextTransferStates (mTransferStates),
      mNextTransferTypes (mTransferTypes),
      mNextFileTypes (mFileTypes),
      mSortCriterion (SORT_BY::PRIORITY),
      mDlNumber (new int(0)),
      mUlNumber (new int(0)),
      mFilterMutex (new QMutex(QMutex::Recursive)),
      mNewFiltersSemaphore (new QSemaphore(3))
{
    mNewFiltersSemaphore->acquire(3);
}

TransfersSortFilterProxyModel::~TransfersSortFilterProxyModel()
{
    delete mUlNumber;
    delete mDlNumber;
}


void TransfersSortFilterProxyModel::setTransferTypes(TransferData::TransferTypes transferTypes)
{
    QMutexLocker lock (mFilterMutex);
    mNextTransferTypes = transferTypes ? transferTypes : TransferData::TYPE_MASK;
    mNewFiltersSemaphore->release();
}

void TransfersSortFilterProxyModel::setTransferStates(TransferData::TransferStates transferStates)
{
    QMutexLocker lock (mFilterMutex);
    mNextTransferStates = transferStates ? transferStates : TransferData::STATE_MASK;
    mNewFiltersSemaphore->release();
}

void TransfersSortFilterProxyModel::setFileTypes(TransferData::FileTypes fileTypes)
{
    QMutexLocker lock (mFilterMutex);
    if (fileTypes)
    {
        mNextFileTypes = fileTypes;
    }
    else
    {
        mNextFileTypes = ~TransferData::FileTypes({});
    }
    mNewFiltersSemaphore->release();
}

void TransfersSortFilterProxyModel::resetAllFilters()
{
    QMutexLocker lock (mFilterMutex);
    mTransferStates = TransferData::STATE_MASK;
    mTransferTypes = TransferData::TYPE_MASK;
    mFileTypes = ~TransferData::FileTypes();
    mNextTransferStates = mTransferStates;
    mNextTransferTypes = mTransferTypes;
    mNextFileTypes = mFileTypes;
    *mDlNumber = 0;
    *mUlNumber = 0;
    mNewFiltersSemaphore->release(3);
}

void TransfersSortFilterProxyModel::setSortBy(SORT_BY sortCriterion)
{
    mSortCriterion = sortCriterion;
}

int  TransfersSortFilterProxyModel::getNumberOfItems(TransferData::TransferType transferType)
{
    auto nbRows (rowCount());
    if (mTransferTypes == (TransferData::TRANSFER_DOWNLOAD | TransferData::TRANSFER_LTCPDOWNLOAD))
    {
        *mDlNumber = nbRows;
        *mUlNumber = 0;
    }
    else if (mTransferTypes == TransferData::TRANSFER_UPLOAD)
    {
        *mDlNumber = 0;
        *mUlNumber = nbRows;
    }
    else
    {
        if (*mDlNumber + *mUlNumber != nbRows)
        {
            // Case where only one type: easy!
            if (*mDlNumber == 0)
            {
                *mUlNumber = nbRows;
            }
            else if (*mUlNumber == 0)
            {
                *mDlNumber = nbRows;
            }
            // Mixed... we have to count :(
            else
            {
                *mUlNumber = 0;
                for (int i = 0; i < nbRows; ++i)
                {
                    QModelIndex idx (index(i, 0));
                    const auto d (qvariant_cast<TransferItem2>(idx.data()).getTransferData());
                    if (d->mType & TransferData::TRANSFER_UPLOAD)
                    {
                        (*mUlNumber)++;
                    }
                }
                *mDlNumber = nbRows - *mUlNumber;
            }
        }
    }


    if (transferType & (TransferData::TRANSFER_DOWNLOAD | TransferData::TRANSFER_LTCPDOWNLOAD))
    {
        return *mDlNumber;
    }
    else
    {
        return *mUlNumber;
    }
}

void TransfersSortFilterProxyModel::resetNumberOfItems()
{
    QMutexLocker lock (mFilterMutex);
    *mDlNumber = 0;
    *mUlNumber = 0;
}

void TransfersSortFilterProxyModel::applyFilters(bool invalidate)
{
    mNewFiltersSemaphore->acquire(3);
    QMutexLocker lock (mFilterMutex);

    mTransferStates = mNextTransferStates;
    mTransferTypes = mNextTransferTypes;
    mFileTypes = mNextFileTypes;
    if (invalidate)
    {
        invalidateFilter();
    }
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QMutexLocker lock (mFilterMutex);
    bool accept (false);

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

    if (filterRegExp().isEmpty())
    {
        accept = (d->mState & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (d->mFileType & mFileTypes);
    }
    else
    {
        accept = (d->mState & mTransferStates)
                 && (d->mType & mTransferTypes)
                 && (d->mFileType & mFileTypes)
                 && d->mFilename.contains(filterRegExp());

        if (accept)
        {
            if (d->mType & TransferData::TRANSFER_UPLOAD)
            {
                (*mUlNumber)++;
            }
            else
            {
                (*mDlNumber)++;
            }
        }
    }

    return accept;
}

bool TransfersSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    bool lessThan (false);
    const auto leftItem (qvariant_cast<TransferItem2>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem2>(right.data()).getTransferData());

    switch (mSortCriterion)
    {
        case SORT_BY::PRIORITY:
        {
            lessThan = leftItem->mPriority < rightItem->mPriority;
            break;
        }
        case SORT_BY::TOTAL_SIZE:
        {
            lessThan = leftItem->mTotalSize < rightItem->mTotalSize;
            break;
        }
        case SORT_BY::NAME:
        {
            lessThan = leftItem->mFilename < rightItem->mFilename;
            break;
        }
        default:
            break;
    }

    return lessThan;
}

bool TransfersSortFilterProxyModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
              const QModelIndex &destinationParent, int destinationChild)
{
    bool moveOk(true);
    int row(sourceRow);
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
