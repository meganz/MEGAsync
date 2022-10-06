#include "InfoDialogTransfersProxyModel.h"

#include "InfoDialogTransferDelegateWidget.h"
#include "TransfersModel.h"

#include <QElapsedTimer>

//SORT FILTER PROXY MODEL
InfoDialogTransfersProxyModel::InfoDialogTransfersProxyModel(QObject *parent) :
    TransfersSortFilterProxyBaseModel(parent),
    mNextUploadSourceRow(-1),
    mNextDownloadSourceRow(-1)
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
    auto sourceM = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourceM)
    {
        QList<int> rows;
        auto index = delegateWidget->getCurrentIndex();
        index = mapToSource(index);
        rows.append(index.row());
        sourceM->getLinks(rows);
    }
}

void InfoDialogTransfersProxyModel::onOpenTransferFolderRequested()
{
    auto delegateWidget = dynamic_cast<InfoDialogTransferDelegateWidget*>(sender());
    auto sourceM = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourceM)
    {
        auto index = delegateWidget->getCurrentIndex();
        index = mapToSource(index);
        sourceM->openFolderByIndex(index);
    }
}

void InfoDialogTransfersProxyModel::onRetryTransferRequested()
{
    auto delegateWidget = dynamic_cast<InfoDialogTransferDelegateWidget*>(sender());
    auto sourceM = dynamic_cast<TransfersModel*>(sourceModel());

    if(delegateWidget && sourceM)
    {
        sourceM->retryTransferByIndex(mapToSource(delegateWidget->getCurrentIndex()));
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
            //Uploads before downloads
            return leftItem->mType > rightItem->mType;
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

           if(!accept && (d->mTag == mNextUploadSourceRow || d->mTag == mNextDownloadSourceRow))
           {
               accept = true;
           }
       }
    }

    return accept;
}

void InfoDialogTransfersProxyModel::onUpdateMostPriorityTransfer(int uploadTag, int downloadTag)
{
    QModelIndex indexToUpdate(index(0,0));
    //Uploads before downloads. If you want to change it, change also the if condition in the lessThan method
    updateMostPriortyTransfer(mNextUploadSourceRow, uploadTag, indexToUpdate);
    updateMostPriortyTransfer(mNextDownloadSourceRow, downloadTag, indexToUpdate);
}

void InfoDialogTransfersProxyModel::updateMostPriortyTransfer(int& tagToUpdate, TransferTag tag, QModelIndex &indexToUpdate)
{
    if(tag != tagToUpdate)
    {
        auto transferModel = dynamic_cast<TransfersModel*>(sourceModel());
        tagToUpdate = tag;

        if(indexToUpdate.isValid())
        {
            const auto d (qvariant_cast<TransferItem>(indexToUpdate.data()).getTransferData());
            if(d)
            {
                if(tag >= 0)
                {
                    transferModel->sendDataChangedByTag(tag);
                }
                transferModel->sendDataChangedByTag(d->mTag);
            }
        }
        else if(tag >= 0 && transferModel->getTransferByTag(tag))
        {
            transferModel->sendDataChangedByTag(tag);
        }
    }

    //Move the index to update to the next row, as the current one is used
    if(tag >= 0)
    {
        indexToUpdate = index(indexToUpdate.row() + 1,0);
    }

}
