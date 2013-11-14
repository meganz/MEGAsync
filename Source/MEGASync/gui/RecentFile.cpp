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
    QFileInfo f(fileName);
    if(WindowsUtils::extensionIcons.contains(f.suffix().toLower()))
        ui->lFileType->setPixmap(WindowsUtils::extensionIcons[f.suffix().toLower()]);
    else
        ui->lFileType->setPixmap(QPixmap(":/images/sync_generic.png"));

    ui->lTime->setText(tr("%1 days ago"));
    ui->lTime->setText(tr("%1 months ago"));
}
