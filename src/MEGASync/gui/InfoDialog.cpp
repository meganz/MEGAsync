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

#ifdef _WIN32    
#include <chrono>
using namespace std::chrono;
#endif

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent, InfoDialog* olddialog) :
    QDialog(parent),
    ui(new Ui::InfoDialog)
{
    ui->setupUi(this);
    //Set window properties
#ifdef Q_OS_LINUX
    if (true || !QSystemTrayIcon::isSystemTrayAvailable()) //To avoid issues with text input we implement popup ourselves by listening to WindowDeactivate event
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
    cloudItem = NULL;
    inboxItem = NULL;
    sharesItem = NULL;
    rubbishItem = NULL;
    gWidget = NULL;
    opacityEffect = NULL;
    animation = NULL;

    actualAccountType = -1;

    overQuotaState = false;
    storageState = Preferences::STATE_BELOW_OVER_STORAGE;

    ui->lSDKblock->setText(QString::fromUtf8(""));
    ui->wBlocked->setVisible(false);
    ui->wContainerBottom->setFixedHeight(56);

    //Initialize header dialog and disable chat features
    ui->wHeader->setStyleSheet(QString::fromUtf8("#wHeader {border: none;}"));

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);

#ifdef __APPLE__
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_9) //Issues with mavericks and popup management
    {
        installEventFilter(this);
    }
#endif

#ifdef Q_OS_LINUX
    installEventFilter(this);
#endif

    ui->lOQDesc->setTextFormat(Qt::RichText);

    state = STATE_STARTING;
    ui->wStatus->setState(state);

    megaApi = app->getMegaApi();
    preferences = Preferences::instance();

    actualAccountType = -1;

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
    connect(ui->wPSA, SIGNAL(PSAseen(int)), app, SLOT(PSAseen(int)), Qt::QueuedConnection);

    on_tTransfers_clicked();

    ui->wListTransfers->setupTransfers(olddialog?olddialog->stealModel():nullptr);

#ifdef __APPLE__
    arrow = new QPushButton(this);
    arrow->setIcon(QIcon(QString::fromAscii("://images/top_arrow.png")));
    arrow->setIconSize(QSize(30,10));
    arrow->setStyleSheet(QString::fromAscii("border: none;"));
    arrow->resize(30,10);
    arrow->hide();

    dummy = NULL;
#endif

    connect(this, SIGNAL(openTransferManager(int)), app, SLOT(externalOpenTransferManager(int)));

    if (preferences->logged())
    {
        setUsage();
    }
    else
    {
        regenerateLayout(olddialog);
    }
    highDpiResize.init(this);

#ifdef _WIN32
    lastWindowHideTime = std::chrono::steady_clock::now() - 5s;

    PSA_info *psaData = olddialog ? olddialog->getPSAdata() : nullptr;
    if (psaData)
    {
        this->setPSAannouncement(psaData->idPSA, psaData->title, psaData->desc,
                                 psaData->urlImage, psaData->textButton, psaData->urlClick);
        delete psaData;
    }
#endif
}

InfoDialog::~InfoDialog()
{
    delete ui;
    delete gWidget;
    delete activeDownload;
    delete activeUpload;
    delete animation;
}

PSA_info *InfoDialog::getPSAdata()
{
    if (ui->wPSA->isPSAshown())
    {
        PSA_info* info = new PSA_info(ui->wPSA->getPSAdata());
        return info;
    }

    return nullptr;
}

