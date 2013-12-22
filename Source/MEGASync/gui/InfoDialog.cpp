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
	ui->lDownloads->setText("");
	ui->bUploads->hide();
	ui->lUploads->setText("");
	ui->bUploads->hide();

	/***************************/
    //Example transfers
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

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);
    ui->bPause->hide();

    //Initialize the "recently updated" list with stored values
    preferences = app->getPreferences();
    if(preferences->getRecentFileHandle(0) != UNDEF)
    {
        ui->wRecent1->setFile(preferences->getRecentFileName(0),
                          preferences->getRecentFileHandle(0),
                          preferences->getRecentLocalPath(0));

        if(preferences->getRecentFileHandle(1) != UNDEF)
        {
            ui->wRecent2->setFile(preferences->getRecentFileName(1),
                                  preferences->getRecentFileHandle(1),
                                  preferences->getRecentLocalPath(1));

            if(preferences->getRecentFileHandle(2) != UNDEF)
            {
                ui->wRecent3->setFile(preferences->getRecentFileName(2),
                                      preferences->getRecentFileHandle(2),
                                      preferences->getRecentLocalPath(2));
            }
        }
    }

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QPushButton(ui->wTransfers);
    overlay->setIcon(QPixmap("://images/tray_paused_large_ico.png"));
    overlay->setIconSize(QSize(64, 64));
    //overlay->setAlignment(Qt::AlignCenter);
    overlay->setStyleSheet("background-color: rgba(255, 255, 255, 200); border: none;");
    overlay->resize(ui->wTransfers->minimumSize());
    overlay->hide();
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));
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
    used += "% of " + Utils::getSizeString(totalBytes);
	ui->lPercentageUsed->setText(used);

    QString usage("Usage: ");
    usage += Utils::getSizeString(usedBytes);
    ui->lTotalUsed->setText(usage);
}

void InfoDialog::setTransfer(int type, QString fileName, long long completedSize, long long totalSize)
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

void InfoDialog::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    QLayoutItem *item = ui->recentLayout->itemAt(2);
    RecentFile * recentFile = ((RecentFile *)item->widget());
    ui->recentLayout->insertWidget(0, recentFile);
	recentFile->setFile(fileName, fileHandle, localPath);
    app->getPreferences()->addRecentFile(fileName, fileHandle, localPath);
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
        if(downloadSpeed)
        {
            if(downloadSpeed > 0) downloadString += " (" + Utils::getSizeString(downloadSpeed) + "/s)";
            else downloadString += tr(" (paused)");
        }
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
        if(uploadSpeed)
        {
            if(uploadSpeed > 0) uploadString += " (" + Utils::getSizeString(uploadSpeed) + "/s)";
            else uploadString += tr(" (paused)");
        }
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
    {
        ui->bPause->setChecked(false);
        ui->bPause->hide();
        this->startAnimation();
    }
    else ui->bPause->show();
}

