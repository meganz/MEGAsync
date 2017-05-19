#include <QDesktopServices>
#include <QDesktopWidget>
#include <QUrl>
#include <QRect>
#include <QTimer>
#include <QHelpEvent>
#include <QToolTip>
#include <QSignalMapper>
#include <QVBoxLayout>
#include <QFileInfo>
#include <QEvent>
#include "InfoDialog.h"
#include "ActiveTransfer.h"
#include "RecentFile.h"
#include "ui_InfoDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"
#include "MenuItemAction.h"

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    //Set window properties
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);

#ifdef __APPLE__
    setAttribute(Qt::WA_TranslucentBackground);
#endif

    //Initialize fields
    this->app = app;
    downloadSpeed = 0;
    uploadSpeed = 0;
    currentUpload = 0;
    currentDownload = 0;
    totalUploads = 0;
    totalDownloads = 0;
    activeDownloadState = activeUploadState = MegaTransfer::STATE_NONE;
    remainingDownloadBytes = remainingUploadBytes = 0;
    meanDownloadSpeed = meanUploadSpeed = 0;
    remainingUploads = remainingDownloads = 0;
    ui->lDownloads->setText(QString::fromAscii(""));
    ui->lUploads->setText(QString::fromAscii(""));
    indexing = false;
    waiting = false;
    syncsMenu = NULL;
    activeDownload = NULL;
    activeUpload = NULL;
    transferMenu = NULL;
    storageUsedMenu = NULL;
    cloudItem = NULL;
    inboxItem = NULL;
    sharesItem = NULL;
    rubbishItem = NULL;
    gWidget = NULL;
    overQuotaState = false;

    //Initialize header dialog and disable chat features
    ui->wHeader->setStyleSheet(QString::fromUtf8("#wHeader {border: none;}"));

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
    ui->wTransfer1->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransfer1->hideTransfer();
    ui->wTransfer2->setType(MegaTransfer::TYPE_UPLOAD);
    ui->wTransfer2->hideTransfer();

    ui->bTransferManager->setToolTip(tr("Open Transfer Manager"));
    ui->bSettings->setToolTip(tr("Access your MEGAsync settings"));

    ui->pUsageStorage->installEventFilter(this);

    state = STATE_STARTING;
    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    scanningTimer.setSingleShot(false);
    scanningTimer.setInterval(60);
    scanningAnimationIndex = 1;
    connect(&scanningTimer, SIGNAL(timeout()), this, SLOT(scanningAnimationStep()));

    uploadsFinishedTimer.setSingleShot(true);
    uploadsFinishedTimer.setInterval(5000);
    connect(&uploadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllUploadsFinished()));

    downloadsFinishedTimer.setSingleShot(true);
    downloadsFinishedTimer.setInterval(5000);
    connect(&downloadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllDownloadsFinished()));

    transfersFinishedTimer.setSingleShot(true);
    transfersFinishedTimer.setInterval(5000);
    connect(&transfersFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllTransfersFinished()));

    ui->wDownloadDesc->hide();
    ui->wUploadDesc->hide();
    ui->lBlockedItem->setText(QString::fromUtf8(""));
    ui->bDotUsedQuota->hide();
    ui->bDotUsedStorage->hide();
    ui->sUsedData->setCurrentWidget(ui->pStorage);

    ui->lDescDisabled->setText(QString::fromUtf8("<p style=\" line-height: 140%;\"><span style=\"font-size:14px;\">")
                               + ui->lDescDisabled->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                                          .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"))
                                                                   + QString::fromUtf8("</span></p>"));

#ifdef __APPLE__
    arrow = new QPushButton(this);
    arrow->setIcon(QIcon(QString::fromAscii("://images/top_arrow.png")));
    arrow->setIconSize(QSize(30,10));
    arrow->setStyleSheet(QString::fromAscii("border: none;"));
    arrow->resize(30,10);
    arrow->hide();
#endif

    //Create the overlay widget with a semi-transparent background
    //that will be shown over the transfers when they are paused
    overlay = new QPushButton(this);
    overlay->setIcon(QIcon(QString::fromAscii("://images/tray_paused_large_ico.png")));
    overlay->setIconSize(QSize(64, 64));
    overlay->setStyleSheet(QString::fromAscii("background-color: rgba(247, 247, 247, 200); "
                                              "border: none; "));

    ui->wTransfer1->hide();
    ui->wTransfer1->hide();
    overlay->resize(ui->wTransfers->minimumSize());
#ifdef __APPLE__
    overlay->move(1, 72);
#else
    overlay->move(2, 60);
    overlay->resize(overlay->width()-4, overlay->height());
#endif
    overlay->hide();
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));
    connect(ui->wTransfer1, SIGNAL(cancel(int, int)), this, SLOT(onTransfer1Cancel(int, int)));
    connect(ui->wTransfer2, SIGNAL(cancel(int, int)), this, SLOT(onTransfer2Cancel(int, int)));

    if (preferences->logged())
    {
        setUsage();
        updateSyncsButton();
    }
    else
    {
        regenerateLayout();
    }
}

