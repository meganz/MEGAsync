#include "InfoDialogTransfersProxyModel.h"

#include "InfoDialogTransferDelegateWidget.h"
#include "TransfersModel.h"

#include <QElapsedTimer>

//SORT FILTER PROXY MODEL
InfoDialogTransfersProxyModel::InfoDialogTransfersProxyModel(QObject *parent) :
    TransfersSortFilterProxyBaseModel(parent),
    mNextTransferSourceRow(-1)
{
}

InfoDialogTransfersProxyModel::~InfoDialogTransfersProxyModel()
{
}

TransferBaseDelegateWidget* InfoDialogTransfersProxyModel::createTransferManagerItem(QWidget*)
{
    auto item = new InfoDialogTransferDelegateWidget(nullptr);

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
    QSortFilterProxyModel::setSourceModel(sourceModel);

    if(auto transferModel = dynamic_cast<TransfersModel*>(sourceModel))
    {
        connect(transferModel, &TransfersModel::mostPriorityTransferUpdate,
                this, &InfoDialogTransfersProxyModel::onUpdateMostPriorityTransfer);

        connect(transferModel, &TransfersModel::unblockUiAndFilter, this, &InfoDialogTransfersProxyModel::invalidate);
    }
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
        if(leftItem->isActiveOrPending() && !rightItem->isActiveOrPending())
        {
            return true;
        }
        else if(rightItem->isActiveOrPending() && !leftItem->isActiveOrPending())
        {
            return false;
        }
        else if(leftItem->isActiveOrPending() && rightItem->isActiveOrPending())
        {
            return leftItem->mPriority < rightItem->mPriority;
        }
        else
        {
            return leftItem->getRawFinishedTime() > rightItem->getRawFinishedTime();
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
           accept = (d->getState() & TransferData::TransferState::TRANSFER_COMPLETED
                     || d->getState() & TransferData::TransferState::TRANSFER_COMPLETING
                     || d->getState() & TransferData::TransferState::TRANSFER_ACTIVE
                     || d->getState() & TransferData::TransferState::TRANSFER_FAILED);

           if(!accept && d->mTag == mNextTransferSourceRow)
           {
               accept = true;
           }
       }
    }

    return accept;
}

void InfoDialogTransfersProxyModel::onUpdateMostPriorityTransfer(int tag)
{
    mNextTransferSourceRow = tag;
    auto transferModel = dynamic_cast<TransfersModel*>(sourceModel());

    auto firstIndex = index(0,0);
    if(firstIndex.isValid())
    {
        const auto d (qvariant_cast<TransferItem>(firstIndex.data()).getTransferData());
        if(d)
        {
            transferModel->sendDataChangedByTag(tag);
            transferModel->sendDataChangedByTag(d->mTag);
        }
    }
    else
    {
        if(!transferModel->getTransferByTag(tag))
        {
            invalidate();
        }
        else
        {
            transferModel->sendDataChangedByTag(tag);
        }
    }
}
