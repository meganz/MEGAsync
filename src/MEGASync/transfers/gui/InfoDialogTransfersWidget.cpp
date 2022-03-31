#include "InfoDialogTransfersWidget.h"
#include "ui_InfoDialogTransfersWidget.h"
#include "MegaApplication.h"
#include "MegaTransferDelegate.h"
#include "model/InfoDialogTransfersProxyModel.h"

using namespace mega;

//WIDGET

InfoDialogTransfersWidget::InfoDialogTransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoDialogTransfersWidget)
{
    ui->setupUi(this);
    this->mModel = NULL;
    app = (MegaApplication *)qApp;
}

void InfoDialogTransfersWidget::setupTransfers()
{
    mModel = new InfoDialogTransfersProxyModel(ui->tView);
    mModel->setSourceModel(app->getTransfersModel());
    mModel->sort(0);
    mModel->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete ui;
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

    auto tDelegate = new MegaTransferDelegate(mModel, ui->tView);
    ui->tView->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tView->header()->close();
    ui->tView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tView->setDragEnabled(false);
    ui->tView->viewport()->setAcceptDrops(false);
    ui->tView->setDropIndicatorShown(false);
    ui->tView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tView->setModel(mModel);
    ui->tView->setFocusPolicy(Qt::NoFocus);
    ui->tView->disableContextMenus(true);

    mViewHoverManager.setView(ui->tView);
}