InfoDialog::~InfoDialog()
{
    delete ui;
    delete gWidget;
    delete activeDownload;
    delete activeUpload;
}

void InfoDialog::setUserName()
{
    QString first = preferences->firstName();
    QString last = preferences->lastName();
    if (first.isNull() || last.isNull())
    {
        return;
    }
    QString pattern(QString::fromUtf8("%1 %2").arg(preferences->firstName()).arg(preferences->lastName()));

    QFont f = ui->lName->font();
    QFontMetrics fm = QFontMetrics(f);
    ui->lName->setText(fm.elidedText(pattern, Qt::ElideRight,ui->lName->maximumWidth()));
}

void InfoDialog::setAvatar()
{
    const char *email = megaApi->getMyEmail();
    if (email)
    {
        drawAvatar(QString::fromUtf8(email));
        delete [] email;
    }
}

void InfoDialog::setUsage()
{
    if (preferences->accountType() == 0)
    {
        ui->bDotUsedQuota->hide();
        ui->bDotUsedStorage->hide();
    }
    else
    {
        ui->bDotUsedQuota->show();
        ui->bDotUsedStorage->show();
    }

    on_bDotUsedStorage_clicked();

    if (preferences->totalStorage() == 0)
    {
        ui->pUsageStorage->setValue(0);
        ui->lPercentageUsedStorage->setText(QString::fromUtf8(""));
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(tr("Data temporarily unavailable")));
    }
    else
    {
        int percentage = ceil((100 * ((double)preferences->usedStorage()) / preferences->totalStorage()));
        ui->pUsageStorage->setValue((percentage < 100) ? percentage : 100);
        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">%1&nbsp;</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii("%"))))
                                     .arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(preferences->totalStorage())));
        ui->lPercentageUsedStorage->setText(used);
        ui->lTotalUsedStorage->setText(tr("USED STORAGE %1").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;&nbsp;%1</span>")
                                       .arg(Utilities::getSizeString(preferences->usedStorage()))));
    }

    if (preferences->totalBandwidth() == 0)
    {
        ui->pUsageQuota->setValue(0);
        ui->lPercentageUsedQuota->setText(QString::fromUtf8(""));
        ui->lTotalUsedQuota->setText(tr("USED BANDWIDTH %1").arg(tr("Data temporarily unavailable")));
    }
    else
    {
        int percentage = ceil(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
        ui->pUsageQuota->setValue((percentage < 100) ? percentage : 100);
        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">%1&nbsp;</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii("%"))))
                                     .arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(preferences->totalBandwidth())));
        ui->lPercentageUsedQuota->setText(used);
        ui->lTotalUsedQuota->setText(tr("USED BANDWIDTH %1").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;&nbsp;%1</span>")
                                                              .arg(Utilities::getSizeString(preferences->usedBandwidth()))));
    }
}

void InfoDialog::setTransfer(MegaTransfer *transfer)
{
    if (!transfer)
    {
        return;
    }

    int type = transfer->getType();
    long long completedSize = transfer->getTransferredBytes();
    long long totalSize = transfer->getTotalBytes();
    long long meanSpeed = transfer->getMeanSpeed();

    ActiveTransfer *wTransfer = NULL;
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        activeDownloadState = transfer->getState();
        long long speed = megaApi->getCurrentDownloadSpeed();
        meanDownloadSpeed = meanSpeed;
        remainingDownloadBytes = totalSize - completedSize;
        if (speed || downloadSpeed < 0)
        {
            downloadSpeed = speed;
        }

        wTransfer = ui->wTransfer1;
        if (!activeDownload || activeDownload->getTag() != transfer->getTag())
        {
            delete activeDownload;
            activeDownload = transfer->copy();
            wTransfer->setFileName(QString::fromUtf8(transfer->getFileName()));
        }
    }
    else
    {
        activeUploadState = transfer->getState();
        long long speed = megaApi->getCurrentUploadSpeed();
        remainingUploadBytes = totalSize - completedSize;
        meanUploadSpeed = meanSpeed;
        if (speed || uploadSpeed < 0)
        {
            uploadSpeed = speed;
        }

        wTransfer = ui->wTransfer2;
        if (!activeUpload || activeUpload->getTag() != transfer->getTag())
        {
            delete activeUpload;
            activeUpload = transfer->copy();
            wTransfer->setFileName(QString::fromUtf8(transfer->getFileName()));
        }
    }
    wTransfer->setProgress(completedSize, totalSize, !transfer->isSyncTransfer());
}

