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
    activeDownloadState = activeUploadState = MegaTransfer::STATE_NONE;
    remainingUploads = remainingDownloads = 0;
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

    ui->bTransferManager->setToolTip(tr("Open Transfer Manager"));
    ui->bSettings->setToolTip(tr("Show MEGAsync options"));

    ui->pUsageStorage->installEventFilter(this);
    ui->pUsageStorage->setMouseTracking(true);

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
            ui->pUsageStorage->setProperty("almostoq", false);
            ui->pUsageStorage->setProperty("crossedge", true);
        }
        else if (percentage > 90)
        {
            ui->pUsageStorage->setProperty("crossedge", false);
            ui->pUsageStorage->setProperty("almostoq", true);
        }
        else
        {
            ui->pUsageStorage->setProperty("crossedge", false);
            ui->pUsageStorage->setProperty("almostoq", false);
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
    if (type == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (!activeDownload || activeDownload->getTag() != transfer->getTag())
        {
            ui->wListTransfers->getModel()->updateActiveTransfer(megaApi, transfer);

            delete activeDownload;
            activeDownload = transfer->copy();
        }
    }
    else
    {
        if (!activeUpload || activeUpload->getTag() != transfer->getTag())
        {
            ui->wListTransfers->getModel()->updateActiveTransfer(megaApi, transfer);

            delete activeUpload;
            activeUpload = transfer->copy();
        }
    }

    ui->wListTransfers->getModel()->onTransferUpdate(megaApi, transfer);
}

void InfoDialog::refreshTransferItems()
{
    ui->wListTransfers->getModel()->refreshTransfers();
}

void InfoDialog::transferFinished(int error)
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();

    if (!remainingDownloads)
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

    if (!remainingUploads)
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
    ui->wStatus->setOverQuotaState(state);

    if (state)
    {
        ui->bUpgrade->setProperty("overquota", true);
        ui->pUsageStorage->setProperty("overquota", true);
        ui->bUpgrade->style()->unpolish(ui->bUpgrade);
        ui->bUpgrade->style()->polish(ui->bUpgrade);
        ui->pUsageStorage->style()->unpolish(ui->pUsageStorage);
        ui->pUsageStorage->style()->polish(ui->pUsageStorage);
    }
    else
    {
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
    if (preferences->getGlobalPaused())
    {
        if (!preferences->logged())
        {
            return;
        }

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
    app->createTrayMenu();
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

void InfoDialog::onAllUploadsFinished()
{
    remainingUploads = megaApi->getNumPendingUploads();
    if (!remainingUploads)
    {
        megaApi->resetTotalUploads();
    }
}

void InfoDialog::onAllDownloadsFinished()
{
    remainingDownloads = megaApi->getNumPendingDownloads();
    if (!remainingDownloads)
    {
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
    emit userActivity();

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
    emit userActivity();
    app->transferManagerActionClicked();
}

void InfoDialog::clearUserAttributes()
{
    ui->bAvatar->clearData();
}

void InfoDialog::handleOverStorage(int state)
{
    switch (state)
    {
        case Preferences::STATE_ALMOST_OVER_STORAGE:
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_almost_full.png")));
            ui->bOQIcon->setIconSize(QSize(64,64));
            ui->lOQTitle->setText(tr("You're running out of storage space."));
            ui->lOQDesc->setText(tr("Upgrade to PRO now before your account runs full and your uploads to MEGA stop."));
            ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
            break;
        case Preferences::STATE_OVER_STORAGE:
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_full.png")));
            ui->bOQIcon->setIconSize(QSize(64,64));
            ui->lOQTitle->setText(tr("Your MEGA account is full."));
            ui->lOQDesc->setText(tr("All file uploads are currently disabled. Please upgrade to PRO"));
            ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
            break;
        case Preferences::STATE_BELOW_OVER_STORAGE:
        default:
            ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
        break;
    }
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
        setMinimumHeight(512);
        setMaximumHeight(512);

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

void InfoDialog::on_bDismiss_clicked()
{
    ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
    emit dismissOQ(overQuotaState);
}

void InfoDialog::on_bBuyQuota_clicked()
{
    on_bUpgrade_clicked();
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
