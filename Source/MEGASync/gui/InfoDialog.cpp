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

    //Install event filters to show custom tooltips
	ui->bDownloads->installEventFilter(this);
	ui->bUploads->installEventFilter(this);

    //Set window properties
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

    //Initialize fields
    this->app = app;
	downloadSpeed = 0;
	uploadSpeed = 0;
	currentUpload = 0;
	currentDownload = 0;
	totalUploads = 0;
	totalDownloads = 0;
	totalDownloadedSize = totalUploadedSize = 0;
	totalDownloadSize = totalUploadSize = 0;
	remainingUploads = remainingDownloads = 0;
    ui->lDownloads->setText(QString::fromAscii(""));
	ui->bUploads->hide();
    ui->lUploads->setText(QString::fromAscii(""));
	ui->bUploads->hide();
    finishing = false;
    indexing = false;
    transfer1 = NULL;
    transfer2 = NULL;

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);

    //Initialize the "recently updated" list with stored values
    preferences = app->getPreferences();
    if(preferences->getRecentFileHandle(0) != UNDEF)
    {
        ui->wRecent1->setFile(preferences->getRecentFileName(0),
                          preferences->getRecentFileHandle(0),
                          preferences->getRecentLocalPath(0),
                          preferences->getRecentFileTime(0));

        if(preferences->getRecentFileHandle(1) != UNDEF)
        {
            ui->wRecent2->setFile(preferences->getRecentFileName(1),
                                  preferences->getRecentFileHandle(1),
                                  preferences->getRecentLocalPath(1),
                                  preferences->getRecentFileTime(1));

            if(preferences->getRecentFileHandle(2) != UNDEF)
            {
                ui->wRecent3->setFile(preferences->getRecentFileName(2),
                                      preferences->getRecentFileHandle(2),
                                      preferences->getRecentLocalPath(2),
                                      preferences->getRecentFileTime(2));
            }
        }
    }
    updateSyncsButton();

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QPushButton(this);
    overlay->setIcon(QPixmap(QString::fromAscii("://images/tray_paused_large_ico.png")));
    overlay->setIconSize(QSize(64, 64));
    //overlay->setAlignment(Qt::AlignCenter);
    overlay->setStyleSheet(QString::fromAscii("background-color: rgba(255, 255, 255, 200);"
                                              "border: none; "
                                              "margin-left: 4px; "
                                              "margin-right: 4px; "));

    overlay->resize(ui->wTransfers->minimumSize());
    overlay->move(1, 60);
    overlay->hide();
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));
    connect(ui->wTransfer1, SIGNAL(cancel(int, int)), this, SLOT(onTransfer1Cancel(int, int)));
    connect(ui->wTransfer2, SIGNAL(cancel(int, int)), this, SLOT(onTransfer2Cancel(int, int)));
}

InfoDialog::~InfoDialog()
{
    delete ui;
}

void InfoDialog::startAnimation()
{
    if(finishing) return;
    finishing = true;
	QTimer::singleShot(3000, this, SLOT(timerUpdate()));
}

void InfoDialog::setUsage(m_off_t totalBytes, m_off_t usedBytes)
{
	int percentage = (100 * usedBytes) / totalBytes;
	ui->pUsage->setProgress(percentage);
	QString used(QString::number(percentage));
    used += tr("% of ") + Utils::getSizeString(totalBytes);
	ui->lPercentageUsed->setText(used);

    QString usage(tr("Usage: "));
    usage += Utils::getSizeString(usedBytes);
    ui->lTotalUsed->setText(usage);
}

void InfoDialog::setTransfer(MegaTransfer *transfer)
{
    int type = transfer->getType();
    QString fileName = QString::fromUtf8(transfer->getFileName());
    long long completedSize = transfer->getTransferredBytes();
    long long totalSize = transfer->getTotalBytes();

    ActiveTransfer *wTransfer;
    if(type == MegaTransfer::TYPE_DOWNLOAD)
    {
        wTransfer = ui->wTransfer1;
        transfer1 = transfer->getTransfer();
    }
    else
    {
        wTransfer = ui->wTransfer2;
        transfer2 = transfer->getTransfer();
    }

    wTransfer->setFileName(fileName);
    wTransfer->setProgress(completedSize, totalSize, app->getMegaApi()->isRegularTransfer(transfer->getTransfer()));
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
}