void InfoDialog::updateTransfers()
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();
    totalUploads = megaApi->getTotalUploads();
    totalDownloads = megaApi->getTotalDownloads();

    if (totalUploads < remainingUploads)
    {
        totalUploads = remainingUploads;
    }

    if (totalDownloads < remainingDownloads)
    {
        totalDownloads = remainingDownloads;
    }

    currentDownload = totalDownloads - remainingDownloads + 1;
    currentUpload = totalUploads - remainingUploads + 1;

    if (isVisible())
    {
        if (remainingDownloads)
        {
            int totalRemainingSeconds = meanDownloadSpeed ? remainingDownloadBytes / meanDownloadSpeed : 0;

            QString remainingTime;
            if (totalRemainingSeconds)
            {
                if (totalRemainingSeconds < 60)
                {
                    remainingTime = QString::fromUtf8("%1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(QString::fromUtf8("&lt; 1"));
                }
                else
                {
                    remainingTime = Utilities::getTimeString(totalRemainingSeconds, false);
                }
            }
            else
            {
                remainingTime = QString::fromAscii("");
            }

            ui->lRemainingTimeD->setText(remainingTime);
            ui->wDownloadDesc->show();
            QString fullPattern = QString::fromAscii("%1");
            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2"));
            QString invalidSpeedPattern(tr("%1 of %2"));
            QString downloadString;


            if (activeDownloadState == MegaTransfer::STATE_PAUSED || preferences->getDownloadsPaused())
            {
                downloadString = pausedPattern.arg(currentDownload).arg(totalDownloads) + QString::fromUtf8(" ") + tr("PAUSED");
            }
            else
            {
                if (downloadSpeed >= 20000)
                {
                    downloadString = pattern.arg(currentDownload)
                            .arg(totalDownloads)
                            .arg(Utilities::getSizeString(downloadSpeed));
                }
                else if (downloadSpeed >= 0)
                {
                    downloadString = invalidSpeedPattern.arg(currentDownload).arg(totalDownloads);
                }
                else
                {
                    downloadString = pausedPattern.arg(currentDownload).arg(totalDownloads) + QString::fromUtf8(" ") + tr("PAUSED");
                }
            }

            if (preferences->logged())
            {
                ui->lDownloads->setText(fullPattern.arg(downloadString));
                if (!ui->wTransfer1->isActive())
                {
                    ui->wDownloadDesc->hide();
                }
                else
                {
                    ui->wDownloadDesc->show();
                }
            }
        }

        if (remainingUploads)
        {
            int totalRemainingSeconds = meanUploadSpeed ? remainingUploadBytes / meanUploadSpeed : 0;

            QString remainingTime;
            if (totalRemainingSeconds)
            {
                if (totalRemainingSeconds < 60)
                {
                    remainingTime = QString::fromUtf8("%1 <span style=\"color:#777777; text-decoration:none;\">m</span>").arg(QString::fromUtf8("&lt; 1"));
                }
                else
                {
                    remainingTime = Utilities::getTimeString(totalRemainingSeconds, false);
                }
            }
            else
            {
                remainingTime = QString::fromAscii("");
            }

            ui->lRemainingTimeU->setText(remainingTime);
            ui->wUploadDesc->show();
            QString fullPattern = QString::fromAscii("%1");
            QString pattern(tr("%1 of %2 (%3/s)"));
            QString pausedPattern(tr("%1 of %2"));
            QString invalidSpeedPattern(tr("%1 of %2"));
            QString uploadString;

            if (activeUploadState == MegaTransfer::STATE_PAUSED || preferences->getUploadsPaused())
            {
                uploadString = pausedPattern.arg(currentUpload).arg(totalUploads) + QString::fromUtf8(" ") + tr("PAUSED");
            }
            else
            {
                if (uploadSpeed >= 20000)
                {
                    uploadString = pattern.arg(currentUpload).arg(totalUploads).arg(Utilities::getSizeString(uploadSpeed));
                }
                else if (uploadSpeed >= 0)
                {
                    uploadString = invalidSpeedPattern.arg(currentUpload).arg(totalUploads);
                }
                else
                {
                    uploadString = pausedPattern.arg(currentUpload).arg(totalUploads) + QString::fromUtf8(" ") + tr("PAUSED");
                }
            }

            ui->lUploads->setText(fullPattern.arg(uploadString));

            if (!ui->wTransfer2->isActive())
            {
                ui->wUploadDesc->hide();
            }
            else
            {
                ui->wUploadDesc->show();
            }
        }

        if (remainingUploads || remainingDownloads)
        {
            if (ui->wTransfer1->isActive() || ui->wTransfer2->isActive())
            {
                ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
            }
        }
    }
}

void InfoDialog::transferFinished(int error)
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();

    if (!remainingDownloads && ui->wTransfer1->isActive())
    {
        if (!downloadsFinishedTimer.isActive())
        {
            if (!error)
            {
                downloadsFinishedTimer.start();
            }
            else
            {
                onAllDownloadsFinished();
            }
        }
    }
    else
    {
        downloadsFinishedTimer.stop();
    }

    if (!remainingUploads && ui->wTransfer2->isActive())
    {
        if (!uploadsFinishedTimer.isActive())
        {
            if (!error)
            {
                uploadsFinishedTimer.start();
            }
            else
            {
                onAllUploadsFinished();
            }
        }
    }
    else
    {
        uploadsFinishedTimer.stop();
    }

    if (!remainingDownloads
            && !remainingUploads
            &&  (ui->sActiveTransfers->currentWidget() != ui->pUpdated))
    {
        if (!transfersFinishedTimer.isActive())
        {
            if (!error)
            {
                transfersFinishedTimer.start();
            }
            else
            {
                onAllTransfersFinished();
            }
        }
    }
    else
    {
        transfersFinishedTimer.stop();
    }
}

