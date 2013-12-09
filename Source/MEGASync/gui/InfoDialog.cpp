#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>
#include <QHelpEvent>
#include <QToolTip>
#include <QSignalMapper>

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
	ui->bDownloads->installEventFilter(this);
	ui->bUploads->installEventFilter(this);

    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    this->app = app;

    QRect screenGeometry = QApplication::desktop()->availableGeometry();
    this->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 500 - 2);

	downloadSpeed = 0;
	uploadSpeed = 0;
	currentUpload = 0;
	currentDownload = 0;
	totalUploads = 0;
	totalDownloads = 0;
	totalDownloadedSize = totalUploadedSize = 0;
	totalDownloadSize = totalUploadSize = 0;
	remainingUploads = remainingDownloads = 0;
	ui->lDownloads->setText("");
	ui->bUploads->hide();
	ui->lUploads->setText("");
	ui->bUploads->hide();

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
	/*ui->bDownloads->setToolTip("<body bgcolor=\"black\"><font color=\"green\">Downloads</font>"
							   "<font color=\"white\"> 1 of 20 files at 908 KB/s</font><br />"
							   "<font color=\"red\">Total Remaining:</font>"
							   "<font color=\"white\"> 512.34 MB</font></body>");*/
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::startAnimation()
{
	QTimer::singleShot(3000, this, SLOT(timerUpdate()));
}

void InfoDialog::setUsage(m_off_t totalBytes, m_off_t usedBytes)
{
	int percentage = (100 * usedBytes) / totalBytes;
	ui->pUsage->setProgress(percentage);
	QString used(QString::number(percentage));
	used += "% of " + WindowsUtils::getSizeString(totalBytes);
	ui->lPercentageUsed->setText(used);

    QString usage("Usage: ");
	usage += WindowsUtils::getSizeString(usedBytes);
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
	transfer->setProgress(completedSize, totalSize);

    ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
}

void InfoDialog::addRecentFile(QString &fileName, long long fileHandle, QString localPath)
{
    QLayoutItem *item = ui->recentLayout->itemAt(2);
    RecentFile * recentFile = ((RecentFile *)item->widget());
    ui->recentLayout->insertWidget(0, recentFile);
	recentFile->setFile(fileName, fileHandle, localPath);
}

void InfoDialog::setTransferCount(int totalDownloads, int totalUploads, int remainingDownloads, int remainingUploads)
{
	cout << "TD: " << totalDownloads << "   TU: " << totalUploads << endl;
	//ui->lQueued->setText(tr("%1 Queued").arg(QString::number(queuedUploads+queuedDownloads)));
	//ui->bDownloads->setText(QString::number(totalDownloads));
	//ui->bUploads->setText(QString::number(totalUploads));
	this->totalDownloads = totalDownloads;
	this->totalUploads = totalUploads;
	this->remainingDownloads = remainingDownloads;
	this->remainingUploads = remainingUploads;
	currentDownload = totalDownloads - remainingDownloads + 1;
	currentUpload = totalUploads - remainingUploads + 1;

	if(remainingDownloads)
	{
		QString pattern(tr("%1 of %2"));
		QString downloadString = pattern.arg(currentDownload).arg(totalDownloads);
		if(downloadSpeed) downloadString += " (" + WindowsUtils::getSizeString(downloadSpeed) + "/s)";
		ui->lDownloads->setText(downloadString);
		ui->bDownloads->show();
	}
	else
	{
		ui->lDownloads->setText("");
		ui->bDownloads->hide();
	}

	if(remainingUploads)
	{
		QString pattern(tr("%1 of %2"));
		QString uploadString = pattern.arg(currentUpload).arg(totalUploads);
		if(uploadSpeed) uploadString += " (" + WindowsUtils::getSizeString(uploadSpeed) + "/s)";
		ui->lUploads->setText(uploadString);
		ui->bUploads->show();
	}
	else
	{
		ui->lUploads->setText("");
		ui->bUploads->hide();
	}
	if(ui->bDownloads->underMouse())
		showPopup(ui->bDownloads->mapToGlobal(QPoint(-130, -102)),true);
	else if(ui->bUploads->underMouse())
		showPopup(ui->bUploads->mapToGlobal(QPoint(-130, -102)), false);

	if(!remainingDownloads && !remainingUploads)
		this->startAnimation();
}