void InfoDialog::hideEvent(QHideEvent *event)
{
#ifdef __APPLE__
    arrow->hide();
#endif

    QDialog::hideEvent(event);

#ifdef _WIN32
    lastWindowHideTime = std::chrono::steady_clock::now();
#endif
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
    int accType = preferences->accountType();
    QString usedStorage;
    if (accType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->wCircularStorage->setValue(0);
        ui->lUsedStorage->setText(QString::fromUtf8(""));
        usedStorage = QString::fromUtf8("%1 used").arg(QString::fromUtf8("<span style=\"color:#333333; font-family: \"Lato\"; text-decoration:none;\">%1</span>")
                                     .arg(Utilities::getSizeString(preferences->usedStorage())));
    }
    else
    {
        if (preferences->totalStorage() == 0)
        {
            ui->wCircularStorage->setValue(0);
            usedStorage = QString::fromUtf8("");
        }
        else
        {

        int percentage = floor((100 * ((double)preferences->usedStorage()) / preferences->totalStorage()));
        ui->wCircularStorage->setValue((percentage < 100) ? percentage : 100);

        QString usageColorS = (percentage < 90 ? QString::fromUtf8("#6D6D6D")
                                                      : percentage >= MAX_VALUE ? QString::fromUtf8("#DF4843")
                                                      : QString::fromUtf8("#FF6F00"));

        usedStorage = QString::fromUtf8("%1 /%2").arg(QString::fromUtf8("<span style=\"color:%1; font-family: \"Lato\"; text-decoration:none;\">%2</span>")
                                     .arg(usageColorS).arg(Utilities::getSizeString(preferences->usedStorage())))
                                     .arg(QString::fromUtf8("<span style=\" font-family: \"Lato\"; text-decoration:none;\">&nbsp;%1</span>")
                                     .arg(Utilities::getSizeString(preferences->totalStorage())));
        }
    }

    ui->lUsedStorage->setText(usedStorage);

    QString usedQuota;
    if (accType == Preferences::ACCOUNT_TYPE_BUSINESS || accType == Preferences::ACCOUNT_TYPE_FREE)
    {
        ui->wCircularQuota->setValue(0);
        ui->lUsedQuota->setText(QString::fromUtf8(""));
        usedQuota = QString::fromUtf8("%1 used").arg(QString::fromUtf8("<span style=\"color:#333333; font-family: \"Lato\"; text-decoration:none;\">%1</span>")
                                     .arg(Utilities::getSizeString(preferences->usedBandwidth())));
    }
    else
    {
        if (preferences->totalBandwidth() == 0)
        {
            ui->wCircularQuota->setValue(0);
            usedQuota = QString::fromUtf8("");
        }
        else
        {
            int percentage = floor(100*((double)preferences->usedBandwidth()/preferences->totalBandwidth()));
            ui->wCircularQuota->setValue((percentage < 100) ? percentage : 100);

            QString usageColorB = (percentage < 90 ? QString::fromUtf8("#6D6D6D")
                                                          : percentage >= MAX_VALUE ? QString::fromUtf8("#DF4843")
                                                          : QString::fromUtf8("#FF6F00"));

            usedQuota = QString::fromUtf8("%1 /%2").arg(QString::fromUtf8("<span style=\"color:%1; font-family: \"Lato\"; text-decoration:none;\">%2</span>")
                                         .arg(usageColorB).arg(Utilities::getSizeString(preferences->usedBandwidth())))
                                         .arg(QString::fromUtf8("<span style=\"font-family: \"Lato\"; text-decoration:none;\">&nbsp;%1</span>")
                                         .arg(Utilities::getSizeString(preferences->totalBandwidth())));
        }
    }

    ui->lUsedQuota->setText(usedQuota);
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
    if (ui->wListTransfers->getModel())
    {
        ui->wListTransfers->getModel()->refreshTransfers();
    }
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
}

void InfoDialog::setIndexing(bool indexing)
{
    this->indexing = indexing;
}

void InfoDialog::setWaiting(bool waiting)
{
    this->waiting = waiting;
}

void InfoDialog::setOverQuotaMode(bool state)
{
    if (overQuotaState == state)
    {
        return;
    }

    overQuotaState = state;
    ui->wStatus->setOverQuotaState(state);
}

void InfoDialog::setAccountType(int accType)
{
    if (actualAccountType == accType)
    {
        return;
    }

    actualAccountType = accType;
    if (actualAccountType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
         ui->bUpgrade->hide();
    }
    else
    {
         ui->bUpgrade->show();
    }
}

