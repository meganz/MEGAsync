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
    transfersActive = false;
}

void TransfersWidget::setupTransfers(MegaTransferData *transferData, int type)
{
    this->type = type;
    model = new QActiveTransfersModel(type, transferData);
    connect(model, SIGNAL(noTransfers()), this, SLOT(noTransfers()));
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(onTransferAdded()));

    noTransfers();
    configureTransferView();

    if ((type == MegaTransfer::TYPE_DOWNLOAD && transferData->getNumDownloads())
            || (type == MegaTransfer::TYPE_UPLOAD && transferData->getNumUploads()))
    {
        onTransferAdded();
    }
}

void TransfersWidget::setupFinishedTransfers(QList<MegaTransfer* > transferData)
{
    this->type = QTransfersModel::TYPE_FINISHED ;
    model = new QFinishedTransfersModel(transferData);
    connect(model, SIGNAL(noTransfers()), this, SLOT(noTransfers()));
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(onTransferAdded()));

    noTransfers();
    configureTransferView();

    if (transferData.size())
    {
        onTransferAdded();
    }
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
}

bool TransfersWidget::areTransfersActive()
{
    return transfersActive;
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
    ui->tvTransfers->setSelectionMode(QAbstractItemView::ContiguousSelection);
    ui->tvTransfers->setDragEnabled(true);
    ui->tvTransfers->viewport()->setAcceptDrops(true);
    ui->tvTransfers->setDropIndicatorShown(true);
    ui->tvTransfers->setDragDropMode(QAbstractItemView::InternalMove);
    ui->tvTransfers->setModel(model);

    switch (type)
    {
        case QTransfersModel::TYPE_DOWNLOAD:
            ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_DOWNLOADS);
            break;
        case QTransfersModel::TYPE_UPLOAD:
            ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_UPLOADS);
            break;
        default:
            ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_TRANSFERS);
            break;
    }
}

void TransfersWidget::pausedTransfers(bool paused)
{
    isPaused = paused;
    if (model->rowCount(QModelIndex()) == 0)
    {
        transfersActive = false;
        noTransfers();
    }
    else
    {
        transfersActive = true;
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
    transfersActive = false;
    if (isPaused)
    {
        ui->pNoTransfers->setState(TransfersStateInfoWidget::PAUSED);
    }
    else
    {
        switch (type)
        {
            case QTransfersModel::TYPE_DOWNLOAD:
                ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_DOWNLOADS);
                break;
            case QTransfersModel::TYPE_UPLOAD:
                ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_UPLOADS);
                break;
            default:
                ui->pNoTransfers->setState(TransfersStateInfoWidget::NO_TRANSFERS);
                break;
        }
    }

    ui->sWidget->setCurrentWidget(ui->pNoTransfers);
}

void TransfersWidget::onTransferAdded()
{
    transfersActive = true;
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