void InfoDialog::setTransferSpeeds(long long downloadSpeed, long long uploadSpeed)
{
	this->downloadSpeed = downloadSpeed;
	this->uploadSpeed = uploadSpeed;

	if(remainingDownloads)
	{
		QString pattern(tr("%1 of %2"));
		QString downloadString = pattern.arg(currentDownload).arg(totalDownloads);
		if(downloadSpeed) downloadString += " (" + WindowsUtils::getSizeString(downloadSpeed) + "/s)";
		ui->lDownloads->setText(downloadString);
		ui->bDownloads->show();
	}
	else
	{
		ui->lDownloads->setText("");
		ui->bDownloads->hide();
	}

	if(remainingUploads)
	{
		QString pattern(tr("%1 of %2"));
		QString uploadString = pattern.arg(currentUpload).arg(totalUploads);
		if(uploadSpeed) uploadString += " (" + WindowsUtils::getSizeString(uploadSpeed) + "/s)";
		ui->lUploads->setText(uploadString);
		ui->bUploads->show();
	}
	else
	{
		ui->lUploads->setText("");
		ui->bUploads->hide();
	}

	if(ui->bDownloads->underMouse())
		showPopup(ui->bDownloads->mapToGlobal(QPoint(-130, -102)), true);
	else if(ui->bUploads->underMouse())
		showPopup(ui->bUploads->mapToGlobal(QPoint(-130, -102)), false);
}

void InfoDialog::setTransferredSize(long long totalDownloadedSize, long long totalUploadedSize)
{
	this->totalDownloadedSize = totalDownloadedSize;
	this->totalUploadedSize = totalUploadedSize;
	if(ui->bDownloads->underMouse())
		showPopup(ui->bDownloads->mapToGlobal(QPoint(-130, -102)), true);
	else if(ui->bUploads->underMouse())
		showPopup(ui->bUploads->mapToGlobal(QPoint(-130, -102)), false);
}

void InfoDialog::setTotalTransferSize(long long totalDownloadSize, long long totalUploadSize)
{
	this->totalDownloadSize = totalDownloadSize;
	this->totalUploadSize = totalUploadSize;
	if(ui->bDownloads->underMouse())
		showPopup(ui->bDownloads->mapToGlobal(QPoint(-130, -102)), true);
	else if(ui->bUploads->underMouse())
		showPopup(ui->bUploads->mapToGlobal(QPoint(-130, -102)), false);
}

void InfoDialog::updateDialog()
{
    updateRecentFiles();
}