void InfoDialog::updateSyncsButton()
{
    int num = preferences->getNumSyncedFolders();
    long long firstSyncHandle = mega::INVALID_HANDLE;
    if (num == 1)
    {
        firstSyncHandle = preferences->getMegaFolderHandle(0);
    }

    MegaNode *rootNode = megaApi->getRootNode();
    if (!rootNode)
    {
        preferences->setCrashed(true);
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
        return;
    }
    long long rootHandle = rootNode->getHandle();

    if ((num == 1) && (firstSyncHandle == rootHandle))
    {
        ui->bSyncFolder->setText(QString::fromAscii("MEGA"));
    }
    else
    {
        ui->bSyncFolder->setText(tr("Syncs"));
    }

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

void InfoDialog::increaseUsedStorage(long long bytes, bool isInShare)
{
    if (isInShare)
    {
        preferences->setInShareStorage(preferences->inShareStorage() + bytes);
        preferences->setInShareFiles(preferences->inShareFiles()+1);
    }
    else
    {
        preferences->setCloudDriveStorage(preferences->cloudDriveStorage() + bytes);
        preferences->setCloudDriveFiles(preferences->cloudDriveFiles()+1);
    }

    preferences->setUsedStorage(preferences->usedStorage() + bytes);
    setUsage();
}

void InfoDialog::setOverQuotaMode(bool state)
{
    overQuotaState = state;
    if (state)
    {
        ui->sActiveTransfers->setCurrentWidget(ui->pOverQuota);
        ui->bUpgrade->setProperty("overquota", true);
        ui->pUsageStorage->setProperty("overquota", true);
        ui->bUpgrade->style()->unpolish(ui->bUpgrade);
        ui->bUpgrade->style()->polish(ui->bUpgrade);
        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);
    }
    else
    {
        ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
        ui->bUpgrade->setProperty("overquota", false);
        ui->pUsageStorage->setProperty("overquota", false);
        ui->bUpgrade->style()->unpolish(ui->bUpgrade);
        ui->bUpgrade->style()->polish(ui->bUpgrade);
        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);
    }
}

