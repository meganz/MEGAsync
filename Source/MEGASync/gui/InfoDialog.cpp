#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>
#include <QHelpEvent>
#include <QToolTip>
#include <QSignalMapper>
#include <QVBoxLayout>
#include "InfoDialog.h"
#include "ActiveTransfer.h"
#include "RecentFile.h"
#include "ui_InfoDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);

    //Set window properties
    setWindowFlags(Qt::FramelessWindowHint | Qt::Tool);

    //Initialize fields
    this->app = app;
	downloadSpeed = 0;
	uploadSpeed = 0;
	currentUpload = 0;
	currentDownload = 0;
	totalUploads = 0;
	totalDownloads = 0;
    totalBytes = usedBytes = 0;
	totalDownloadedSize = totalUploadedSize = 0;
	totalDownloadSize = totalUploadSize = 0;
	remainingUploads = remainingDownloads = 0;
    uploadStartTime = 0;
    downloadStartTime = 0;
    effectiveDownloadSpeed = 200000;
    effectiveUploadSpeed = 200000;
    ui->lDownloads->setText(QString::fromAscii(""));
    ui->lUploads->setText(QString::fromAscii(""));
    indexing = false;
    waiting = false;
    transfer1 = NULL;
    transfer2 = NULL;

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);

    megaApi = app->getMegaApi();

    //Initialize the "recently updated" list with stored values
    preferences = Preferences::instance();
    if(preferences->getRecentFileHandle(0) != mega::UNDEF)
    {
        ui->wRecent1->setFile(preferences->getRecentFileName(0),
                          preferences->getRecentFileHandle(0),
                          preferences->getRecentLocalPath(0),
                          preferences->getRecentFileTime(0));

        if(preferences->getRecentFileHandle(1) != mega::UNDEF)
        {
            ui->wRecent2->setFile(preferences->getRecentFileName(1),
                                  preferences->getRecentFileHandle(1),
                                  preferences->getRecentLocalPath(1),
                                  preferences->getRecentFileTime(1));

            if(preferences->getRecentFileHandle(2) != mega::UNDEF)
            {
                ui->wRecent3->setFile(preferences->getRecentFileName(2),
                                      preferences->getRecentFileHandle(2),
                                      preferences->getRecentLocalPath(2),
                                      preferences->getRecentFileTime(2));
            }
        }
    }
    updateRecentFiles();
    updateSyncsButton();

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QPushButton(this);
    overlay->setIcon(QPixmap(QString::fromAscii("://images/tray_paused_large_ico.png")));
    overlay->setIconSize(QSize(64, 64));
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

void InfoDialog::setUsage(m_off_t totalBytes, m_off_t usedBytes)
{
    if(!totalBytes) return;

    this->totalBytes = totalBytes;
    this->usedBytes = usedBytes;
    int percentage = (100 * usedBytes) / totalBytes;
	ui->pUsage->setProgress(percentage);
    QString used = tr("%1 of %2").arg(QString::number(percentage).append(QString::fromAscii("%")))
            .arg(Utilities::getSizeString(totalBytes));
	ui->lPercentageUsed->setText(used);
    ui->lTotalUsed->setText(tr("Usage: %1").arg(Utilities::getSizeString(usedBytes)));
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
        transfer1 = transfer->copy();
        if(!downloadStartTime)
        {
            downloadStartTime = QDateTime::currentMSecsSinceEpoch();
            elapsedDownloadTime=0;
            lastUpdate = QDateTime::currentMSecsSinceEpoch();
        }
    }
    else
    {
        wTransfer = ui->wTransfer2;
        transfer2 = transfer->copy();
        if(!uploadStartTime)
        {
            uploadStartTime = QDateTime::currentMSecsSinceEpoch();
            elapsedUploadTime=0;
            lastUpdate = QDateTime::currentMSecsSinceEpoch();
        }
    }

    wTransfer->setFileName(fileName);
    wTransfer->setProgress(completedSize, totalSize, megaApi->isRegularTransfer(transfer));
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
}

void InfoDialog::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    QVBoxLayout *recentLayout = (QVBoxLayout *)ui->wRecentLayout->layout();
    QLayoutItem *item = recentLayout->itemAt(2);
    RecentFile * recentFile = ((RecentFile *)item->widget());
    recentLayout->insertWidget(0, recentFile);
    recentFile->setFile(fileName, fileHandle, localPath, QDateTime::currentDateTime().toMSecsSinceEpoch());
    preferences->addRecentFile(fileName, fileHandle, localPath);
}

