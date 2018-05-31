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
#include "ui_InfoDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"
#include "MenuItemAction.h"
#include "platform/Platform.h"

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
#ifdef Q_OS_LINUX
    if (!QSystemTrayIcon::isSystemTrayAvailable())
    {
        setWindowFlags(Qt::FramelessWindowHint);
    }
    else
#endif
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
    }

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
    ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
    ui->wTransferDown->setType(MegaTransfer::TYPE_DOWNLOAD);
    ui->wTransferDown->hideTransfer();
    ui->wTransferUp->setType(MegaTransfer::TYPE_UPLOAD);
    ui->wTransferUp->hideTransfer();

    ui->bTransferManager->setToolTip(tr("Open Transfer Manager"));
    ui->bSettings->setToolTip(tr("Show MEGAsync options"));

    ui->pUsageStorage->installEventFilter(this);
    ui->pUsageStorage->setMouseTracking(true);

    ui->wDownloadDesc->installEventFilter(this);
    ui->wDownloadDesc->setMouseTracking(true);
    ui->bClockDown->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    ui->wUploadDesc->installEventFilter(this);
    ui->wUploadDesc->setMouseTracking(true);
    ui->bClockUp->setAttribute(Qt::WA_TransparentForMouseEvents, true);

    state = STATE_STARTING;
    ui->wStatus->setState(state);

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

    connect(ui->wStatus, SIGNAL(clicked()), app, SLOT(pauseTransfers()), Qt::QueuedConnection);

    ui->wDownloadDesc->hide();
    ui->wUploadDesc->hide();
    ui->lBlockedItem->setText(QString::fromUtf8(""));
    ui->bDotUsedQuota->hide();
    ui->bDotUsedStorage->hide();
    ui->sUsedData->setCurrentWidget(ui->pStorage);

    ui->wListTransfers->setupTransfers();


#ifdef __APPLE__
    arrow = new QPushButton(this);
    arrow->setIcon(QIcon(QString::fromAscii("://images/top_arrow.png")));
    arrow->setIconSize(QSize(30,10));
    arrow->setStyleSheet(QString::fromAscii("border: none;"));
    arrow->resize(30,10);
    arrow->hide();
