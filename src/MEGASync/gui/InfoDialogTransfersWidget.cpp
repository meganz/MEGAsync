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
    this->model = NULL;
    app = (MegaApplication *)qApp;
}

void InfoDialogTransfersWidget::setupTransfers()
{
    model = new InfoDialogTransfersProxyModel(ui->tView);
    model->setSourceModel(app->getTransfersModel());
    model->sort(0);
    model->setDynamicSortFilter(true);

    configureTransferView();
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete ui;
    delete model;
}

void InfoDialogTransfersWidget::showEvent(QShowEvent*)
{
    if(model)
    {
       model->invalidate();
    }
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!model)
    {
        return;
    }

    auto tDelegate = new MegaTransferDelegate(model, ui->tView);
    ui->tView->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tView->header()->close();
    ui->tView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tView->setDragEnabled(false);
    ui->tView->viewport()->setAcceptDrops(false);
    ui->tView->setDropIndicatorShown(false);
    ui->tView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tView->setModel(model);
    ui->tView->setFocusPolicy(Qt::NoFocus);
    ui->tView->disableContextMenus(true);

    mViewHoverManager.setView(ui->tView);
}
