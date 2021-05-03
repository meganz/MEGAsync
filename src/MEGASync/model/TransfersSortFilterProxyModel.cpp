#include "TransfersSortFilterProxyModel.h"
#include "QTransfersModel2.h"
#include <megaapi.h>


TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      mTransferTypes (TransferData::TYPE_MASK),
      mTransferStates (TransferData::STATE_MASK),
      mFileTypes (~TransferData::FileTypes()),
      mSortCriterion (SORT_BY::PRIORITY),
      mDlNumber (new int(0)),
      mUlNumber (new int(0))
{
}

TransfersSortFilterProxyModel::~TransfersSortFilterProxyModel()
{
    delete mUlNumber;
    delete mDlNumber;
}


void TransfersSortFilterProxyModel::setTransferTypes(TransferData::TransferTypes transferTypes)
{
    mTransferTypes = transferTypes ? transferTypes : TransferData::TYPE_MASK;
}

void TransfersSortFilterProxyModel::addTransferTypes(TransferData::TransferTypes transferTypes)
{
    mTransferTypes |= transferTypes;
}

void TransfersSortFilterProxyModel::setTransferStates(TransferData::TransferStates transferStates)
{
    mTransferStates = transferStates ? transferStates : TransferData::STATE_MASK;
}

void TransfersSortFilterProxyModel::addTransferStates(TransferData::TransferStates transferStates)
{
    mTransferStates |= transferStates;
}

void TransfersSortFilterProxyModel::setFileTypes(TransferData::FileTypes fileTypes)
{
    if (fileTypes)
    {
        mFileTypes = fileTypes;
    }
    else
    {
        mFileTypes = ~TransferData::FileTypes({});
    }
}

void TransfersSortFilterProxyModel::addFileTypes(TransferData::FileTypes fileTypes)
{
    mFileTypes |= fileTypes;
}

void TransfersSortFilterProxyModel::resetAllFilters()
{
    mTransferStates = TransferData::STATE_MASK;
    mTransferTypes = TransferData::TYPE_MASK;
    mFileTypes = ~TransferData::FileTypes();
    *mDlNumber = 0;
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
                    if (d->mType == TransferData::TRANSFER_UPLOAD)
                    {
                        (*mUlNumber)++;
                    }
                }
                *mDlNumber = nbRows - *mUlNumber;
            }
        }
    }


    if (transferType == TransferData::TRANSFER_DOWNLOAD)
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
    *mDlNumber = 0;
    *mUlNumber = 0;
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

    if (filterRegExp().isEmpty())
    {
        return     (d->mState & mTransferStates)
                && (d->mType & mTransferTypes)
                && (d->mFileType & mFileTypes);
    }

    bool accept ( (d->mState & mTransferStates)
                  && (d->mType & mTransferTypes)
                  && (d->mFileType & mFileTypes)
                  && d->mFilename.contains(filterRegExp()));

    if (accept && d->mType == TransferData::TRANSFER_UPLOAD)
    {
        (*mUlNumber)++;
    }
    else
    {
        (*mDlNumber)++;
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