void InfoDialog::updateBlockedState()
{
    if (!preferences->logged())
    {
        return;
    }

    if (!waiting)
    {
        if (ui->wBlocked->isVisible())
        {
            ui->lSDKblock->setText(QString::fromUtf8(""));
            ui->wBlocked->setVisible(false);
            ui->wContainerBottom->setFixedHeight(120);
        }
    }
    else
    {
        const char *blockedPath = megaApi->getBlockedPath();
        if (blockedPath)
        {
            QFileInfo fileBlocked (QString::fromUtf8(blockedPath));

            if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
            {
                ui->wContainerBottom->setFixedHeight(86);
                ui->wBlocked->setVisible(true);
                ui->lSDKblock->setText(tr("Blocked file: %1").arg(QString::fromUtf8("<a href=\"local://#%1\">%2</a>")
                                                                  .arg(fileBlocked.absoluteFilePath())
                                                                  .arg(fileBlocked.fileName())));
            }
            else
            {
                 ui->lSDKblock->setText(QString::fromUtf8(""));
                 ui->wBlocked->setVisible(false);
                 ui->wContainerBottom->setFixedHeight(56);
            }

            ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 14px;"));
            ui->lUploadToMegaDesc->setText(tr("Blocked file: %1").arg(QString::fromUtf8("<a href=\"local://#%1\">%2</a>")
                                                           .arg(fileBlocked.absoluteFilePath())
                                                           .arg(fileBlocked.fileName())));
            delete [] blockedPath;
        }
        else if (megaApi->areServersBusy())
        {

            if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
            {
                ui->wContainerBottom->setFixedHeight(86);
                ui->wBlocked->setVisible(true);
                ui->lSDKblock->setText(tr("The process is taking longer than expected. Please wait..."));
            }
            else
            {
                 ui->lSDKblock->setText(QString::fromUtf8(""));
                 ui->wBlocked->setVisible(false);
                 ui->wContainerBottom->setFixedHeight(56);
            }

            ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 14px;"));
            ui->lUploadToMegaDesc->setText(tr("The process is taking longer than expected. Please wait..."));
        }
        else
        {
            if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
            {
                ui->lSDKblock->setText(QString::fromUtf8(""));
                ui->wBlocked->setVisible(false);
                ui->wContainerBottom->setFixedHeight(56);
            }

            ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 14px;"));
            ui->lUploadToMegaDesc->setText(QString::fromUtf8(""));
        }
    }
}