void InfoDialog::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    QLayoutItem *item = ui->recentLayout->itemAt(2);
    RecentFile * recentFile = ((RecentFile *)item->widget());
    ui->recentLayout->insertWidget(0, recentFile);
    recentFile->setFile(fileName, fileHandle, localPath, QDateTime::currentDateTime().toMSecsSinceEpoch());
    app->getPreferences()->addRecentFile(fileName, fileHandle, localPath);
}

void InfoDialog::updateTransfers()
{
    remainingUploads = app->getMegaApi()->getNumPendingUploads();
    remainingDownloads = app->getMegaApi()->getNumPendingDownloads();
    totalUploads = app->getMegaApi()->getTotalUploads();
    totalDownloads = app->getMegaApi()->getTotalDownloads();

    currentDownload = totalDownloads - remainingDownloads + 1;
    currentUpload = totalUploads - remainingUploads + 1;

    if(remainingDownloads)
    {
        QString pattern(tr("%1 of %2"));
        QString downloadString = pattern.arg(currentDownload).arg(totalDownloads);

        if(downloadSpeed > 0) downloadString += QString::fromAscii(" (") + Utils::getSizeString(downloadSpeed) + QString::fromAscii("/s)");
        else downloadString += tr(" (paused)");

        ui->lDownloads->setText(downloadString);
        ui->bDownloads->show();
    }
    else
    {
        ui->wTransfer1->hideTransfer();
        ui->lDownloads->setText(QString::fromAscii(""));
        ui->bDownloads->hide();
    }

    if(remainingUploads)
    {
        QString pattern(tr("%1 of %2"));
        QString uploadString = pattern.arg(currentUpload).arg(totalUploads);

        if(uploadSpeed > 0) uploadString += QString::fromAscii(" (") + Utils::getSizeString(uploadSpeed) + QString::fromAscii("/s)");
        else uploadString += tr(" (paused)");

        ui->lUploads->setText(uploadString);
        ui->bUploads->show();
    }
    else
    {
        ui->wTransfer2->hideTransfer();
        ui->lUploads->setText(QString::fromAscii(""));
        ui->bUploads->hide();
    }
    if(ui->bDownloads->underMouse())
        showPopup(ui->bDownloads->mapToGlobal(QPoint(-130, -102)),true);
    else if(ui->bUploads->underMouse())
        showPopup(ui->bUploads->mapToGlobal(QPoint(-130, -102)), false);

    if((ui->sActiveTransfers->currentWidget() != ui->pUpdated) && !remainingDownloads && !remainingUploads)
        this->startAnimation();
    else finishing = false;
}

void InfoDialog::updateSyncsButton()
{
    int num = preferences->getNumSyncedFolders();
    if(num == 1 && preferences->getMegaFolderHandle(0)==app->getMegaApi()->getRootNode()->nodehandle)
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
    else
        ui->bSyncFolder->setText(tr("Syncs"));
}

void InfoDialog::setIndexing(bool indexing)
{
    this->indexing = indexing;
    if(ui->bPause->isChecked()) ui->lSyncUpdated->setText(tr("File transfers paused"));
    else if(!indexing) ui->lSyncUpdated->setText(tr("MEGAsync is up to date"));
    else ui->lSyncUpdated->setText(tr("MEGAsync is scanning"));
}

