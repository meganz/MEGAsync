#include "RecentFile.h"
#include "ui_RecentFile.h"

RecentFile::RecentFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);
    ui->lTime->setText(QString());
    ui->pArrow->hide();
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFileName(QString fileName)
{
    this->fileName = fileName;
    ui->lFileName->setText(fileName);
    if(!fileName.length())
    {
        ui->lFileType->setPixmap(QPixmap());
        ui->lTime->setText(QString());
        ui->pArrow->hide();
        return;
    }

    QFileInfo f(fileName);
    if(WindowsUtils::extensionIcons.contains(f.suffix().toLower()))
        ui->lFileType->setPixmap(WindowsUtils::extensionIcons[f.suffix().toLower()]);
    else
        ui->lFileType->setPixmap(QPixmap(":/images/sync_generic.png"));

    dateTime = QDateTime::currentDateTime();
    ui->lTime->setText(tr("just now"));
    ui->pArrow->show();
}

void RecentFile::updateTime()
{
    if(!fileName.size())
        return;

    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = dateTime.secsTo(now);
    if(secs < 2)
        ui->lTime->setText(tr("just now"));
    else if(secs < 60)
        ui->lTime->setText(tr("%1 seconds ago").arg(secs));
    else if(secs < 3600)
        ui->lTime->setText(tr("%1 minutes ago").arg(secs/60));
    else if(secs < 86400)
        ui->lTime->setText(tr("%1 hours ago").arg(secs/3600));
    else if(secs < 292000)
        ui->lTime->setText(tr("%1 days ago").arg(secs/86400));
    else if(secs < 31536000)
        ui->lTime->setText(tr("%1 months ago").arg(secs/292000));
    else
        ui->lTime->setText(tr("%1 years ago").arg(secs/31536000));
}