void InfoDialog::updateState()
{
    if (!preferences->logged())
    {
        if (gWidget)
        {
            gWidget->resetFocus();
        }
    }

    if (!preferences->logged())
    {
        return;
    }

    if (preferences->getGlobalPaused())
    {
        state = STATE_PAUSED;
        animateStates(waiting || indexing);
    }
    else
    {
        if (waiting)
        {
            if (state != STATE_WAITING)
            {
                state = STATE_WAITING;
                animateStates(true);
            }
        }
        else if (indexing)
        {
            if (state != STATE_INDEXING)
            {
                state = STATE_INDEXING;
                animateStates(true);
            }
        }
        else
        {
            if (state != STATE_UPDATED)
            {
                state = STATE_UPDATED;
                animateStates(false);
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
            updateDialogState();
        }

        if ((QDateTime::currentMSecsSinceEpoch() - preferences->lastTransferNotificationTimestamp()) > Preferences::MIN_TRANSFER_NOTIFICATION_INTERVAL_MS)
        {
            app->showNotificationMessage(tr("All transfers have been completed"));
        }
    }
}

void InfoDialog::updateDialogState()
{
    updateState();
    switch (storageState)
    {
        case Preferences::STATE_ALMOST_OVER_STORAGE:
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_almost_full.png")));
            ui->bOQIcon->setIconSize(QSize(64,64));
            ui->lOQTitle->setText(tr("You're running out of storage space."));
            ui->lOQDesc->setText(tr("Upgrade to PRO now before your account runs full and your uploads to MEGA stop."));
            ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
            ui->wPSA->hidePSA();
            break;
        case Preferences::STATE_OVER_STORAGE:
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_full.png")));
            ui->bOQIcon->setIconSize(QSize(64,64));
            ui->lOQTitle->setText(tr("Your MEGA account is full."));
            ui->lOQDesc->setText(tr("All file uploads are currently disabled.")
                                    + QString::fromUtf8("<br>")
                                    + tr("Please upgrade to PRO."));
            ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
            ui->wPSA->hidePSA();
            break;
        case Preferences::STATE_BELOW_OVER_STORAGE:
        case Preferences::STATE_OVER_STORAGE_DISMISSED:
        default:
            remainingUploads = megaApi->getNumPendingUploads();
            remainingDownloads = megaApi->getNumPendingDownloads();

            if (remainingUploads || remainingDownloads || (ui->wListTransfers->getModel() && ui->wListTransfers->getModel()->rowCount(QModelIndex())) || ui->wPSA->isPSAready())
            {
                ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
                ui->wPSA->showPSA();
            }
            else
            {
                ui->wPSA->hidePSA();
                ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
            }
            break;
    }
    updateBlockedState();
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
    Preferences *preferences = Preferences::instance();
    if (preferences->lastPublicHandleTimestamp() && (QDateTime::currentMSecsSinceEpoch() - preferences->lastPublicHandleTimestamp()) < 86400000)
    {
        mega::MegaHandle aff = preferences->lastPublicHandle();
        if (aff != mega::INVALID_HANDLE)
        {
            char *base64aff = mega::MegaApi::handleToBase64(aff);
            url.append(QString::fromUtf8("/aff=%1/aff_time=%2").arg(QString::fromUtf8(base64aff)).arg(preferences->lastPublicHandleTimestamp() / 1000));
            delete [] base64aff;
        }
    }

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

void InfoDialog::on_bAddSync_clicked()
{
    addSync();
}

void InfoDialog::on_bUpload_clicked()
{
    app->uploadActionClicked();
}

void InfoDialog::on_bDownload_clicked()
{
    app->downloadActionClicked();
}

void InfoDialog::clearUserAttributes()
{
    ui->bAvatar->clearData();
}

bool InfoDialog::updateOverStorageState(int state)
{
    if (storageState != state)
    {
        storageState = state;
        updateDialogState();
        return true;
    }
    return false;
}

QCustomTransfersModel *InfoDialog::stealModel()
{
    QCustomTransfersModel *toret = ui->wListTransfers->getModel();
    ui->wListTransfers->setupTransfers(); // this will create a new empty model
    return toret;
}

void InfoDialog::setPSAannouncement(int id, QString title, QString text, QString urlImage, QString textButton, QString linkButton)
{
    ui->wPSA->setAnnounce(id, title, text, urlImage, textButton, linkButton);
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
            setUsage();
            state = STATE_STARTING;
            updateDialogState();
        }
    }
    QDialog::changeEvent(event);
}

bool InfoDialog::eventFilter(QObject *obj, QEvent *e)
{
#ifdef Q_OS_LINUX
    if (obj == this && e->type() == QEvent::WindowDeactivate)
    {
        close();
        return true;
    }
#endif
#ifdef __APPLE__
    if (QSysInfo::MacintoshVersion <= QSysInfo::MV_10_9) //manage spontaneus mouse press events
    {
        if (obj == this && e->type() == QEvent::MouseButtonPress && e->spontaneous())
        {
            return true;
        }
    }
#endif
    return QDialog::eventFilter(obj, e);
}

void InfoDialog::regenerateLayout(InfoDialog* olddialog)
{
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
            connect(gWidget, SIGNAL(forwardAction(int)), this, SLOT(onUserAction(int)));
            if (olddialog)
            {
                auto t = olddialog->gWidget->getTexts();
                gWidget->setTexts(t.first, t.second);
            }
        }
        else
        {
            gWidget->enableListener();
        }

        updateOverStorageState(Preferences::STATE_BELOW_OVER_STORAGE);
        setOverQuotaMode(false);
        ui->wPSA->removeAnnounce();

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

        #ifdef __APPLE__
            if (!dummy)
            {
                dummy = new QWidget();
            }

            dummy->resize(1,1);
            dummy->setWindowFlags(Qt::FramelessWindowHint);
            dummy->setAttribute(Qt::WA_NoSystemBackground);
            dummy->setAttribute(Qt::WA_TranslucentBackground);
            dummy->show();

            setMinimumHeight(404);
            setMaximumHeight(404);
        #else
            setMinimumHeight(394);
            setMaximumHeight(394);
        #endif
    }
    else
    {
        gWidget->disableListener();
        gWidget->initialize();

#ifdef __APPLE__
        setMinimumHeight(524);
        setMaximumHeight(524);
#else
        setMinimumHeight(514);
        setMaximumHeight(514);
#endif

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

        #ifdef __APPLE__
            if (dummy)
            {
                dummy->hide();
                delete dummy;
                dummy = NULL;
            }
        #endif
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

        Preferences *preferences = Preferences::instance();
        QString fullname = (preferences->firstName() + preferences->lastName()).trimmed();
        if (fullname.isEmpty())
        {
            char *email = megaApi->getMyEmail();
            if (email)
            {
                fullname = QString::fromUtf8(email);
                delete [] email;
            }
            else
            {
                fullname = preferences->email();
            }

            if (fullname.isEmpty())
            {
                fullname = QString::fromUtf8(" ");
            }
        }

        ui->bAvatar->setAvatarLetter(fullname.at(0).toUpper(), color);
        delete [] userHandle;
    }
}