void InfoDialog::updateState()
{
    updateTransfers();
    if (preferences->getGlobalPaused())
    {
        if (!preferences->logged())
        {
            return;
        }

        downloadSpeed = -1;
        uploadSpeed = -1;
        if (state != STATE_PAUSED)
        {
            state = STATE_PAUSED;
            if (scanningTimer.isActive())
            {
                scanningTimer.stop();
            }

            ui->lSyncUpdated->setText(tr("Paused"));
            QIcon icon;
            icon.addFile(QString::fromUtf8(":/images/tray_paused_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);

            ui->label->setIcon(icon);
            ui->label->setIconSize(QSize(36, 36));
        }

        if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
        {
            overlay->setVisible(true);
        }
        else
        {
            overlay->setVisible(false);
        }
    }
    else
    {
        if (!preferences->logged())
        {
            return;
        }
        overlay->setVisible(false);
        if (downloadSpeed < 0 && uploadSpeed < 0)
        {
            downloadSpeed = 0;
            uploadSpeed = 0;
        }

        if (!waiting)
        {
            ui->lBlockedItem->setText(QString::fromUtf8(""));
        }

        if (waiting)
        {
            const char *blockedPath = megaApi->getBlockedPath();
            if (blockedPath)
            {
                QFileInfo fileBlocked (QString::fromUtf8(blockedPath));
                ui->lBlockedItem->setToolTip(fileBlocked.absoluteFilePath());
                ui->lBlockedItem->setAlignment(Qt::AlignLeft);
                ui->lBlockedItem->setText(tr("Blocked file: %1").arg(QString::fromUtf8("<a style=\" font-size: 12px;\" href=\"local://#%1\">%2</a>")
                                                               .arg(fileBlocked.absoluteFilePath())
                                                               .arg(fileBlocked.fileName())));
                delete [] blockedPath;
            }
            else if (megaApi->areServersBusy())
            {
                ui->lBlockedItem->setText(tr("Servers are too busy. Please wait..."));
                ui->lBlockedItem->setAlignment(Qt::AlignCenter);
            }
            else
            {
                ui->lBlockedItem->setText(QString::fromUtf8(""));
            }

            if (state != STATE_WAITING)
            {
                state = STATE_WAITING;
                if (scanningTimer.isActive())
                {
                    scanningTimer.stop();
                }

                ui->lSyncUpdated->setText(tr("Waiting"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/tray_scanning_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);

                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(36, 36));
            }
        }
        else if (indexing)
        {
            if (state != STATE_INDEXING)
            {
                state = STATE_INDEXING;
                if (!scanningTimer.isActive())
                {
                    scanningAnimationIndex = 1;
                    scanningTimer.start();
                }

                ui->lSyncUpdated->setText(tr("Scanning..."));

                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/tray_scanning_large_ico.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(36, 36));
            }
        }
        else
        {
            if (state != STATE_UPDATED)
            {
                state = STATE_UPDATED;
                if (scanningTimer.isActive())
                {
                    scanningTimer.stop();
                }

                ui->lSyncUpdated->setText(tr("Up to date"));
                QIcon icon;
                icon.addFile(QString::fromUtf8(":/images/empty_upToDate.png"), QSize(), QIcon::Normal, QIcon::Off);
                ui->label->setIcon(icon);
                ui->label->setIconSize(QSize(36, 36));
            }
        }
    }
}

void InfoDialog::closeSyncsMenu()
{
#ifdef __APPLE__
    if (syncsMenu && syncsMenu->isVisible())
    {
        syncsMenu->close();
    }

    if (transferMenu && transferMenu->isVisible())
    {
        transferMenu->close();
    }
#endif
}

void InfoDialog::addSync()
{
    addSync(INVALID_HANDLE);
}

void InfoDialog::onTransfer1Cancel(int x, int y)
{
    if (transferMenu)
    {
#ifdef __APPLE__
        transferMenu->close();
        return;
#else
        transferMenu->deleteLater();
#endif
    }

    transferMenu = new QMenu();
#ifndef __APPLE__
    transferMenu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeDownloadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume download"), this, SLOT(downloadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_DOWNLOAD) ? tr("Resume downloads") : tr("Pause downloads"), this, SLOT(globalDownloadState()));
    transferMenu->addAction(tr("Cancel download"), this, SLOT(cancelCurrentDownload()));
    transferMenu->addAction(tr("Cancel all downloads"), this, SLOT(cancelAllDownloads()));

#ifdef __APPLE__
    transferMenu->exec(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransfer1->mapToGlobal(QPoint(x, y)));
#endif
}

void InfoDialog::onTransfer2Cancel(int x, int y)
{
    if (transferMenu)
    {
#ifdef __APPLE__
        transferMenu->close();
        return;
#else
        transferMenu->deleteLater();
#endif
    }

    transferMenu = new QMenu();
#ifndef __APPLE__
    transferMenu->setStyleSheet(QString::fromAscii(
            "QMenu {background-color: white; border: 2px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeUploadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume upload"), this, SLOT(uploadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_UPLOAD) ? tr("Resume uploads") : tr("Pause uploads"), this, SLOT(globalUploadState()));
    transferMenu->addAction(tr("Cancel upload"), this, SLOT(cancelCurrentUpload()));
    transferMenu->addAction(tr("Cancel all uploads"), this, SLOT(cancelAllUploads()));

#ifdef __APPLE__
    transferMenu->exec(ui->wTransfer2->mapToGlobal(QPoint(x, y)));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransfer2->mapToGlobal(QPoint(x, y)));
#endif
}

void InfoDialog::globalDownloadState()
{
    if (!activeDownload)
    {
        return;
    }

    if (megaApi->areTransfersPaused(MegaTransfer::TYPE_DOWNLOAD))
    {
        megaApi->pauseTransfers(false, MegaTransfer::TYPE_DOWNLOAD);
    }
    else
    {
        megaApi->pauseTransfers(true, MegaTransfer::TYPE_DOWNLOAD);
    }
}

void InfoDialog::downloadState()
{
    if (!activeDownload)
    {
        return;
    }

    if (activeDownloadState == MegaTransfer::STATE_PAUSED)
    {
        megaApi->pauseTransfer(activeDownload, false);
    }
    else
    {
        megaApi->pauseTransfer(activeDownload, true);
    }
}

void InfoDialog::globalUploadState()
{
    if (!activeUpload)
    {
        return;
    }

    if (megaApi->areTransfersPaused(MegaTransfer::TYPE_UPLOAD))
    {
        megaApi->pauseTransfers(false, MegaTransfer::TYPE_UPLOAD);
    }
    else
    {
        megaApi->pauseTransfers(true, MegaTransfer::TYPE_UPLOAD);
    }
}

void InfoDialog::uploadState()
{
    if (!activeUpload)
    {
        return;
    }

    if (activeUploadState == MegaTransfer::STATE_PAUSED)
    {
        megaApi->pauseTransfer(activeUpload, false);
    }
    else
    {
        megaApi->pauseTransfer(activeUpload, true);
    }
}

void InfoDialog::cancelAllUploads()
{
    megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
}

void InfoDialog::cancelAllDownloads()
{
    megaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}

void InfoDialog::cancelCurrentUpload()
{
    megaApi->cancelTransfer(activeUpload);
}

