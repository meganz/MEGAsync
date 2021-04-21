#include "TransfersSortFilterProxyModel.h"
#include "QTransfersModel2.h"
#include <megaapi.h>


TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      mTransferTypes (),
      mTransferStates (~TransferData::TransferStates({})),
      mFileTypes (~TransferData::FileTypes({})),
      mSortCriterion (SORT_BY::PRIORITY)
{
}

void TransfersSortFilterProxyModel::setTransferTypes(const QSet<int>& transferTypes)
{
    mTransferTypes = transferTypes;
}

void TransfersSortFilterProxyModel::addTransferTypes(const QSet<int>& transferTypes)
{
    mTransferTypes += transferTypes;
}

void TransfersSortFilterProxyModel::setTransferStates(const TransferData::TransferStates transferStates)
{
    if (transferStates)
    {
        mTransferStates = transferStates;
    }
    else
    {
        mTransferStates = ~TransferData::TransferStates({});
    }
}

void TransfersSortFilterProxyModel::addTransferStates(const TransferData::TransferStates transferStates)
{
    mTransferStates |= transferStates;
}

void TransfersSortFilterProxyModel::setFileTypes(const TransferData::FileTypes fileTypes)
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

void TransfersSortFilterProxyModel::addFileTypes(const TransferData::FileTypes fileTypes)
{
    mFileTypes |= fileTypes;
}

void TransfersSortFilterProxyModel::resetAllFilters()
{
    mTransferTypes.clear();
    mFileTypes = ~TransferData::FileTypes({});
    mTransferStates = ~TransferData::TransferStates({});
}

void TransfersSortFilterProxyModel::setSortBy(SORT_BY sortCriterion)
{
    mSortCriterion = sortCriterion;
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    const auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

    return     (d->mState & mTransferStates)
            && (mTransferTypes.isEmpty()  || mTransferTypes.contains(d->mType))
            && (d->mFileType & mFileTypes)
            && d->mFilename.contains(filterRegExp());
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
