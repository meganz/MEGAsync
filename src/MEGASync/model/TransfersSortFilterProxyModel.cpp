#include "TransfersSortFilterProxyModel.h"
#include <megaapi.h>

TransfersSortFilterProxyModel::TransfersSortFilterProxyModel(QObject* parent)
    : QSortFilterProxyModel(parent),
      mTransferType(QSet<int>()),
      mTransferState(QSet<int>()),
      mFileType(QSet<TransferData::FileTypes>())
{
}

void TransfersSortFilterProxyModel::setTransferType(const QSet<int> transferTypes)
{
    mTransferType = transferTypes;
    invalidateFilter();
}

void TransfersSortFilterProxyModel::addTransferType(const QSet<int> transferTypes)
{
    mTransferType += transferTypes;
    invalidateFilter();
}

void TransfersSortFilterProxyModel::setTransferState(const QSet<int> transferStates)
{
    mTransferState = transferStates;
    invalidateFilter();
}

void TransfersSortFilterProxyModel::addTransferState(const QSet<int> transferStates)
{
    mTransferState += transferStates;
    invalidateFilter();
}

void TransfersSortFilterProxyModel::setFileType(const QSet<TransferData::FileTypes> fileTypes)
{
    mFileType = fileTypes;
    invalidateFilter();
}

void TransfersSortFilterProxyModel::addFileType(const QSet<TransferData::FileTypes> fileTypes)
{
    mFileType += fileTypes;
    invalidateFilter();
}

bool TransfersSortFilterProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    auto d (qvariant_cast<TransferItem2>(index.data()).getTransferData());

    return     (mTransferState.isEmpty() || mTransferState.contains(d->mState))
            && (mTransferType.isEmpty()  || mTransferType.contains(d->mType))
            && (mFileType.isEmpty()      || mFileType.contains(d->mFileType))
            && d->mFilename.contains(filterRegExp());
}

bool TransfersSortFilterProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftItem (qvariant_cast<TransferItem2>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem2>(right.data()).getTransferData());

    return leftItem->mPriority > rightItem->mPriority;
}