void InfoDialog::setTransferSpeeds(long long downloadSpeed, long long uploadSpeed)
{
	this->downloadSpeed = downloadSpeed;
	this->uploadSpeed = uploadSpeed;
    updateTransfers();
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

void InfoDialog::setPaused(bool paused)
{
    ui->bPause->setChecked(paused);
    ui->bPause->setEnabled(true);
    overlay->setVisible(paused);
    if(ui->bPause->isChecked())
    {
        ui->lSyncUpdated->setText(tr("File transfers paused"));
        setTransferSpeeds(-1, -1);
    }
    else
    {
        if(!indexing) ui->lSyncUpdated->setText(tr("MEGAsync is up to date"));
        else ui->lSyncUpdated->setText(tr("MEGAsync is scanning"));
    }
}

void InfoDialog::updateDialog()
{
    updateRecentFiles();
}

void InfoDialog::timerUpdate()
{
    remainingDownloads = app->getMegaApi()->getNumPendingDownloads();
    remainingUploads = app->getMegaApi()->getNumPendingUploads();

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
        app->getMegaApi()->resetTransferCounters();
        updateTransfers();
		app->showSyncedIcon();
		app->getMegaApi()->getAccountDetails();
        app->showNotificationMessage(tr("All transfers have been finished"));
    }
}

void InfoDialog::addSync()
{
    BindFolderDialog *dialog = new BindFolderDialog(app, this);
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

    QString localFolderPath = QDir(dialog->getLocalFolder()).canonicalPath();
    MegaApi *megaApi = app->getMegaApi();
    long long handle = dialog->getMegaFolder();
    Node *node = megaApi->getNodeByHandle(handle);
    QString syncName = dialog->getSyncName();
    if(!localFolderPath.length() || !node)
        return;

   Preferences *preferences = app->getPreferences();
   preferences->addSyncedFolder(localFolderPath, QString::fromUtf8(megaApi->getNodePath(node)), handle, syncName);
   app->getMegaApi()->syncFolder(localFolderPath.toUtf8().constData(), node);
   updateSyncsButton();
}