void InfoDialog::updateTransfers()
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();
    totalUploads = megaApi->getTotalUploads();
    totalDownloads = megaApi->getTotalDownloads();
    currentDownload = totalDownloads - remainingDownloads + 1;
    currentUpload = totalUploads - remainingUploads + 1;

    if(remainingDownloads)
    {
        long long remainingBytes = totalDownloadSize-totalDownloadedSize;
        long long timeIncrement = QDateTime::currentMSecsSinceEpoch()-lastUpdate;
        if(timeIncrement < 1000) elapsedDownloadTime += timeIncrement;
        double effectiveSpeed = totalDownloadedSize/(elapsedDownloadTime/1000+1);
        effectiveDownloadSpeed += (effectiveSpeed-effectiveDownloadSpeed)/10; //Smooth the effective speed

        if(isVisible())
        {
            int totalRemainingSeconds = (effectiveDownloadSpeed>0) ? remainingBytes/effectiveDownloadSpeed : 0;
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

            ui->lRemainingTimeD->setText(remainingTime);
            ui->wDownloadDesc->show();
            QString fullPattern = QString::fromAscii("<span style=\"color: rgb(120, 178, 66); \">%1</span>%2");
            QString operation = tr("Downloading ");
            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2 (paused)"));
            QString downloadString;

            if(downloadSpeed >= 0)  downloadString = pattern.arg(currentDownload).arg(totalDownloads).arg(Utilities::getSizeString(downloadSpeed));
            else downloadString += pausedPattern.arg(currentDownload).arg(totalDownloads);

            ui->lDownloads->setText(fullPattern.arg(operation).arg(downloadString));
        }
    }
    else
    {
        ui->wTransfer1->hideTransfer();
        ui->lDownloads->setText(QString::fromAscii(""));
        ui->wDownloadDesc->hide();
        downloadStartTime = 0;
        downloadSpeed = 0;
        currentDownload = 0;
        totalDownloads = 0;
        totalDownloadedSize = 0;
        totalDownloadSize = 0;
        megaApi->resetTotalDownloads();
    }

    if(remainingUploads)
    {
        long long remainingBytes = totalUploadSize-totalUploadedSize;
        long long timeIncrement = QDateTime::currentMSecsSinceEpoch()-lastUpdate;
        if(timeIncrement < 1000) elapsedUploadTime += timeIncrement;
        double effectiveSpeed = totalUploadedSize/(elapsedUploadTime/1000+1);
        effectiveUploadSpeed += (effectiveSpeed-effectiveUploadSpeed)/10; //Smooth the effective speed

        if(isVisible())
        {
            int totalRemainingSeconds = (effectiveUploadSpeed>0) ? remainingBytes/effectiveUploadSpeed : 0;
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

            ui->lRemainingTimeU->setText(remainingTime);
            ui->wUploadDesc->show();
            QString fullPattern = QString::fromAscii("<span style=\"color: rgb(119, 185, 217); \">%1</span>%2");
            QString operation = tr("Uploading ");
            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2 (paused)"));
            QString uploadString;

            if(uploadSpeed >= 0) uploadString = pattern.arg(currentUpload).arg(totalUploads).arg(Utilities::getSizeString(uploadSpeed));
            else uploadString += pausedPattern.arg(currentUpload).arg(totalUploads);

            ui->lUploads->setText(fullPattern.arg(operation).arg(uploadString));
        }
    }
    else
    {
        ui->wTransfer2->hideTransfer();
        ui->lUploads->setText(QString::fromAscii(""));
        ui->wUploadDesc->hide();
        uploadStartTime = 0;
        uploadSpeed = 0;
        currentUpload = 0;
        totalUploads = 0;
        totalUploadedSize = 0;
        totalUploadSize = 0;
        megaApi->resetTotalUploads();
    }

    lastUpdate = QDateTime::currentMSecsSinceEpoch();
    if(!remainingDownloads && !remainingUploads &&  (ui->sActiveTransfers->currentWidget() != ui->pUpdated))
    {
        ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
        app->updateUserStats();
        app->showNotificationMessage(tr("All transfers have been completed"));
    }
}

