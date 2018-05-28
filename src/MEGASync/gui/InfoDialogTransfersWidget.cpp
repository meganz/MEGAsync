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
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(enableAlternateRowStyle()));
    connect(model, SIGNAL(noTransfers()), this, SLOT(disableAlternateRowStyle()));

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
    ui->tView->setSelectionMode(QAbstractItemView::NoSelection);
    ui->tView->setDragEnabled(false);
    ui->tView->viewport()->setAcceptDrops(false);
    ui->tView->setDropIndicatorShown(false);
    ui->tView->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tView->setModel(model);
    ui->tView->setFocusPolicy(Qt::NoFocus);
    ui->tView->disableContextMenus(true);
}

void InfoDialogTransfersWidget::enableAlternateRowStyle()
{
    ui->tView->setAlternatingRowColors(true);
    ui->tView->setStyleSheet(QString::fromUtf8("QTreeView {background: #fafafa; alternate-background-color: white;}"));
}

void InfoDialogTransfersWidget::disableAlternateRowStyle()
{
    ui->tView->setAlternatingRowColors(false);
    ui->tView->setStyleSheet(QString::fromUtf8("QTreeView {background: white;}"));
}
