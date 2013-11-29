#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>

#include "InfoDialog.h"
#include "ActiveTransfer.h"
#include "RecentFile.h"
#include "ui_InfoDialog.h"

#include "MegaApplication.h"

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    this->app = app;

    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    this->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 500 - 2);

	/***************************/
	/*ui->wRecent1->setFileName("filename_compressed.zip");
    ui->wRecent2->setFileName("filename_document.pdf");
    ui->wRecent3->setFileName("filename_image.png");

    ui->wTransfer1->setFileName("Photoshop_file_2.psd");
    ui->wTransfer1->setPercentage(76);
    ui->wTransfer1->setType(0);
    ui->wTransfer2->setFileName("Illustrator_file.ai");
    ui->wTransfer2->setPercentage(50);
	ui->wTransfer2->setType(1);*/
	/******************************/
	ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::startAnimation()
{
	QTimer::singleShot(3000, this, SLOT(timerUpdate()));
}

void InfoDialog::setUsage(int totalGB, int percentage)
{
    double usedGB = totalGB * percentage / 100.0;
    ui->pUsage->setProgress(percentage);
    QString used(QString::number(percentage));
    used += "% of " + QString::number(totalGB) + " GB";
    ui->lPercentageUsed->setText(used);

    QString usage("Usage: ");
    usage += QString::number(usedGB) + " GB";
    ui->lTotalUsed->setText(usage);
}

void InfoDialog::setTransfer(int type, QString &fileName, long long completedSize, long long totalSize)
{
    ActiveTransfer *transfer;
    if(type == MegaTransfer::TYPE_DOWNLOAD)
       transfer = ui->wTransfer1;
    else
       transfer = ui->wTransfer2;

    transfer->setFileName(fileName);
    int percentage = 100*(double)completedSize/totalSize;
    transfer->setPercentage(percentage);

    ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
}

void InfoDialog::addRecentFile(QString &fileName, long long fileHandle)
{
    QLayoutItem *item = ui->recentLayout->itemAt(2);
    RecentFile * recentFile = ((RecentFile *)item->widget());
    ui->recentLayout->insertWidget(0, recentFile);
	recentFile->setFile(fileName, fileHandle);
}

void InfoDialog::setQueuedTransfers(int queuedDownloads, int queuedUploads)
{
	cout << "TD: " << queuedDownloads << "   TU: " << queuedUploads << endl;
	int activeDownloads=0;
	int activeUploads=0;
	if(queuedDownloads)
	{
		queuedDownloads--;
		activeDownloads++;
	}
	if(queuedUploads)
	{
		queuedUploads--;
		activeUploads++;
	}

    ui->lQueued->setText(tr("%1 Queued").arg(QString::number(queuedUploads+queuedDownloads)));

	ui->bDownloads->setText(QString::number(activeDownloads));
	ui->bUploads->setText(QString::number(activeUploads));

	if(!activeDownloads && !activeUploads)
        this->startAnimation();
}

void InfoDialog::updateDialog()
{
    updateRecentFiles();
}

void InfoDialog::timerUpdate()
{
	if(ui->wTransfer1->getPercentage()==100)
	{
		ui->wTransfer1->hideTransfer();
	}

	if(ui->wTransfer2->getPercentage()==100)
	{
		ui->wTransfer2->hideTransfer();
	}

	if((ui->wTransfer1->getPercentage()==100) && ui->wTransfer2->getPercentage()==100)
	{
		ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
		app->showSyncedIcon();
	}
}

void InfoDialog::on_bSettings_clicked()
{
	app->openSettings();
	this->hide();
}

void InfoDialog::on_bOfficialWeb_clicked()
{
    QString helpUrl("https://mega.co.nz/");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void InfoDialog::on_bSyncFolder_clicked()
{
    QString filePath = app->getPreferences()->getLocalFolder(0);
    QStringList args;
    args << QDir::toNativeSeparators(filePath);
	QProcess::startDetached("explorer", args);
}

void InfoDialog::updateRecentFiles()
{
	ui->wRecent1->updateWidget();
	ui->wRecent2->updateWidget();
	ui->wRecent3->updateWidget();
}