#endif

    on_bDotUsedStorage_clicked();

    ui->wTransferDown->hide();
    ui->wTransferUp->hide();

    connect(ui->wTransferDown, SIGNAL(showContextMenu(QPoint, bool)), this, SLOT(onContextDownloadMenu(QPoint, bool)));
    connect(ui->wTransferUp, SIGNAL(showContextMenu(QPoint, bool)), this, SLOT(onContextUploadMenu(QPoint, bool)));
    connect(ui->wTransferDown, SIGNAL(openTransferManager(int)), app, SLOT(externalOpenTransferManager(int)));
    connect(ui->wTransferUp, SIGNAL(openTransferManager(int)), app, SLOT(externalOpenTransferManager(int)));

    connect(this, SIGNAL(openTransferManager(int)), app, SLOT(externalOpenTransferManager(int)));

    if (preferences->logged())
    {
        setUsage();
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
    QString pattern(QString::fromUtf8("%1 %2").arg(first).arg(last));

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
        if (percentage > 100)
        {
            ui->pUsageStorage->setProperty("crossedge", true);
        }
        else
        {
            ui->pUsageStorage->setProperty("crossedge", false);
        }
        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);

        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">%1&nbsp;</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii(" %"))))
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
        ui->lTotalUsedQuota->setText(tr("TRANSFER QUOTA %1").arg(tr("Data temporarily unavailable")));
    }
    else
    {
        int percentage = ceil(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
        ui->pUsageQuota->setValue((percentage < 100) ? percentage : 100);
        if (percentage > 100)
        {
            ui->pUsageQuota->setProperty("crossedge", true);
        }
        else
        {
            ui->pUsageQuota->setProperty("crossedge", false);
        }
        ui->pUsageQuota->style()->unpolish(ui->pUsageQuota);
        ui->pUsageQuota->style()->polish(ui->pUsageQuota);

        QString used = tr("%1 of %2").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">%1&nbsp;</span>")
                                     .arg(QString::number(percentage).append(QString::fromAscii("%"))))
                                     .arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(preferences->totalBandwidth())));
        ui->lPercentageUsedQuota->setText(used);
        ui->lTotalUsedQuota->setText(tr("TRANSFER QUOTA %1").arg(QString::fromUtf8("<span style=\"color:#333333; font-size: 16px; text-decoration:none;\">&nbsp;&nbsp;%1</span>")
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

        wTransfer = ui->wTransferDown;

        if (!activeDownload || activeDownload->getTag() != transfer->getTag())
        {
            ui->wListTransfers->getModel()->updateActiveTransfer(megaApi, transfer);

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

        wTransfer = ui->wTransferUp;
        if (!activeUpload || activeUpload->getTag() != transfer->getTag())
        {
            ui->wListTransfers->getModel()->updateActiveTransfer(megaApi, transfer);

            delete activeUpload;
            activeUpload = transfer->copy();
            wTransfer->setFileName(QString::fromUtf8(transfer->getFileName()));
        }
    }
    wTransfer->setProgress(completedSize, totalSize, !transfer->isSyncTransfer());
    ui->wListTransfers->getModel()->onTransferUpdate(megaApi, transfer);
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
        QString formattedValue(QString::fromUtf8("<span style=\"color:#333333; text-decoration:none;\">%1</span>"));

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
                downloadString = pausedPattern.arg(formattedValue.arg(currentDownload)).arg(formattedValue.arg(totalDownloads)) + QString::fromUtf8(" ") + tr("PAUSED");
            }
            else
            {
                if (downloadSpeed >= 20000)
                {
                    downloadString = pattern.arg(formattedValue.arg(currentDownload))
                            .arg(formattedValue.arg(totalDownloads))
                            .arg(Utilities::getSizeString(downloadSpeed));
                }
                else if (downloadSpeed >= 0)
                {
                    downloadString = invalidSpeedPattern.arg(formattedValue.arg(currentDownload)).arg(formattedValue.arg(totalDownloads));
                }
                else
                {
                    downloadString = pausedPattern.arg(formattedValue.arg(currentDownload)).arg(formattedValue.arg(totalDownloads)) + QString::fromUtf8(" ") + tr("PAUSED");
                }
            }

            if (preferences->logged())
            {
                ui->lDownloads->setText(fullPattern.arg(downloadString));
                if (!ui->wTransferDown->isActive())
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
                uploadString = pausedPattern.arg(formattedValue.arg(currentUpload)).arg(formattedValue.arg(totalUploads)) + QString::fromUtf8(" ") + tr("PAUSED");
            }
            else
            {
                if (uploadSpeed >= 20000)
                {
                    uploadString = pattern.arg(formattedValue.arg(currentUpload)).arg(formattedValue.arg(totalUploads)).arg(Utilities::getSizeString(uploadSpeed));
                }
                else if (uploadSpeed >= 0)
                {
                    uploadString = invalidSpeedPattern.arg(formattedValue.arg(currentUpload)).arg(formattedValue.arg(totalUploads));
                }
                else
                {
                    uploadString = pausedPattern.arg(formattedValue.arg(currentUpload)).arg(formattedValue.arg(totalUploads)) + QString::fromUtf8(" ") + tr("PAUSED");
                }
            }

            ui->lUploads->setText(fullPattern.arg(uploadString));

            if (!ui->wTransferUp->isActive())
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
            if (ui->wTransferDown->isActive() || ui->wTransferUp->isActive())
            {
                ui->sActiveTransfers->setCurrentWidget(ui->pUpdating);
            }
        }
    }
}

void InfoDialog::refreshTransferItems()
{
    ui->wListTransfers->getModel()->refreshTransfers();
}

void InfoDialog::transferFinished(int error)
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();

    if (!remainingDownloads && ui->wTransferDown->isActive())
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

    if (!remainingUploads && ui->wTransferUp->isActive())
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
    }
    else
    {
        if (!preferences->logged())
        {
            return;
        }

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
                ui->lBlockedItem->setAlignment(Qt::AlignCenter);
                ui->lBlockedItem->setText(tr("Blocked file: %1").arg(QString::fromUtf8("<a style=\" font-size: 12px;\" href=\"local://#%1\">%2</a>")
                                                               .arg(fileBlocked.absoluteFilePath())
                                                               .arg(fileBlocked.fileName())));
                delete [] blockedPath;
            }
            else if (megaApi->areServersBusy())
            {
                ui->lBlockedItem->setText(tr("The process is taking longer than expected. Please wait..."));
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

    ui->wStatus->setState(state);
}