void InfoDialog::timerUpdate()
{
	if(!remainingDownloads) ui->wTransfer1->hideTransfer();
	if(!remainingUploads) ui->wTransfer2->hideTransfer();

	if(!remainingDownloads && !remainingUploads)
	{
		downloadSpeed = 0;
		uploadSpeed = 0;
		currentUpload = 0;
		currentDownload = 0;
		totalUploads = 0;
		totalDownloads = 0;
		totalDownloadedSize = totalUploadedSize = 0;
		totalDownloadSize = totalUploadSize = 0;
		ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
		app->showSyncedIcon();
		app->getMegaApi()->getAccountDetails();
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
	Preferences *prefences = app->getPreferences();
	int num = prefences->getNumSyncedFolders();
	/*if(num==0)
	{
		QString filePath = app->getPreferences()->getLocalFolder(0);
		QStringList args;
		args << QDir::toNativeSeparators(filePath);
		QProcess::startDetached("explorer", args);
	}
	else*/
	{
		QMenu menu;
		QSignalMapper signalMapper;

		for(int i=0; i<num; i++)
		{
			QFileInfo info(prefences->getLocalFolder(i));

			QAction *action = menu.addAction(info.fileName(), &signalMapper, SLOT(map()));
			action->setIcon(QIcon("://images/folder.ico"));
			signalMapper.setMapping(action, info.absoluteFilePath());
			connect(&signalMapper, SIGNAL(mapped(QString)), this, SLOT(openFolder(QString)));
		}
		menu.exec(ui->bSyncFolder->mapToGlobal(QPoint(0, -(num-1)*30)));
	}
}

void InfoDialog::openFolder(QString path)
{
	cout << path.toStdString() << endl;
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

bool InfoDialog::eventFilter(QObject *obj, QEvent *event)
{
	if (event->type() != QEvent::ToolTip) return false;

	bool download;
	QPoint globalpos;
	if(obj == ui->bDownloads)
	{
		globalpos = ui->bDownloads->mapToGlobal(QPoint(-130, -102));
		download = true;
	}
	else
	{
		globalpos = ui->bUploads->mapToGlobal(QPoint(-130, -102));
		download = false;
	}

	showPopup(globalpos, download);
	return true;
}

void InfoDialog::showPopup(QPoint globalpos, bool download)
{
	if(lastPopupUpdate.isNull()) lastPopupUpdate = QDateTime::currentDateTime();

	QDateTime now = QDateTime::currentDateTime();
	if(lastPopupUpdate.secsTo(now)<1) return;
	lastPopupUpdate = now;

	cout << "SHOW POPUP " << lastPopupUpdate.secsTo(now) << endl;

	QString operation;
	QString xOfxFilesPattern(tr("%1 of %2 files at %3/s"));
	QString totalRemaining(tr("Total Remaining: "));
	QString remainingSize;
	QString xOfxFiles;
	long long totalRemainingSeconds;

	if(download)
	{
		if(!totalDownloads) return;
		operation = tr("Downloading ");
		long long remainingBytes = totalDownloadSize-totalDownloadedSize;
		remainingSize = WindowsUtils::getSizeString(remainingBytes);
		xOfxFiles = xOfxFilesPattern.arg(currentDownload).arg(totalDownloads).arg(WindowsUtils::getSizeString(downloadSpeed));
		totalRemainingSeconds = downloadSpeed ? remainingBytes/downloadSpeed : 0;
	}
	else
	{
		if(!totalUploads) return;

		operation = tr("Uploading ");
		long long remainingBytes = totalUploadSize-totalUploadedSize;
		remainingSize = WindowsUtils::getSizeString(totalUploadSize-totalUploadedSize);
		xOfxFiles = xOfxFilesPattern.arg(currentUpload).arg(totalUploads).arg(WindowsUtils::getSizeString(uploadSpeed));
		totalRemainingSeconds = uploadSpeed ? remainingBytes/uploadSpeed : 0;
	}

	int remainingHours = totalRemainingSeconds/3600;
	int remainingMinutes = (totalRemainingSeconds%3600)/60;
	int remainingSeconds =  (totalRemainingSeconds%60);
	QString remainingTime = QString("%1:%2:%3").arg(remainingHours, 2, 10, QChar('0'))
			.arg(remainingMinutes, 2, 10, QChar('0'))
			.arg(remainingSeconds, 2, 10, QChar('0'));

	QString popupHtml(
		"<table border=\"0\" height=\"75\" width=\"280\" cellspacing=\"12\">"
			"<tr width=\"280\">"
				"<td colspan=\"2\"><font color=\"#78B240\">%1</font>"
				"<font color=\"white\">%2</font></td>"
			"</tr>"

			"<tr height=\"50\" style=\"vertical-align:middle;\">"
				"<td width=\"160\"><font color=\"#FF2823\">%3</font>"
					"<font color=\"white\">%4</font>"
				"</td>"
				"<td width=\"70\" style=\"vertical-align:middle; align=\"right\">"
					"<table>"
						"<tr>"
							"<td>"
								"<img src=\"://images/tray_tooltip_clock_ico.png\"></img>"
							"</td>"
							"<td>"
								"<font color=\"white\">%5</font>"
							"</td>"
						"</tr>"
					"</table>"
				"</td>"
			"</tr>"
		"</table>");
	QString popupText = popupHtml.arg(operation).arg(xOfxFiles).arg(totalRemaining).arg(remainingSize).arg(remainingTime);

	QToolTip::showText(globalpos,popupText);
}

void InfoDialog::updateRecentFiles()
{
	ui->wRecent1->updateWidget();
	ui->wRecent2->updateWidget();
	ui->wRecent3->updateWidget();
}
