#ifndef INFODIALOGTRANSFERSWIDGET_H
#define INFODIALOGTRANSFERSWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include "QTransfersModel.h"
#include "MegaTransferDelegate.h"
#include "TransfersSortFilterProxyModel.h"

namespace Ui {
class InfoDialogTransfersWidget;
}

class TransferBaseDelegateWidget;
class MegaDelegateHoverManager;

class InfoDialogCurrentTransfersProxyModel : public TransfersSortFilterProxyModel
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

class MegaApplication;

class InfoDialogTransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoDialogTransfersWidget(QWidget *parent = 0);
    void setupTransfers();
    ~InfoDialogTransfersWidget();

protected:
    void showEvent(QShowEvent *) override;

private:
    Ui::InfoDialogTransfersWidget *ui;
    InfoDialogCurrentTransfersProxyModel *model;
    std::unique_ptr<MegaDelegateHoverManager> mViewHoverManager;

private:
    void configureTransferView();

private:
    MegaApplication *app;
};

#endif // INFODIALOGTRANSFERSWIDGET_H