void InfoDialog::cancelCurrentDownload()
{
    megaApi->cancelTransfer(activeDownload);
}

void InfoDialog::onAllUploadsFinished()
{
    remainingUploads = megaApi->getNumPendingUploads();
    if (!remainingUploads)
    {
        ui->wTransfer2->hideTransfer();
        ui->lUploads->setText(QString::fromAscii(""));
        ui->wUploadDesc->hide();
        uploadSpeed = 0;
        currentUpload = 0;
        totalUploads = 0;
        remainingUploadBytes = 0;
        meanUploadSpeed = 0;
        megaApi->resetTotalUploads();
    }
}

void InfoDialog::onAllDownloadsFinished()
{
    remainingDownloads = megaApi->getNumPendingDownloads();
    if (!remainingDownloads)
    {

        ui->wTransfer1->hideTransfer();
        ui->lDownloads->setText(QString::fromAscii(""));
        ui->wDownloadDesc->hide();

        downloadSpeed = 0;
        currentDownload = 0;
        totalDownloads = 0;
        remainingDownloadBytes = 0;
        meanDownloadSpeed = 0;
        megaApi->resetTotalDownloads();
    }
}

void InfoDialog::onAllTransfersFinished()
{
    if (!remainingDownloads && !remainingUploads)
    {
        if (!overQuotaState && (ui->sActiveTransfers->currentWidget() != ui->pUpdated))
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
        }

        if (preferences->logged())
        {
            app->updateUserStats();
        }

        app->showNotificationMessage(tr("All transfers have been completed"));
    }
}

void InfoDialog::on_bSettings_clicked()
{
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width()-6, ui->bSettings->height()));

#ifdef __APPLE__
    QPointer<InfoDialog> iod = this;
#endif

    app->showTrayMenu(&p);

#ifdef __APPLE__
    if (!iod)
    {
        return;
    }

    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }
#endif
}

void InfoDialog::on_bSyncFolder_clicked()
{
    int num = preferences->getNumSyncedFolders();

    MegaNode *rootNode = megaApi->getRootNode();
    if (!rootNode)
    {
        preferences->setCrashed(true);
        return;
    }

    if ((num == 1) && (preferences->getMegaFolderHandle(0) == rootNode->getHandle()))
    {
        openFolder(preferences->getLocalFolder(0));
    }
    else
    {
        syncsMenu = new QMenu();
        syncsMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));

        MenuItemAction *addSyncAction = new MenuItemAction(tr("Add Sync"), QIcon(QString::fromAscii("://images/ico_add_sync.png")),
                                                           QIcon(QString::fromAscii("://images/ico_drop_add_sync_over.png")));
        connect(addSyncAction, SIGNAL(triggered()), this, SLOT(addSync()));
        syncsMenu->addAction(addSyncAction);
        syncsMenu->addSeparator();

        QSignalMapper *menuSignalMapper = new QSignalMapper();
        connect(menuSignalMapper, SIGNAL(mapped(QString)), this, SLOT(openFolder(QString)));

        int activeFolders = 0;
        for (int i = 0; i < num; i++)
        {
            if (!preferences->isFolderActive(i))
            {
                continue;
            }

            activeFolders++;
            MenuItemAction *action = new MenuItemAction(preferences->getSyncName(i), QIcon(QString::fromAscii("://images/ico_drop_synched_folder.png")),
                                                        QIcon(QString::fromAscii("://images/ico_drop_synched_folder_over.png")));
            connect(action, SIGNAL(triggered()), menuSignalMapper, SLOT(map()));
            syncsMenu->addAction(action);
            menuSignalMapper->setMapping(action, preferences->getLocalFolder(i));
        }

        connect(syncsMenu, SIGNAL(aboutToHide()), syncsMenu, SLOT(deleteLater()));
        connect(syncsMenu, SIGNAL(destroyed(QObject*)), menuSignalMapper, SLOT(deleteLater()));

#ifdef __APPLE__
        syncsMenu->exec(this->mapToGlobal(QPoint(20, this->height() - (activeFolders + 1) * 28 - (activeFolders ? 16 : 8))));
        if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
        {
            this->hide();
        }
#else
        syncsMenu->popup(ui->bSyncFolder->mapToGlobal(QPoint(0, -activeFolders*35)));
#endif
        syncsMenu = NULL;
    }
    delete rootNode;
}

void InfoDialog::on_bUpgrade_clicked()
{
    QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
    QString url = QString::fromUtf8("pro/uao=%1").arg(userAgent);
    megaApi->getSessionTransferURL(url.toUtf8().constData());
}

void InfoDialog::openFolder(QString path)
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl::fromLocalFile(path));
}

