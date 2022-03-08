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

protected slots:
    void onCopyTransferLinkRequested();
    void onOpenTransferFolderRequested();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

};

#endif // INFODIALOGCURRENTTRANSFERSPROXYMODEL_H