void InfoDialog::animateStates(bool opt)
{
    if (opt) //Enable animation for scanning/waiting states
    {        
        ui->lUploadToMega->setIcon(QIcon(QString::fromAscii("://images/init_scanning.png")));
        ui->lUploadToMega->setIconSize(QSize(352,234));
        ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 14px;"));

        if (!opacityEffect)
        {
            opacityEffect = new QGraphicsOpacityEffect();
            ui->lUploadToMega->setGraphicsEffect(opacityEffect);
        }

        if (!animation)
        {
            animation = new QPropertyAnimation(opacityEffect, "opacity");
            animation->setDuration(2000);
            animation->setStartValue(1.0);
            animation->setEndValue(0.5);
            animation->setEasingCurve(QEasingCurve::InOutQuad);
            connect(animation, SIGNAL(finished()), SLOT(onAnimationFinished()));
        }

        if (animation->state() != QAbstractAnimation::Running)
        {
            animation->start();
        }
    }
    else //Disable animation
    {   
        ui->lUploadToMega->setIcon(QIcon(QString::fromAscii("://images/upload_to_mega.png")));
        ui->lUploadToMega->setIconSize(QSize(352,234));
        ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 18px;"));
        ui->lUploadToMegaDesc->setText(tr("Upload to MEGA now"));

        if (animation)
        {
            if (opacityEffect) //Reset opacity
            {
                opacityEffect->setOpacity(1.0);
            }

            if (animation->state() == QAbstractAnimation::Running)
            {
                animation->stop();
            }
        }
    }
}

void InfoDialog::onUserAction(int action)
{
    app->userAction(action);
}

void InfoDialog::on_tTransfers_clicked()
{
    ui->lTransfers->setStyleSheet(QString::fromUtf8("background-color: #3C434D;"));
    ui->lRecents->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tTransfers->setStyleSheet(QString::fromUtf8("color : #1D1D1D;"));
    ui->tRecents->setStyleSheet(QString::fromUtf8("color : #989899;"));
}

void InfoDialog::on_tRecents_clicked()
{
    ui->lTransfers->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lRecents->setStyleSheet(QString::fromUtf8("background-color: #3C434D;"));

    ui->tRecents->setStyleSheet(QString::fromUtf8("color : #1D1D1D;"));
    ui->tTransfers->setStyleSheet(QString::fromUtf8("color : #989899;"));
}

void InfoDialog::on_bDiscard_clicked()
{
    updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
    emit dismissOQ(overQuotaState);
}

void InfoDialog::on_bBuyQuota_clicked()
{
    on_bUpgrade_clicked();
}

void InfoDialog::onAnimationFinished()
{
    if (animation->direction() == QAbstractAnimation::Forward)
    {
        animation->setDirection(QAbstractAnimation::Backward);
        animation->start();
    }
    else
    {
        animation->setDirection(QAbstractAnimation::Forward);
        animation->start();
    }
}

#ifdef __APPLE__
void InfoDialog::paintEvent( QPaintEvent * e)
{
    QDialog::paintEvent(e);
    QPainter p( this );
    p.setCompositionMode( QPainter::CompositionMode_Clear);
    p.fillRect( ui->wArrow->rect(), Qt::transparent );
}
#endif