void InfoDialog::updateSyncsButton()
{
    int num = preferences->getNumSyncedFolders();
    MegaNode *rootNode = megaApi->getRootNode();
    if(!rootNode)
    {
        LOG("rootNode is NULL. I'm about to crash :-(");
    }
    long long rootHandle = rootNode->getHandle();
    long long firstSyncHandle = 0;
    if(num == 1)
        firstSyncHandle = preferences->getMegaFolderHandle(0);

    if((num == 1) && (firstSyncHandle==rootHandle))
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
    else
        ui->bSyncFolder->setText(tr("Syncs"));

    delete rootNode;
}

void InfoDialog::setIndexing(bool indexing)
{
    this->indexing = indexing;
}

void InfoDialog::setWaiting(bool waiting)
{
    this->waiting = waiting;
}

void InfoDialog::increaseUsedStorage(long long bytes)
{
    this->usedBytes+=bytes;
    this->setUsage(totalBytes, usedBytes);
}

void InfoDialog::updateState()
{
    if(ui->bPause->isChecked())
    {
        setTransferSpeeds(-1, -1);
        ui->lSyncUpdated->setText(tr("File transfers paused"));
    }
    else if(indexing) ui->lSyncUpdated->setText(tr("MEGAsync is scanning"));
    else if(waiting) ui->lSyncUpdated->setText(tr("MEGAsync is waiting"));
    else ui->lSyncUpdated->setText(tr("MEGAsync is up to date"));
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
}

void InfoDialog::setTotalTransferSize(long long totalDownloadSize, long long totalUploadSize)
{
	this->totalDownloadSize = totalDownloadSize;
	this->totalUploadSize = totalUploadSize;
}

void InfoDialog::setPaused(bool paused)
{
    ui->bPause->setChecked(paused);
    ui->bPause->setEnabled(true);
    overlay->setVisible(paused);
}

void InfoDialog::updateDialog()
{
    updateRecentFiles();
}

void InfoDialog::addSync()
{
    BindFolderDialog *dialog = new BindFolderDialog(app);
    this->hide();
    int result = dialog->exec();
    if(result != QDialog::Accepted)
        return;

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    long long handle = dialog->getMegaFolder();
    MegaNode *node = megaApi->getNodeByHandle(handle);
    QString syncName = dialog->getSyncName();
    delete dialog;
    if(!localFolderPath.length() || !node)
    {
        delete node;
        return;
    }

   const char *nPath = megaApi->getNodePath(node);
   if(!nPath)
   {
       delete node;
       return;
   }

   preferences->addSyncedFolder(localFolderPath, QString::fromUtf8(nPath), handle, syncName);
   delete nPath;
   megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
   delete node;
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
    megaApi->cancelTransfers(1);
}

void InfoDialog::cancelAllDownloads()
{
    megaApi->cancelTransfers(0);
}

void InfoDialog::cancelCurrentUpload()
{
    megaApi->cancelTransfer(transfer2);
}

void InfoDialog::cancelCurrentDownload()
{
    megaApi->cancelTransfer(transfer1);
}


void InfoDialog::on_bSettings_clicked()
{
    this->hide();
	app->openSettings();
}

void InfoDialog::on_bOfficialWeb_clicked()
{
    this->hide();
    QString helpUrl = QString::fromAscii("https://mega.co.nz/");
    QDesktopServices::openUrl(QUrl(helpUrl));
}

void InfoDialog::on_bSyncFolder_clicked()
{
    int num = preferences->getNumSyncedFolders();

    MegaNode *rootNode = megaApi->getRootNode();
    if(num == 1 && preferences->getMegaFolderHandle(0)==rootNode->getHandle())
    {
        openFolder(preferences->getLocalFolder(0));
    }
    else
    {
        QMenu menu;
        menu.setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
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
    delete rootNode;
}

void InfoDialog::openFolder(QString path)
{
    this->hide();
	QDesktopServices::openUrl(QUrl::fromLocalFile(path));
}

void InfoDialog::updateRecentFiles()
{
	ui->wRecent1->updateWidget();
	ui->wRecent2->updateWidget();
    ui->wRecent3->updateWidget();
}

void InfoDialog::focusOutEvent(QFocusEvent *event)
{
    QPoint p = mapFromGlobal(QCursor::pos());
    if(p.x()<0 || p.y()<0 || p.x()>this->width() || p.y()>this->height())
        this->hide();
    else
        this->setFocus();
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

void InfoDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if(totalBytes) setUsage(totalBytes, usedBytes);
        updateSyncsButton();
        updateTransfers();
    }
    QDialog::changeEvent(event);
}