void InfoDialog::addSync(MegaHandle h)
{
    static BindFolderDialog *dialog = NULL;
    if (dialog)
    {
        if (h != mega::INVALID_HANDLE)
        {
            dialog->setMegaFolder(h);
        }

        dialog->activateWindow();
        dialog->raise();
        dialog->setFocus();
        return;
    }

    dialog = new BindFolderDialog(app);
    if (h != mega::INVALID_HANDLE)
    {
        dialog->setMegaFolder(h);
    }

    int result = dialog->exec();
    if (result != QDialog::Accepted)
    {
        delete dialog;
        dialog = NULL;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(dialog->getLocalFolder()).canonicalPath());
    MegaHandle handle = dialog->getMegaFolder();
    MegaNode *node = megaApi->getNodeByHandle(handle);
    QString syncName = dialog->getSyncName();
    delete dialog;
    dialog = NULL;
    if (!localFolderPath.length() || !node)
    {
        delete node;
        return;
    }

   const char *nPath = megaApi->getNodePath(node);
   if (!nPath)
   {
       delete node;
       return;
   }

   preferences->addSyncedFolder(localFolderPath, QString::fromUtf8(nPath), handle, syncName);
   delete [] nPath;
   megaApi->syncFolder(localFolderPath.toUtf8().constData(), node);
   delete node;
   updateSyncsButton();
}

#ifdef __APPLE__
void InfoDialog::moveArrow(QPoint p)
{
    arrow->move(p.x()-(arrow->width()/2+1), 2);
    arrow->show();
}
#endif

void InfoDialog::on_bChats_clicked()
{
    QString userAgent = QString::fromUtf8(QUrl::toPercentEncoding(QString::fromUtf8(megaApi->getUserAgent())));
    QString url = QString::fromUtf8("").arg(userAgent);
    megaApi->getSessionTransferURL(url.toUtf8().constData());
}

void InfoDialog::on_bTransferManager_clicked()
{
    app->transferManagerActionClicked();
}

void InfoDialog::onOverlayClicked()
{
    app->pauseTransfers();
}

void InfoDialog::clearUserAttributes()
{
    ui->bAvatar->clearData();
}

void InfoDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        if (preferences->logged())
        {
            setUserName();
            if (preferences->totalStorage())
            {
                setUsage();
            }
            updateSyncsButton();
            state = STATE_STARTING;
            updateState();
            ui->lDescDisabled->setText(QString::fromUtf8("<p style=\" line-height: 140%;\"><span style=\"font-size:14px;\">")
                                       + ui->lDescDisabled->text().replace(QString::fromUtf8("[A]"), QString::fromUtf8("<font color=\"#d90007\"> "))
                                                                  .replace(QString::fromUtf8("[/A]"), QString::fromUtf8(" </font>"))
                                                                           + QString::fromUtf8("</span></p>"));
        }
    }
    QDialog::changeEvent(event);
}

bool InfoDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj != ui->pUsageStorage)
    {
        return false;
    }

    //Hide if InfoDialog is not visible
    if (e->type() == QEvent::Hide)
    {
        if (storageUsedMenu)
        {
            storageUsedMenu->hide();
        }
    }

    if (e->type() == QEvent::MouseButtonPress)
    {
        QMouseEvent* me = dynamic_cast<QMouseEvent*>(e);
        createQuotaUsedMenu();
        QPoint p = ui->pUsageStorage->mapToGlobal(me->pos());
        QSize s = storageUsedMenu->sizeHint();
        storageUsedMenu->exec(QPoint(p.x() - s.width() / 2, p.y() - s.height()));
    }

    return false;
}

void InfoDialog::regenerateLayout()
{
    static bool loggedInMode = true;

    if (loggedInMode == preferences->logged())
    {
        return;
    }
    loggedInMode = preferences->logged();

    QLayout *dialogLayout = layout();
    if (!loggedInMode)
    {
        if (!gWidget)
        {
            gWidget = new GuestWidget();
            connect(gWidget, SIGNAL(actionButtonClicked(int)), this, SLOT(onUserAction(int)));
        }

        ui->bTransferManager->setVisible(false);
        ui->bSyncFolder->setVisible(false);
        ui->bAvatar->setVisible(false);
        ui->bTransferManager->setVisible(false);
        dialogLayout->removeWidget(ui->wContainerHeader);
        ui->wContainerHeader->setVisible(false);
        dialogLayout->removeWidget(ui->wContainerBottom);
        ui->wContainerBottom->setVisible(false);
        dialogLayout->addWidget(gWidget);
        gWidget->setVisible(true);

        overlay->setVisible(false);
    }
    else
    {
        ui->bTransferManager->setVisible(true);
        ui->bSyncFolder->setVisible(true);
        ui->bAvatar->setVisible(true);
        ui->bTransferManager->setVisible(true);
        dialogLayout->removeWidget(gWidget);
        gWidget->setVisible(false);
        dialogLayout->addWidget(ui->wContainerHeader);
        ui->wContainerHeader->setVisible(true);
        dialogLayout->addWidget(ui->wContainerBottom);
        ui->wContainerBottom->setVisible(true);
    }

    if (activeDownload)
    {
        ActiveTransfer *wTransfer = ui->wTransfer1;
        wTransfer->setFileName(QString::fromUtf8(activeDownload->getFileName()));
        wTransfer->setProgress(activeDownload->getTotalBytes() - remainingDownloadBytes,
                               activeDownload->getTotalBytes(),
                               !activeDownload->isSyncTransfer());
    }

    updateTransfers();
    app->onGlobalSyncStateChanged(NULL);
}

