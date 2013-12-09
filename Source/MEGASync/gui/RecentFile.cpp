#include "RecentFile.h"
#include "ui_RecentFile.h"
#include "MegaApplication.h"

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

void RecentFile::setFile(QString fileName, long long fileHandle, QString localPath)
{
    this->fileName = fileName;
	this->fileHandle = fileHandle;
	this->localPath = localPath;
	this->dateTime = QDateTime::currentDateTime();
}

void RecentFile::updateWidget()
{
	if(!fileName.length())
	{
		ui->lFileType->setPixmap(QPixmap());
		ui->lTime->setText(QString());
		ui->pArrow->hide();
		return;
	}

	if(fileName.compare(ui->lFileName->text()))
	{
		ui->lFileName->setText(fileName);
		QFileInfo f(fileName);
		if(WindowsUtils::extensionIcons.contains(f.suffix().toLower()))
			ui->lFileType->setPixmap(QPixmap(QString(WindowsUtils::extensionIcons[f.suffix().toLower()]).insert(0, "://images/drag_")));
		else
			ui->lFileType->setPixmap(QPixmap("://images/drag_generic.png"));
		ui->pArrow->show();
	}

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