void InfoDialog::setTransferSpeeds(long long downloadSpeed, long long uploadSpeed)
{
	this->downloadSpeed = downloadSpeed;
	this->uploadSpeed = uploadSpeed;

	if(remainingDownloads)
	{
		QString pattern(tr("%1 of %2"));
		QString downloadString = pattern.arg(currentDownload).arg(totalDownloads);
        if(downloadSpeed)
        {
            if(downloadSpeed > 0) downloadString += " (" + Utils::getSizeString(downloadSpeed) + "/s)";
            else downloadString += tr(" (paused)");
        }
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
        if(uploadSpeed)
        {
            if(uploadSpeed > 0) uploadString += " (" + Utils::getSizeString(uploadSpeed) + "/s)";
            else uploadString += tr(" (paused)");
        }
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

void InfoDialog::setPaused(bool paused)
{
    ui->bPause->setEnabled(true);
    ui->bPause->setChecked(paused);
    overlay->setVisible(paused);
    if(paused) setTransferSpeeds(-1, -1);
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
        app->showNotificationMessage(tr("All transfers have been finished"));
    }
}

void InfoDialog::addSync()
{
    BindFolderDialog *dialog = new BindFolderDialog(this);
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

   QString localFolderPath = dialog->getLocalFolder();
   if(!Utils::verifySyncedFolderLimits(localFolderPath))
   {
       QMessageBox::warning(this, tr("Warning"), tr("Too many files or folders (+%1 folders or +%2 files).\n"
            "Please, select another folder.").arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
            .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER), QMessageBox::Ok);
       return;
   }

   MegaApi *megaApi = app->getMegaApi();
   long long handle = dialog->getMegaFolder();
   Node *node = megaApi->getNodeByHandle(handle);
   if(!localFolderPath.length() || !node)
       return;

   bool repeated;
   QString syncName = QFileInfo(localFolderPath).fileName();
   do {
       repeated = false;
       for(int i=0; i<preferences->getNumSyncedFolders(); i++)
       {
           if(!syncName.compare(preferences->getSyncName(i)))
           {
                repeated = true;

                bool ok;
                QString text = QInputDialog::getText(this, tr("Sync name"),
                     tr("The name \"%1\" is already in use for another sync.\n"
                     "Please, enter another name to identify this synced folder:").arg(syncName),
                     QLineEdit::Normal, syncName, &ok).trimmed();
                if (!ok && text.isEmpty())
                    return;

                syncName = text;
           }
       }
   }while(repeated);

   Preferences *preferences = app->getPreferences();
   preferences->addSyncedFolder(localFolderPath, megaApi->getNodePath(node), handle, syncName);
   app->reloadSyncs();
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
    Preferences *preferences = app->getPreferences();
    int num = preferences->getNumSyncedFolders();

    QMenu menu;

    menu.setStyleSheet("QMenu { background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;}");
    QAction *addSyncAction = menu.addAction("Add Sync", this, SLOT(addSync()));
    addSyncAction->setIcon(QIcon("://images/tray_add_sync_ico.png"));
    menu.addSeparator();

    QSignalMapper signalMapper;
    for(int i=0; i<num; i++)
    {
        QAction *action = menu.addAction(preferences->getSyncName(i), &signalMapper, SLOT(map()));
        action->setIcon(QIcon("://images/small_folder.png"));
        signalMapper.setMapping(action, preferences->getLocalFolder(i));
        connect(&signalMapper, SIGNAL(mapped(QString)), this, SLOT(openFolder(QString)));
    }
    menu.exec(ui->bSyncFolder->mapToGlobal(QPoint(0, -num*30)));

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
        if(downloadSpeed>=0)
            xOfxFiles = xOfxFilesPattern.arg(currentDownload).arg(totalDownloads).arg(Utils::getSizeString(downloadSpeed));
        else
            xOfxFiles = xOfxFilesPausedPattern.arg(currentDownload).arg(totalDownloads);

        totalRemainingSeconds = (downloadSpeed>0) ? remainingBytes/downloadSpeed : 0;
	}
	else
	{
		if(!totalUploads) return;

		operation = tr("Uploading ");
		long long remainingBytes = totalUploadSize-totalUploadedSize;
        remainingSize = Utils::getSizeString(totalUploadSize-totalUploadedSize);
        if(uploadSpeed>=0)
            xOfxFiles = xOfxFilesPattern.arg(currentUpload).arg(totalUploads).arg(Utils::getSizeString(uploadSpeed));
        else
            xOfxFiles = xOfxFilesPausedPattern.arg(currentUpload).arg(totalUploads);

        totalRemainingSeconds = (uploadSpeed>0) ? remainingBytes/uploadSpeed : 0;
	}

	int remainingHours = totalRemainingSeconds/3600;
	int remainingMinutes = (totalRemainingSeconds%3600)/60;
	int remainingSeconds =  (totalRemainingSeconds%60);
    QString remainingTime;
    if(totalRemainingSeconds)
    {
        remainingTime = QString("%1:%2:%3").arg(remainingHours, 2, 10, QChar('0'))
			.arg(remainingMinutes, 2, 10, QChar('0'))
			.arg(remainingSeconds, 2, 10, QChar('0'));
    }
    else
    {
        remainingTime = QString("--:--:--");
    }

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

void InfoDialog::on_bPause_clicked()
{
    ui->bPause->setEnabled(false);
    app->pauseTransfers(ui->bPause->isChecked());
}

void InfoDialog::onOverlayClicked()
{
    ui->bPause->setChecked(false);
    on_bPause_clicked();
}
