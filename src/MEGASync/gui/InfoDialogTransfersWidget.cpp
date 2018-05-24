#include "InfoDialogTransfersWidget.h"
#include "ui_InfoDialogTransfersWidget.h"
#include <QDebug>
using namespace mega;

InfoDialogTransfersWidget::InfoDialogTransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::InfoDialogTransfersWidget)
{
    ui->setupUi(this);
    this->model = NULL;
    tDelegate = NULL;
}

void InfoDialogTransfersWidget::setupTransfers()
{
    model = new QCustomTransfersModel(QTransfersModel::TYPE_CUSTOM_TRANSFERS);
    configureTransferView();
}

QCustomTransfersModel *InfoDialogTransfersWidget::getModel()
{
    return model;
}

InfoDialogTransfersWidget::~InfoDialogTransfersWidget()
{
    delete ui;
    delete model;
    delete tDelegate;
}

void InfoDialogTransfersWidget::configureTransferView()
{
    if (!model)
    {
        return;
    }

    tDelegate = new MegaTransferDelegate(model, this);
    ui->tView->setup(QTransfersModel::TYPE_CUSTOM_TRANSFERS);
    ui->tView->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tView->header()->close();
    ui->tView->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tView->setDragEnabled(false);
    ui->tView->viewport()->setAcceptDrops(false);
    ui->tView->setDropIndicatorShown(false);
    ui->tView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tView->setModel(model);
    ui->tView->setFocusPolicy(Qt::NoFocus);
    ui->tView->disableContextMenus(true);
}
