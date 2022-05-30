#ifndef INFODIALOGCURRENTTRANSFERSPROXYMODEL_H
#define INFODIALOGCURRENTTRANSFERSPROXYMODEL_H

#include "TransfersManagerSortFilterProxyModel.h"

class TransferBaseDelegateWidget;
class MegaDelegateHoverManager;

class InfoDialogTransfersProxyModel : public TransfersSortFilterProxyBaseModel
{
    Q_OBJECT

public:
    InfoDialogTransfersProxyModel(QObject* parent);
    ~InfoDialogTransfersProxyModel();

    TransferBaseDelegateWidget* createTransferManagerItem(QWidget *parent) override;

    void setSourceModel(QAbstractItemModel* sourceModel) override;

    int rowCount(const QModelIndex &parent) const override;

    void invalidate();

protected slots:
    void onCopyTransferLinkRequested();
    void onOpenTransferFolderRequested();
    void onRetryTransferRequested();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

private:
    mutable unsigned long long mNextTransferPriority;
    mutable int mNextTransferSourceRow;
    mutable bool mInvalidating;

};

#endif // INFODIALOGCURRENTTRANSFERSPROXYMODEL_H