void InfoDialog::onTransfer1Cancel(int x, int y)
{
    QMenu menu;
    QAction *cancelAll = menu.addAction(tr("Cancel all downloads"), this, SLOT(cancelAllDownloads()));
    QAction *cancelCurrent = menu.addAction(tr("Cancel download"), this, SLOT(cancelCurrentDownload()));
    menu.addAction(cancelCurrent);
    menu.addAction(cancelAll);
    menu.exec(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
}

void InfoDialog::onTransfer2Cancel(int x, int y)
{
    QMenu menu;
    QAction *cancelAll = menu.addAction(tr("Cancel all uploads"), this, SLOT(cancelAllUploads()));
    QAction *cancelCurrent = menu.addAction(tr("Cancel upload"), this, SLOT(cancelCurrentUpload()));
    menu.addAction(cancelCurrent);
    menu.addAction(cancelAll);
    menu.exec(ui->wTransfer2->mapToGlobal(QPoint(x, y)));
}

void InfoDialog::cancelAllUploads()
{
    app->getMegaApi()->cancelRegularTransfers(1);
}

void InfoDialog::cancelAllDownloads()
{
    app->getMegaApi()->cancelRegularTransfers(0);
}

void InfoDialog::cancelCurrentUpload()
{
    app->getMegaApi()->cancelTransfer(transfer2);
}

void InfoDialog::cancelCurrentDownload()
{
    app->getMegaApi()->cancelTransfer(transfer1);
}


void InfoDialog::on_bSettings_clicked()
{
	app->openSettings();
	this->hide();
}

void InfoDialog::on_bOfficialWeb_clicked()
{
    QString helpUrl = QString::fromAscii("https://mega.co.nz/");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void InfoDialog::on_bSyncFolder_clicked()
{
    Preferences *preferences = app->getPreferences();
    int num = preferences->getNumSyncedFolders();

    if(num == 1 && preferences->getMegaFolderHandle(0)==app->getMegaApi()->getRootNode()->nodehandle)
    {
        openFolder(preferences->getLocalFolder(0));
    }
    else
    {
        QMenu menu;
        menu.setStyleSheet(QString::fromAscii("QMenu { background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;}"));
        QAction *addSyncAction = menu.addAction(tr("Add Sync"), this, SLOT(addSync()));
        addSyncAction->setIcon(QIcon(QString::fromAscii("://images/tray_add_sync_ico.png")));
        menu.addSeparator();

        QSignalMapper signalMapper;
        for(int i=0; i<num; i++)
        {
            QAction *action = menu.addAction(preferences->getSyncName(i), &signalMapper, SLOT(map()));
            action->setIcon(QIcon(QString::fromAscii("://images/tray_sync_ico.png")));
            signalMapper.setMapping(action, preferences->getLocalFolder(i));
            connect(&signalMapper, SIGNAL(mapped(QString)), this, SLOT(openFolder(QString)));
        }
        menu.exec(ui->bSyncFolder->mapToGlobal(QPoint(0, -num*30)));
    }
}

void InfoDialog::openFolder(QString path)
{
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

	QString operation;
    QString oneFile = tr("one file at %1/s");
    QString oneFilePaused = tr("one file (paused)");
	QString xOfxFilesPattern(tr("%1 of %2 files at %3/s"));
    QString xOfxFilesPausedPattern(tr("%1 of %2 files (paused)"));
	QString totalRemaining(tr("Total Remaining: "));
	QString remainingSize;
	QString xOfxFiles;
	long long totalRemainingSeconds;

	if(download)
	{
		if(!totalDownloads) return;
		operation = tr("Downloading ");
		long long remainingBytes = totalDownloadSize-totalDownloadedSize;
        remainingSize = Utils::getSizeString(remainingBytes);
        if(totalDownloads == 1)
        {
            if(downloadSpeed>0)
                xOfxFiles = oneFile.arg(Utils::getSizeString(downloadSpeed));
            else
                xOfxFiles = oneFilePaused;
        }
        else
        {
            if(downloadSpeed>0)
                xOfxFiles = xOfxFilesPattern.arg(currentDownload).arg(totalDownloads).arg(Utils::getSizeString(downloadSpeed));
            else
                xOfxFiles = xOfxFilesPausedPattern.arg(currentDownload).arg(totalDownloads);
        }
        totalRemainingSeconds = (downloadSpeed>0) ? remainingBytes/downloadSpeed : 0;
	}
	else
	{
		if(!totalUploads) return;

		operation = tr("Uploading ");
		long long remainingBytes = totalUploadSize-totalUploadedSize;
        remainingSize = Utils::getSizeString(totalUploadSize-totalUploadedSize);

        if(totalUploads == 1)
        {
            if(uploadSpeed>0)
                xOfxFiles = oneFile.arg(Utils::getSizeString(uploadSpeed));
            else
                xOfxFiles = oneFilePaused;
        }
        else
        {
            if(uploadSpeed>0)
                xOfxFiles = xOfxFilesPattern.arg(currentUpload).arg(totalUploads).arg(Utils::getSizeString(uploadSpeed));
            else
                xOfxFiles = xOfxFilesPausedPattern.arg(currentUpload).arg(totalUploads);
        }

        totalRemainingSeconds = (uploadSpeed>0) ? remainingBytes/uploadSpeed : 0;
	}

	int remainingHours = totalRemainingSeconds/3600;
	int remainingMinutes = (totalRemainingSeconds%3600)/60;
	int remainingSeconds =  (totalRemainingSeconds%60);
    QString remainingTime;
    if(totalRemainingSeconds)
    {
        remainingTime = QString::fromAscii("%1:%2:%3").arg(remainingHours, 2, 10, QChar::fromAscii('0'))
            .arg(remainingMinutes, 2, 10, QChar::fromAscii('0'))
            .arg(remainingSeconds, 2, 10, QChar::fromAscii('0'));
    }
    else
    {
        remainingTime = QString::fromAscii("--:--:--");
    }

	QString popupHtml(
QString::fromUtf8("<table border=\"0\" height=\"75\" width=\"280\" cellspacing=\"12\">"
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
        "</table>"));
	QString popupText = popupHtml.arg(operation).arg(xOfxFiles).arg(totalRemaining).arg(remainingSize).arg(remainingTime);

	QToolTip::showText(globalpos,popupText);
}

void InfoDialog::updateRecentFiles()
{
	ui->wRecent1->updateWidget();
	ui->wRecent2->updateWidget();
	ui->wRecent3->updateWidget();
}

void InfoDialog::on_bPause_clicked()
{
    app->pauseTransfers(ui->bPause->isChecked());
}

void InfoDialog::onOverlayClicked()
{
    ui->bPause->setChecked(false);
    on_bPause_clicked();
}
