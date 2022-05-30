#include "InfoDialogTransfersWidget.h"
#include "ui_InfoDialogTransfersWidget.h"
#include "MegaApplication.h"
#include "MegaTransferDelegate.h"
#include "model/InfoDialogTransfersProxyModel.h"

using namespace mega;

//WIDGET

InfoDialogTransfersWidget::InfoDialogTransfersWidget(QWidget *parent) :
    QWidget(parent),
    mUi(new Ui::InfoDialogTransfersWidget),
    mProxyModel(nullptr)
{
    mUi->setupUi(this);
}

void InfoDialogTransfersWidget::setupTransfers()
{
    mProxyModel = new InfoDialogTransfersProxyModel(mUi->tView);
    mProxyModel->setSourceModel(MegaSyncApp->getTransfersModel());
    mProxyModel->sort(0);
    mProxyModel->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete mUi;
    delete mProxyModel;
}

void InfoDialogTransfersWidget::showEvent(QShowEvent*)
{
    if(mProxyModel)
    {    
        qDebug() << "SHOW AGAIN";
        mProxyModel->invalidate();
    }
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!mProxyModel)
    {
        return;
    }

    auto tDelegate = new MegaTransferDelegate(mProxyModel, mUi->tView);
    mUi->tView->setItemDelegate(tDelegate);
    mUi->tView->header()->close();
    mUi->tView->setSelectionMode(QAbstractItemView::NoSelection);
    mUi->tView->setDragEnabled(false);
    mUi->tView->viewport()->setAcceptDrops(false);
    mUi->tView->setDropIndicatorShown(false);
    mUi->tView->setDragDropMode(QAbstractItemView::InternalMove);
    mUi->tView->setModel(mProxyModel);
    mUi->tView->setFocusPolicy(Qt::NoFocus);
    mUi->tView->disableContextMenus(true);

    mViewHoverManager.setView(mUi->tView);
}
