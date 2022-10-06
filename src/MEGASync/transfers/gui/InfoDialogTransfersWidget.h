#ifndef INFODIALOGTRANSFERSWIDGET_H
#define INFODIALOGTRANSFERSWIDGET_H

#include <QWidget>
#include <QSortFilterProxyModel>

#include "MegaTransferDelegate.h"
#include "MegaDelegateHoverManager.h"
#include <ViewLoadingScene.h>
#include <InfoDialogTransferLoadingItem.h>

namespace Ui {
class InfoDialogTransfersWidget;
}

class MegaDelegateHoverManager;
class MegaApplication;
class InfoDialogTransfersProxyModel;

class InfoDialogTransfersWidget : public QWidget
{
    Q_OBJECT

public:
    explicit InfoDialogTransfersWidget(QWidget *parent = 0);
    void setupTransfers();
    ~InfoDialogTransfersWidget();

protected:
    void showEvent(QShowEvent *) override;

private slots:
    void onUiBlocked();
    void onUiUnblocked();

private:
    Ui::InfoDialogTransfersWidget *mUi;
    InfoDialogTransfersProxyModel *mProxyModel;
    MegaDelegateHoverManager mViewHoverManager;
    ViewLoadingScene<InfoDialogTransferLoadingItem> mLoadingScene;

private:
    void configureTransferView();
};

#endif // INFODIALOGTRANSFERSWIDGET_H