void InfoDialog::drawAvatar(QString email)
{
    QString avatarsPath = Utilities::getAvatarPath(email);
    QFileInfo avatar(avatarsPath);
    if (avatar.exists())
    {
        ui->bAvatar->setAvatarImage(Utilities::getAvatarPath(email));
    }
    else
    {
        QString color;
        const char* userHandle = megaApi->getMyUserHandle();
        const char* avatarColor = megaApi->getUserAvatarColor(userHandle);
        if (avatarColor)
        {
            color = QString::fromUtf8(avatarColor);
            delete [] avatarColor;
        }
        ui->bAvatar->setAvatarLetter(Utilities::getAvatarLetter(), color);
        delete [] userHandle;
    }
}

void InfoDialog::createQuotaUsedMenu()
{
    if (!storageUsedMenu)
    {
        storageUsedMenu = new DataUsageMenu(this);
    }
    else
    {
        QList<QAction *> actions = storageUsedMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            storageUsedMenu->removeAction(actions[i]);
        }
    }

    if (cloudItem)
    {
        cloudItem->deleteLater();
        cloudItem = NULL;
    }
    cloudItem = new MenuItemAction(tr("Cloud Drive"), Utilities::getSizeString(preferences->cloudDriveStorage()), QIcon(QString::fromAscii("://images/ic_small_cloud_drive.png")), QSize(16,16));

    if (inboxItem)
    {
        inboxItem->deleteLater();
        inboxItem = NULL;
    }
    inboxItem = new MenuItemAction(tr("Inbox"), Utilities::getSizeString(preferences->inboxStorage()), QIcon(QString::fromAscii("://images/ic_small_inbox.png")), QSize(16,16));

    if (sharesItem)
    {
        sharesItem->deleteLater();
        sharesItem = NULL;
    }
    sharesItem = new MenuItemAction(tr("Incoming Shares"), Utilities::getSizeString(preferences->inShareStorage()), QIcon(QString::fromAscii("://images/ic_small_shares.png")), QSize(16,16));

    if (rubbishItem)
    {
        rubbishItem->deleteLater();
        rubbishItem = NULL;
    }
    rubbishItem = new MenuItemAction(tr("Rubbish bin"), Utilities::getSizeString(preferences->rubbishStorage()), QIcon(QString::fromAscii("://images/ic_small_rubbish.png")), QSize(16,16));

    storageUsedMenu->addAction(cloudItem);
    storageUsedMenu->addAction(inboxItem);
    storageUsedMenu->addAction(sharesItem);
    storageUsedMenu->addAction(rubbishItem);
}

void InfoDialog::onUserAction(int action)
{
    app->userAction(action);
}

void InfoDialog::on_bDotUsedStorage_clicked()
{
    ui->bDotUsedStorage->setIcon(QIcon(QString::fromAscii("://images/Nav_Dot_active.png")));
    ui->bDotUsedStorage->setIconSize(QSize(6,6));
    ui->bDotUsedQuota->setIcon(QIcon(QString::fromAscii("://images/Nav_Dot_inactive.png")));
    ui->bDotUsedQuota->setIconSize(QSize(6,6));

    ui->sUsedData->setCurrentWidget(ui->pStorage);
}

void InfoDialog::on_bDotUsedQuota_clicked()
{
    ui->bDotUsedStorage->setIcon(QIcon(QString::fromAscii("://images/Nav_Dot_inactive.png")));
    ui->bDotUsedStorage->setIconSize(QSize(6,6));
    ui->bDotUsedQuota->setIcon(QIcon(QString::fromAscii("://images/Nav_Dot_active.png")));
    ui->bDotUsedQuota->setIconSize(QSize(6,6));

    ui->sUsedData->setCurrentWidget(ui->pQuota);
}
void InfoDialog::scanningAnimationStep()
{
    scanningAnimationIndex = scanningAnimationIndex%18;
    scanningAnimationIndex++;
    QIcon icon;
    icon.addFile(QString::fromUtf8(":/images/scanning_anime")+
                 QString::number(scanningAnimationIndex) + QString::fromUtf8(".png") , QSize(), QIcon::Normal, QIcon::Off);

    ui->label->setIcon(icon);
    ui->label->setIconSize(QSize(36, 36));
}

#ifdef __APPLE__
void InfoDialog::paintEvent( QPaintEvent * e)
{
    QDialog::paintEvent(e);
    QPainter p( this );
    p.setCompositionMode( QPainter::CompositionMode_Clear);
    p.fillRect( ui->wArrow->rect(), Qt::transparent );
}

void InfoDialog::hideEvent(QHideEvent *event)
{
    arrow->hide();
    QDialog::hideEvent(event);
}
#endif
