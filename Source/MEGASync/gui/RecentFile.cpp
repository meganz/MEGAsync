#include "RecentFile.h"
#include "ui_RecentFile.h"

RecentFile::RecentFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFileName(QString fileName)
{
    this->fileName = fileName;
    ui->lFileName->setText(fileName);
    if(fileName.endsWith(".pdf"))
        ui->lFileType->setPixmap(QPixmap(":/images/sync_pdf.png"));
    else if(fileName.endsWith(".zip"))
        ui->lFileType->setPixmap(QPixmap(":/images/sync_compressed.png"));
    else if(fileName.endsWith(".png"))
        ui->lFileType->setPixmap(QPixmap(":/images/sync_image.png"));
    ui->lTime->setText(tr("%1 days ago"));
    ui->lTime->setText(tr("%1 months ago"));
}