void InfoDialog::addSync()
{
    addSync(INVALID_HANDLE);
    app->regenerateTrayMenu();
}

void InfoDialog::onContextDownloadMenu(QPoint pos, bool regular)
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
            "QMenu {background-color: white; border: 1px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeDownloadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume download"), this, SLOT(downloadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_DOWNLOAD) ? tr("Resume downloads") : tr("Pause downloads"), this, SLOT(globalDownloadState()));

    if (regular)
    {
        transferMenu->addAction(tr("Cancel download"), this, SLOT(cancelCurrentDownload()));
        transferMenu->addAction(tr("Cancel all downloads"), this, SLOT(cancelAllDownloads()));
    }

#ifdef __APPLE__
    transferMenu->exec(ui->wTransferDown->mapToGlobal(pos));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransferDown->mapToGlobal(pos));
#endif
}

void InfoDialog::onContextUploadMenu(QPoint pos, bool regular)
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
            "QMenu {background-color: white; border: 1px solid #B8B8B8; padding: 5px; border-radius: 5px;} "
            "QMenu::item {background-color: white; color: black;} "
            "QMenu::item:selected {background-color: rgb(242, 242, 242);}"));
#endif

    if (activeUploadState == MegaTransfer::STATE_PAUSED)
    {
        transferMenu->addAction(tr("Resume upload"), this, SLOT(uploadState()));
    }
    transferMenu->addAction(megaApi->areTransfersPaused(MegaTransfer::TYPE_UPLOAD) ? tr("Resume uploads") : tr("Pause uploads"), this, SLOT(globalUploadState()));

    if (regular)
    {
        transferMenu->addAction(tr("Cancel upload"), this, SLOT(cancelCurrentUpload()));
        transferMenu->addAction(tr("Cancel all uploads"), this, SLOT(cancelAllUploads()));
    }

#ifdef __APPLE__
    transferMenu->exec(ui->wTransferUp->mapToGlobal(pos));
    if (!this->rect().contains(this->mapFromGlobal(QCursor::pos())))
    {
        this->hide();
    }

    transferMenu->deleteLater();
    transferMenu = NULL;
#else
    transferMenu->popup(ui->wTransferUp->mapToGlobal(pos));
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
        ui->wTransferUp->hideTransfer();
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
        ui->wTransferDown->hideTransfer();
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

        if ((QDateTime::currentMSecsSinceEpoch() - preferences->lastTransferNotificationTimestamp()) > Preferences::MIN_TRANSFER_NOTIFICATION_INTERVAL_MS)
        {
            app->showNotificationMessage(tr("All transfers have been completed"));
        }
    }
}

void InfoDialog::on_bSettings_clicked()
{
    QPoint p = ui->bSettings->mapToGlobal(QPoint(ui->bSettings->width() - 2, ui->bSettings->height()));

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
    static QPointer<BindFolderDialog> dialog = NULL;
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

    Platform::execBackgroundWindow(dialog);
    if (!dialog)
    {
        return;
    }

    if (dialog->result() != QDialog::Accepted)
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

void InfoDialog::clearUserAttributes()
{
    ui->bAvatar->clearData();
}

void InfoDialog::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    ui->wListTransfers->getModel()->onTransferFinish(api, transfer, e);
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
            state = STATE_STARTING;
            ui->wStatus->setState(state);
            updateState();   
        }
    }
    QDialog::changeEvent(event);
}

