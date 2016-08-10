#include "TransfersWidget.h"
#include "ui_TransfersWidget.h"
#include <QTextEdit>
#include <QTimer>

TransfersWidget::TransfersWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::TransfersWidget)
{
    ui->setupUi(this);
    this->model = NULL;

    tDelegate = new MegaTransferDelegate(this);
    ui->tvTransfers->setItemDelegate((QAbstractItemDelegate *)tDelegate);
    ui->tvTransfers->header()->close();

}

void TransfersWidget::setupTransfers(mega::MegaTransferList *tList, int type)
{
    this->type = type;

    noTransfers(type);
    model = new QTransfersModel(type);
    connect(model, SIGNAL(noTransfers(int)), this, SLOT(noTransfers(int)));
    connect(model, SIGNAL(onTransferAdded()), this, SLOT(onTransferAdded()));
    model->setupModelTransfers(tList);
    ui->tvTransfers->setModel(model);
}

TransfersWidget::~TransfersWidget()
{
    delete ui;
    delete tDelegate;
    delete model;
}

void TransfersWidget::noTransfers(int type)
{
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
