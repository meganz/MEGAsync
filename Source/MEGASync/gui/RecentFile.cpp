#include "RecentFile.h"
#include "ui_RecentFile.h"
#include "MegaApplication.h"

#include <QImageReader>

RecentFile::RecentFile(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::RecentFile)
{
    ui->setupUi(this);
    ui->lTime->setText(QString::fromAscii(""));
    ui->pArrow->setIcon(QIcon());
}

RecentFile::~RecentFile()
{
    delete ui;
}

void RecentFile::setFile(QString fileName, long long fileHandle, QString localPath, long long time)
{
    this->fileName = fileName;
	this->fileHandle = fileHandle;
	this->localPath = localPath;
    this->dateTime = QDateTime::fromMSecsSinceEpoch(time);
}

void RecentFile::updateWidget()
{
	if(!fileName.length())
	{
		ui->lFileType->setPixmap(QPixmap());
        ui->lTime->setText(QString::fromAscii(""));
        ui->pArrow->setIcon(QIcon());
        return;
	}

	if(fileName.compare(ui->lFileName->text()))
	{
        QFont f = ui->lFileName->font();
        QFontMetrics fm = QFontMetrics(f);
        ui->lFileName->setText(fm.elidedText(fileName, Qt::ElideRight,ui->lFileName->width()));

        QImage image = Utils::createThumbnail(localPath, 120);
        if(!image.isNull())
            ui->lFileType->setPixmap(QPixmap::fromImage(image.scaled(48, 48, Qt::KeepAspectRatioByExpanding, Qt::SmoothTransformation)));
        else
            ui->lFileType->setPixmap(Utils::getExtensionPixmapMedium(fileName));
        ui->pArrow->setIcon(QIcon(QString::fromAscii(":/images/tray_share_ico.png")));
    }

    QDateTime now = QDateTime::currentDateTime();
    qint64 secs = dateTime.secsTo(now);
    if(secs < 2)
        ui->lTime->setText(tr("just now"));
    else if(secs < 60)
        ui->lTime->setText(tr("%1 seconds ago").arg(secs));
    else if(secs < 3600)
    {
        int minutes = secs/60;
        if(minutes == 1)
            ui->lTime->setText(tr("1 minute ago"));
        else
            ui->lTime->setText(tr("%1 minutes ago").arg(minutes));
    }
    else if(secs < 86400)
    {
        int hours = secs/3600;
        if(hours == 1)
            ui->lTime->setText(tr("1 hour ago"));
        else
            ui->lTime->setText(tr("%1 hours ago").arg(hours));
    }
    else if(secs < 292000)
    {
        int days = secs/86400;
        if(days == 1)
            ui->lTime->setText(tr("1 day ago"));
        else
            ui->lTime->setText(tr("%1 days ago").arg(days));
    }
    else if(secs < 31536000)
    {
        int months = secs/292000;
        if(months == 1)
            ui->lTime->setText(tr("1 month ago"));
        else
            ui->lTime->setText(tr("%1 months ago").arg(months));
    }
    else
    {
        int years = secs/31536000;
        if(years == 1)
            ui->lTime->setText(tr("1 year ago"));
        else
            ui->lTime->setText(tr("%1 years ago").arg(years));
    }
}

void RecentFile::on_pArrow_clicked()
{
	((MegaApplication*)qApp)->copyFileLink(fileHandle);
}

void RecentFile::on_lFileType_customContextMenuRequested(const QPoint &pos)
{
	QMenu menu;
	menu.addAction(tr("Open"), this, SLOT(openFile()));
	menu.addAction(tr("Show in folder"), this, SLOT(showInFolder()));
	menu.exec(this->mapToGlobal(pos));
}

void RecentFile::on_wText_customContextMenuRequested(const QPoint &pos)
{
	QMenu menu;
	menu.addAction(tr("Open"), this, SLOT(openFile()));
	menu.addAction(tr("Show in folder"), this, SLOT(showInFolder()));
	menu.exec(this->mapToGlobal(pos));
}

void RecentFile::showInFolder()
{
	WindowsUtils::showInFolder(localPath);
}

void RecentFile::openFile()
{
	QDesktopServices::openUrl(QUrl::fromLocalFile(localPath));
}
