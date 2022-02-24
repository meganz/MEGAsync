#ifndef INFODIALOGCURRENTTRANSFERSPROXYMODEL_H
#define INFODIALOGCURRENTTRANSFERSPROXYMODEL_H

#include "TransfersSortFilterProxyModel.h"

class TransferBaseDelegateWidget;
class MegaDelegateHoverManager;

class InfoDialogCurrentTransfersProxyModel : public TransfersSortFilterProxyModelBase
{
    Q_OBJECT

public:
    InfoDialogCurrentTransfersProxyModel(QObject* parent);
    ~InfoDialogCurrentTransfersProxyModel();

    TransferBaseDelegateWidget* createTransferManagerItem(QWidget *parent) override;

protected slots:
    void onCopyTransferLinkRequested();
    void onOpenTransferFolderRequested();

protected:
    bool lessThan(const QModelIndex &left, const QModelIndex &right) const override;
    bool filterAcceptsRow(int sourceRow, const QModelIndex& sourceParent) const override;

};

#endif // INFODIALOGCURRENTTRANSFERSPROXYMODEL_H
