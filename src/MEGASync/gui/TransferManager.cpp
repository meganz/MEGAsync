#include "TransferManager.h"
#include "ui_TransferManager.h"

using namespace mega;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferManager)
{
    ui->setupUi(this);
    this->megaApi = megaApi;

    ui->wUploads->setupTransfers(megaApi->getTransfers(QTransfersModel::TYPE_UPLOAD), QTransfersModel::TYPE_UPLOAD);
    ui->wDownloads->setupTransfers(megaApi->getTransfers(QTransfersModel::TYPE_DOWNLOAD), QTransfersModel::TYPE_DOWNLOAD);
    ui->wAllTransfers->setupTransfers(megaApi->getTransfers(QTransfersModel::TYPE_ALL), QTransfersModel::TYPE_ALL);

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-bottom: 2px solid #31b500; }"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-bottom: 2px solid #ff333a; }"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-bottom: 2px solid #2ba6de; }"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("QToolButton:checked { border-bottom: 2px solid #31b500; }"));


    on_tAllTransfers_clicked();
}

TransferManager::~TransferManager()
{
    delete ui;
}

void TransferManager::on_tCompleted_clicked()
{
    if (ui->wTransfers->currentWidget() == ui->wCompleted)
    {
        ui->tCompleted->setChecked(true);
        return;
    }
    ui->tCompleted->setChecked(true);
    ui->tDownloads->setChecked(false);
    ui->tUploads->setChecked(false);
    ui->tAllTransfers->setChecked(false);
    ui->wTransfers->setCurrentWidget(ui->wCompleted);
}

void TransferManager::on_tDownloads_clicked()
{
    if (ui->wTransfers->currentWidget() == ui->wDownloads)
    {
        ui->tDownloads->setChecked(true);
        return;
    }
    ui->tDownloads->setChecked(true);
    ui->tCompleted->setChecked(false);
    ui->tUploads->setChecked(false);
    ui->tAllTransfers->setChecked(false);
    ui->wTransfers->setCurrentWidget(ui->wDownloads);
}

void TransferManager::on_tUploads_clicked()
{
    if (ui->wTransfers->currentWidget() == ui->wUploads)
    {
        ui->tUploads->setChecked(true);
        return;
    }
    ui->tUploads->setChecked(true);
    ui->tCompleted->setChecked(false);
    ui->tDownloads->setChecked(false);
    ui->tAllTransfers->setChecked(false);
    ui->wTransfers->setCurrentWidget(ui->wUploads);
}

void TransferManager::on_tAllTransfers_clicked()
{
    if (ui->wTransfers->currentWidget() == ui->wAllTransfers)
    {
        ui->tAllTransfers->setChecked(true);
        return;
    }
    ui->tAllTransfers->setChecked(true);
    ui->tCompleted->setChecked(false);
    ui->tDownloads->setChecked(false);
    ui->tUploads->setChecked(false);
    ui->wTransfers->setCurrentWidget(ui->wAllTransfers);
}
