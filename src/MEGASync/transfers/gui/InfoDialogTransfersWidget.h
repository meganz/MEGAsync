#ifndef INFODIALOGTRANSFERSWIDGET_H
#define INFODIALOGTRANSFERSWIDGET_H

#include "InfoDialogTransferLoadingItem.h"
#include "MegaDelegateHoverManager.h"
#include "TransferItem.h"
#include "ViewLoadingScene.h"

#include <QSortFilterProxyModel>
#include <QWidget>

namespace Ui {
class InfoDialogTransfersWidget;
}

class MegaDelegateHoverManager;
class MegaApplication;
class InfoDialogTransfersProxyModel;

class InfoDialogTransfersView : public LoadingSceneView<InfoDialogTransferLoadingItem, QTreeView>
{
public:
    InfoDialogTransfersView(QWidget* parent)
        : LoadingSceneView<InfoDialogTransferLoadingItem, QTreeView>(parent)
    {}
};

class InfoDialogTransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoDialogTransfersWidget(QWidget *parent = 0);
    void setupTransfers();
    ~InfoDialogTransfersWidget();

    std::optional<TransferData::TransferTypes> getTopTransferType();

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void onUiBlocked();
    void onUiUnblocked();
    void onProxyModelModified();

private:
    Ui::InfoDialogTransfersWidget *mUi;
    InfoDialogTransfersProxyModel *mProxyModel;
    MegaDelegateHoverManager mViewHoverManager;

private:
    void configureTransferView();

signals:
    void topTransferTypeChanged(TransferData::TransferTypes);
};

#endif // INFODIALOGTRANSFERSWIDGET_H
