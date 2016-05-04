#include "TransferManager.h"
#include "ui_TransferManager.h"

TransferManager::TransferManager(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferManager)
{
    ui->setupUi(this);
}

TransferManager::~TransferManager()
{
    delete ui;
}
