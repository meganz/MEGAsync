#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include <QTimer>

using namespace mega;

TransfersWidget::TransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersWidget)
{
    ui->setupUi(this);
    this->model = NULL;
    isPaused = false;

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QToolButton(this);
    overlay->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
    overlay->setIcon(QIcon(QString::fromAscii("://images/paused_transfers.png")));
    overlay->setIconSize(QSize(156, 156));
    overlay->setText(tr("Paused Transfers"));
    overlay->setStyleSheet(QString::fromAscii("background-color: rgba(247, 247, 247, 200); "
                                              "padding: 50px 0 20px 0;"
                                              "font-family: \"Source Sans Pro\";"
                                              "font-size: 24px;"
                                              "font-weight: 300;"
                                              "color: rgba(51, 51, 51, 0.7);"
                                              "border: none; "));
    overlay->resize(ui->pTransfers->minimumSize());
    overlay->hide();
}

void TransfersWidget::setupTransfers(MegaTransferData *transferData, int type)
{
    this->type = type;
    model = new QTransfersModel(type);
    connect(model, SIGNAL(noTransfers()), this, SLOT(noTransfers()));
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(onTransferAdded()));
    model->setupModelTransfers(transferData);

    configureTransferView();
}

void TransfersWidget::setupTransfers(QList<MegaTransfer* > transferData, int type)
{
    this->type = type;
    model = new QTransfersModel(type);
    connect(model, SIGNAL(noTransfers()), this, SLOT(noTransfers()));
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(onTransferAdded()));

    configureTransferView();
    model->setupModelTransfers(transferData);
}

void TransfersWidget::refreshTransferItems()
{
    model->refreshTransfers();
}

void TransfersWidget::clearTransfers()
{
    model->removeAllTransfers();
}

TransfersWidget::~TransfersWidget()
{
    delete ui;
    delete tDelegate;
    delete model;
    delete overlay;
}

void TransfersWidget::configureTransferView()
{
    if (!model)
    {
        return;
    }

    tDelegate = new MegaTransferDelegate(model, this);
    ui->tvTransfers->setup(type);
    ui->tvTransfers->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tvTransfers->header()->close();
    ui->tvTransfers->setSelectionMode(QAbstractItemView::SingleSelection);
    ui->tvTransfers->setDragEnabled(true);
    ui->tvTransfers->viewport()->setAcceptDrops(true);
    ui->tvTransfers->setDropIndicatorShown(true);
    ui->tvTransfers->setDragDropMode(QAbstractItemView::InternalMove);

    noTransfers();
    ui->tvTransfers->setModel(model);
}

void TransfersWidget::pausedTransfers(bool paused)
{
    isPaused = paused;
    overlay->setVisible(paused);
    if (model->rowCount(QModelIndex()) == 0)
    {
        noTransfers();
    }
    else
    {
        ui->sWidget->setCurrentWidget(ui->pTransfers);
    }
}

void TransfersWidget::disableGetLink(bool disable)
{
    ui->tvTransfers->disableGetLink(disable);
}

QTransfersModel *TransfersWidget::getModel()
{
    return model;
}

void TransfersWidget::noTransfers()
{
    if (isPaused)
    {
        ui->sWidget->setCurrentWidget(ui->pTransfers);
        return;
    }

    ui->sWidget->setCurrentWidget(ui->pNoTransfers);
    switch (type)
    {
        case QTransfersModel::TYPE_DOWNLOAD:
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/no_downloads.png")));
            ui->lStatusIcon->setIconSize(QSize(156, 156));
            ui->lStatus->setText(tr("No Downloads"));
            break;
        case QTransfersModel::TYPE_UPLOAD:
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/no_uploads.png")));
            ui->lStatusIcon->setIconSize(QSize(156, 156));
            ui->lStatus->setText(tr("No Uploads"));
            break;
        default:
            ui->lStatusIcon->setIcon(QIcon(QString::fromAscii("://images/no_transfers.png")));
            ui->lStatusIcon->setIconSize(QSize(156, 156));
            ui->lStatus->setText(tr("No Transfers"));
            break;
    }
}

void TransfersWidget::onTransferAdded()
{
    ui->sWidget->setCurrentWidget(ui->pTransfers);
}

void TransfersWidget::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QWidget::changeEvent(event);
}
