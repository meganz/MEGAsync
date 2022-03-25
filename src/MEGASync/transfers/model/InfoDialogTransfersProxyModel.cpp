#include "InfoDialogTransfersProxyModel.h"

#include "InfoDialogTransferDelegateWidget.h"
#include "TransfersModel.h"

//SORT FILTER PROXY MODEL
InfoDialogTransfersProxyModel::InfoDialogTransfersProxyModel(QObject *parent) : mNextTransferTag(-1), TransfersSortFilterProxyBaseModel(parent)
{
}

InfoDialogTransfersProxyModel::~InfoDialogTransfersProxyModel()
{
}

TransferBaseDelegateWidget* InfoDialogTransfersProxyModel::createTransferManagerItem(QWidget *parent)
{
    auto item = new InfoDialogTransferDelegateWidget(parent);

    connect(item, &InfoDialogTransferDelegateWidget::copyTransferLink,
            this, &InfoDialogTransfersProxyModel::onCopyTransferLinkRequested);
    connect(item, &InfoDialogTransferDelegateWidget::openTransferFolder,
            this, &InfoDialogTransfersProxyModel::onOpenTransferFolderRequested);
    connect(item, &InfoDialogTransferDelegateWidget::retryTransfer,
            this, &InfoDialogTransfersProxyModel::onRetryTransferRequested);

    return item;
}

void InfoDialogTransfersProxyModel::setSourceModel(QAbstractItemModel *sourceModel)
{
    connect(sourceModel, &QAbstractItemModel::rowsAboutToBeRemoved,
            this, &InfoDialogTransfersProxyModel::onRowsAboutToBeRemoved, Qt::DirectConnection);

    QSortFilterProxyModel::setSourceModel(sourceModel);
}

void InfoDialogTransfersProxyModel::onCopyTransferLinkRequested()
{
    auto delegateWidget = dynamic_cast<InfoDialogTransferDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        QList<int> rows;
        auto index = delegateWidget->getCurrentIndex();
        index = mapToSource(index);
        rows.append(index.row());
        sourModel->getLinks(rows);
    }
}

void InfoDialogTransfersProxyModel::onOpenTransferFolderRequested()
{
    auto delegateWidget = dynamic_cast<InfoDialogTransferDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        auto index = delegateWidget->getCurrentIndex();
        index = mapToSource(index);
        sourModel->openFolderByIndex(index);
    }
}

void InfoDialogTransfersProxyModel::onRetryTransferRequested()
{
    auto delegateWidget = dynamic_cast<InfoDialogTransferDelegateWidget*>(sender());
    auto sourModel = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourModel)
    {
        sourModel->retryTransferByIndex(mapToSource(delegateWidget->getCurrentIndex()));
    }
}

bool InfoDialogTransfersProxyModel::lessThan(const QModelIndex &left, const QModelIndex &right) const
{
    const auto leftItem (qvariant_cast<TransferItem>(left.data()).getTransferData());
    const auto rightItem (qvariant_cast<TransferItem>(right.data()).getTransferData());

    if(leftItem && rightItem)
    {
        if(leftItem->isActive() && !rightItem->isActive())
        {
            return true;
        }
        else if(rightItem->isActive() && !leftItem->isActive())
        {
            return false;
        }
        else if(leftItem->isActive() && rightItem->isActive())
        {
            return leftItem->mPriority < rightItem->mPriority;
        }
        else
        {
            return leftItem->getFinishedTime() < rightItem->getFinishedTime();
        }
    }
    else
    {
        return QSortFilterProxyModel::lessThan(left, right);
    }
}

bool InfoDialogTransfersProxyModel::filterAcceptsRow(int sourceRow, const QModelIndex &sourceParent) const
{
    bool accept (false);

    QModelIndex index = sourceModel()->index(sourceRow, 0, sourceParent);

    if(index.isValid())
    {
       const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());
       if(d)
       {
           accept = (d->mState & TransferData::TransferState::TRANSFER_COMPLETED
                     || d->mState & TransferData::TransferState::TRANSFER_COMPLETING
                     || d->mState & TransferData::TransferState::TRANSFER_ACTIVE || d->mState & TransferData::TransferState::TRANSFER_FAILED);


           //Show next transfer to process
           if(accept && d->mTag == mNextTransferTag)
           {
               mNextTransferTag = -1;
           }
           else if(!accept && (mNextTransferTag < 0 || mNextTransferTag == d->mTag || mNextTransferTag > d->mTag))
           {
               accept = true;
               mNextTransferTag = d->mTag;
           }
       }
    }

    return accept;
}

void InfoDialogTransfersProxyModel::onRowsAboutToBeRemoved(const QModelIndex &parent, int first, int last)
{
    //Update next transfer to process
    for(int row = first; row <= last; ++row)
    {
        QModelIndex index = sourceModel()->index(row, 0, parent);

        if(index.isValid())
        {
            const auto d (qvariant_cast<TransferItem>(index.data()).getTransferData());
            if(d && mNextTransferTag == d->mTag)
            {
                mNextTransferTag = -1;
                break;
            }
        }
    }
}
