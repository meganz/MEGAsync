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
    mModel(nullptr)
{
    mUi->setupUi(this);
}

void InfoDialogTransfersWidget::setupTransfers()
{
    mModel = new InfoDialogTransfersProxyModel(mUi->tView);
    mModel->setSourceModel(MegaSyncApp->getTransfersModel());
    mModel->sort(0);
    mModel->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete mUi;
    delete mModel;
}

void InfoDialogTransfersWidget::showEvent(QShowEvent*)
{
    if(mModel)
    {
       mModel->invalidate();
    }
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!mModel)
    {
        return;
    }

    auto tDelegate = new MegaTransferDelegate(mModel, mUi->tView);
    mUi->tView->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    mUi->tView->header()->close();
    mUi->tView->setSelectionMode(QAbstractItemView::NoSelection);
    mUi->tView->setDragEnabled(false);
    mUi->tView->viewport()->setAcceptDrops(false);
    mUi->tView->setDropIndicatorShown(false);
    mUi->tView->setDragDropMode(QAbstractItemView::InternalMove);
    mUi->tView->setModel(mModel);
    mUi->tView->setFocusPolicy(Qt::NoFocus);
    mUi->tView->disableContextMenus(true);

    mViewHoverManager.setView(mUi->tView);
}