bool InfoDialog::eventFilter(QObject *obj, QEvent *e)
{
    if (obj == ui->wDownloadDesc)
    {
        if( e->type() == QEvent::MouseButtonPress
                        && ((QMouseEvent *)e)->button() == Qt::LeftButton)
        {
            emit openTransferManager(TransferManager::DOWNLOADS_TAB);
        }
        else if (e->type() == Qt::ToolTip)
        {
            ui->wDownloadDesc->setToolTip(tr("Open Transfer Manager"));
        }
    }

    if (obj == ui->wUploadDesc)
    {
        if( e->type() == QEvent::MouseButtonPress
                        && ((QMouseEvent *)e)->button() == Qt::LeftButton)
        {
            emit openTransferManager(TransferManager::UPLOADS_TAB);
        }
        else if (e->type() == Qt::ToolTip)
        {
            ui->wUploadDesc->setToolTip(tr("Open Transfer Manager"));
        }
    }

    if (obj != ui->pUsageStorage)
    {
        return false;
    }

    QPoint mousePos;
    switch (e->type())
    {
    case QEvent::Hide:
    case QEvent::Enter:
         hideUsageBalloon();
         break;

    case QEvent::MouseButtonPress:
    {
         QMouseEvent *me = dynamic_cast<QMouseEvent*>(e);
         mousePos = me->pos();
         break;
    }
    case QEvent::ToolTip:
    {
         QHelpEvent *me = static_cast<QHelpEvent*>(e);
         mousePos = me->pos();
         break;
    }
    default:
         break;

    }

    if (!mousePos.isNull())
    {
        createQuotaUsedMenu();
        QPoint p = ui->pUsageStorage->mapToGlobal(mousePos);
        QSize s = storageUsedMenu->sizeHint();
        storageUsedMenu->exec(QPoint(p.x() - s.width() / 2, p.y() - s.height()));
    }

    return QDialog::eventFilter(obj, e);
}

void InfoDialog::regenerateLayout()
{
    static bool loggedInMode = true;
    bool logged = preferences->logged();

    if (loggedInMode == logged)
    {
        return;
    }
    loggedInMode = logged;

    QLayout *dialogLayout = layout();
    if (!loggedInMode)
    {
        if (!gWidget)
        {
            gWidget = new GuestWidget();
            connect(gWidget, SIGNAL(actionButtonClicked(int)), this, SLOT(onUserAction(int)));
        }

        ui->bTransferManager->setVisible(false);
        ui->bAvatar->setVisible(false);
        ui->bTransferManager->setVisible(false);
        dialogLayout->removeWidget(ui->wContainerHeader);
        ui->wContainerHeader->setVisible(false);
        dialogLayout->removeWidget(ui->wSeparator);
        ui->wSeparator->setVisible(false);
        dialogLayout->removeWidget(ui->wContainerBottom);
        ui->wContainerBottom->setVisible(false);
        dialogLayout->addWidget(gWidget);
        gWidget->setVisible(true);

        setMinimumHeight(385);
        setMaximumHeight(385);
    }
    else
    {
        setMinimumHeight(366);
        setMaximumHeight(366);

        ui->bTransferManager->setVisible(true);
        ui->bAvatar->setVisible(true);
        ui->bTransferManager->setVisible(true);
        dialogLayout->removeWidget(gWidget);
        gWidget->setVisible(false);
        dialogLayout->addWidget(ui->wContainerHeader);
        ui->wContainerHeader->setVisible(true);    
        dialogLayout->addWidget(ui->wSeparator);
        ui->wSeparator->setVisible(true);
        dialogLayout->addWidget(ui->wContainerBottom);
        ui->wContainerBottom->setVisible(true);
    }

    if (activeDownload)
    {
        ActiveTransfer *wTransfer = ui->wTransferDown;
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
    cloudItem = new MenuItemAction(tr("Cloud Drive"), Utilities::getSizeString(preferences->cloudDriveStorage()), QIcon(QString::fromAscii("://images/ic_small_cloud_drive.png")), false, QSize(16,16));

    if (inboxItem)
    {
        inboxItem->deleteLater();
        inboxItem = NULL;
    }
    inboxItem = new MenuItemAction(tr("Inbox"), Utilities::getSizeString(preferences->inboxStorage()), QIcon(QString::fromAscii("://images/ic_small_inbox.png")), false, QSize(16,16));

    if (sharesItem)
    {
        sharesItem->deleteLater();
        sharesItem = NULL;
    }
    sharesItem = new MenuItemAction(tr("Incoming Shares"), Utilities::getSizeString(preferences->inShareStorage()), QIcon(QString::fromAscii("://images/ic_small_shares.png")), false, QSize(16,16));

    if (rubbishItem)
    {
        rubbishItem->deleteLater();
        rubbishItem = NULL;
    }
    rubbishItem = new MenuItemAction(tr("Rubbish bin"), Utilities::getSizeString(preferences->rubbishStorage()), QIcon(QString::fromAscii("://images/ic_small_rubbish.png")), false, QSize(16,16));

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

void InfoDialog::hideUsageBalloon()
{
    if (storageUsedMenu)
    {
        storageUsedMenu->hide();
    }
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
