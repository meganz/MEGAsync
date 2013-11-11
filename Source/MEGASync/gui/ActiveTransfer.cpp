#include "ActiveTransfer.h"
#include "ui_ActiveTransfer.h"

ActiveTransfer::ActiveTransfer(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ActiveTransfer)
{
    ui->setupUi(this);
    fileName = "";
    percentage = 0;
}

ActiveTransfer::~ActiveTransfer()
{
    delete ui;
}

void ActiveTransfer::setFileName(QString fileName)
{
    this->fileName = fileName;
    ui->lFileName->setText(fileName);
}

void ActiveTransfer::setPercentage(int percentage)
{
    if(percentage > 100) percentage = 100;
    this->percentage = percentage;
    ui->pProgress->setProgress(percentage);
    ui->lPercentage->setText(QString::number(percentage));
}

int ActiveTransfer::getPercentage()
{
    return percentage;
}

void ActiveTransfer::setType(int type)
{
    this->type = type;
    if(type)
    {
        ui->lType->setPixmap(QPixmap(":/images/BAD_upload.png"));
        ui->lPercentage->setStyleSheet("color: rgb(119, 186, 216);");
    }
    else
    {
        ui->lType->setPixmap(QPixmap(":/images/tray_download_ico.png"));
        ui->lPercentage->setStyleSheet("color: rgb(122, 177, 72);");
    }
}
