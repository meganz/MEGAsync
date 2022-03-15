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
#include <QScrollBar>
#include "InfoDialog.h"
#include "ui_InfoDialog.h"
#include "control/Utilities.h"
#include "MegaApplication.h"
#include "MenuItemAction.h"
#include "platform/Platform.h"
#include "assert.h"
#include "BackupsWizard.h"
#include "QMegaMessageBox.h"

// TODO: remove
#include "control/MegaController.h"

#ifdef _WIN32    
#include <chrono>
using namespace std::chrono;
#endif

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

static constexpr int DEFAULT_MIN_PERCENTAGE{1};

void InfoDialog::pauseResumeClicked()
{
    app->pauseTransfers();
}

void InfoDialog::generalAreaClicked()
{
    app->transferManagerActionClicked();
}

void InfoDialog::dlAreaClicked()
{
    app->transferManagerActionClicked(TransferManager::DOWNLOADS_TAB);
}

void InfoDialog::upAreaClicked()
{
    app->transferManagerActionClicked(TransferManager::UPLOADS_TAB);
}

void InfoDialog::pauseResumeHovered(QMouseEvent *event)
{
    QToolTip::showText(event->globalPos(), tr("Pause/Resume"));
}

void InfoDialog::generalAreaHovered(QMouseEvent *event)
{
    QToolTip::showText(event->globalPos(), tr("Open Transfer Manager"));
}
void InfoDialog::dlAreaHovered(QMouseEvent *event)
{
    QToolTip::showText(event->globalPos(), tr("Open Downloads"));
}

void InfoDialog::upAreaHovered(QMouseEvent *event)
{
    QToolTip::showText(event->globalPos(), tr("Open Uploads"));
}

InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent, InfoDialog* olddialog) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    mSyncController (new SyncController()),
    mBackupRootHandle (mega::INVALID_HANDLE)
{
    ui->setupUi(this);

    filterMenu = new FilterAlertWidget(this);
    connect(filterMenu, SIGNAL(onFilterClicked(int)), this, SLOT(applyFilterOption(int)));

    setUnseenNotifications(0);

#if QT_VERSION > 0x050200
    QSizePolicy sp_retain = ui->bNumberUnseenNotifications->sizePolicy();
    sp_retain.setRetainSizeWhenHidden(true);
    ui->bNumberUnseenNotifications->setSizePolicy(sp_retain);
#endif
    ui->tvNotifications->setVerticalScrollMode(QAbstractItemView::ScrollPerPixel);
    ui->tvNotifications->verticalScrollBar()->setSingleStep(12);

    connect(ui->bTransferManager, SIGNAL(pauseResumeClicked()), this, SLOT(pauseResumeClicked()));
    connect(ui->bTransferManager, SIGNAL(generalAreaClicked()), this, SLOT(generalAreaClicked()));
    connect(ui->bTransferManager, SIGNAL(upAreaClicked()), this, SLOT(upAreaClicked()));
    connect(ui->bTransferManager, SIGNAL(dlAreaClicked()), this, SLOT(dlAreaClicked()));

    connect(ui->bTransferManager, SIGNAL(pauseResumeHovered(QMouseEvent *)), this, SLOT(pauseResumeHovered(QMouseEvent *)));
    connect(ui->bTransferManager, SIGNAL(generalAreaHovered(QMouseEvent *)), this, SLOT(generalAreaHovered(QMouseEvent *)));
    connect(ui->bTransferManager, SIGNAL(upAreaHovered(QMouseEvent *)), this, SLOT(upAreaHovered(QMouseEvent*)));
    connect(ui->bTransferManager, SIGNAL(dlAreaHovered(QMouseEvent *)), this, SLOT(dlAreaHovered(QMouseEvent *)));

    connect(ui->wSortNotifications, SIGNAL(clicked()), this, SLOT(on_bActualFilter_clicked()));
    connect(app, &MegaApplication::avatarReady, this, &InfoDialog::setAvatar);


    //Set window properties
#ifdef Q_OS_LINUX
    doNotActAsPopup = Platform::getValue("USE_MEGASYNC_AS_REGULAR_WINDOW", false);

    if (!doNotActAsPopup && QSystemTrayIcon::isSystemTrayAvailable())
    {
        // To avoid issues with text input we implement a popup ourselves
        // instead of using Qt::Popup by listening to the WindowDeactivate
        // event.
        Qt::WindowFlags flags = Qt::FramelessWindowHint;

        if (Platform::isTilingWindowManager())
        {
            flags |= Qt::Dialog;
        }

        setWindowFlags(flags);
    }
    else
    {
        setWindowFlags(Qt::Window);
        doNotActAsPopup = true; //the first time systray is not available will set this flag to true to disallow popup until restarting
    }
#else
    setWindowFlags(Qt::FramelessWindowHint | Qt::Popup);
#endif

#ifdef _WIN32
    if(getenv("QT_SCREEN_SCALE_FACTORS") || getenv("QT_SCALE_FACTOR"))
    {
        //do not use WA_TranslucentBackground when using custom scale factors in windows
        setStyleSheet(styleSheet().append(QString::fromUtf8("#wInfoDialogIn{border-radius: 0px;}" ) ));
    }
    else
#endif
    {
        setAttribute(Qt::WA_TranslucentBackground);
    }

    //Initialize fields
    this->app = app;

    circlesShowAllActiveTransfersProgress = true;

    indexing = false;
    waiting = false;
    syncing = false;
    transferring = false;
    activeDownload = NULL;
    activeUpload = NULL;
    cloudItem = NULL;
    inboxItem = NULL;
    sharesItem = NULL;
    rubbishItem = NULL;
    gWidget = NULL;
    opacityEffect = NULL;
    animation = NULL;
    accountDetailsDialog = NULL;

    actualAccountType = -1;

    notificationsReady = false;
    ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
    ui->wSortNotifications->setActualFilter(AlertFilterType::ALL_TYPES);

    overQuotaState = false;
    storageState = Preferences::STATE_BELOW_OVER_STORAGE;

    reset();

    ui->lSDKblock->setText(QString());
    ui->wBlocked->setVisible(false);

    //Initialize header dialog and disable chat features
    ui->wHeader->setStyleSheet(QString::fromUtf8("#wHeader {border: none;}"));

    //Set properties of some widgets
    ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);

    ui->sStorage->setCurrentWidget(ui->wCircularStorage);
    ui->sQuota->setCurrentWidget(ui->wCircularQuota);

    ui->wCircularQuota->setProgressBarGradient(QColor("#60D1FE"), QColor("#58B9F3"));

#ifdef __APPLE__
    auto current = QOperatingSystemVersion::current();
    if (current <= QOperatingSystemVersion::OSXMavericks) //Issues with mavericks and popup management
    {
        installEventFilter(this);
    }
#endif

#ifdef Q_OS_LINUX
    installEventFilter(this);
#endif

    ui->wStorageUsage->installEventFilter(this);

    ui->lOQDesc->setTextFormat(Qt::RichText);

    state = STATE_STARTING;
    ui->wStatus->setState(state);

    megaApi = app->getMegaApi();
    preferences = Preferences::instance();
    model = SyncModel::instance();
    controller = Controller::instance();

    actualAccountType = -1;

    connect(model, SIGNAL(syncDisabledListUpdated()), this, SLOT(updateDialogState()));

    uploadsFinishedTimer.setSingleShot(true);
    uploadsFinishedTimer.setInterval(5000);
    connect(&uploadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllUploadsFinished()));

    downloadsFinishedTimer.setSingleShot(true);
    downloadsFinishedTimer.setInterval(5000);
    connect(&downloadsFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllDownloadsFinished()));

    transfersFinishedTimer.setSingleShot(true);
    transfersFinishedTimer.setInterval(5000);
    connect(&transfersFinishedTimer, SIGNAL(timeout()), this, SLOT(onAllTransfersFinished()));

    connect(ui->wPSA, SIGNAL(PSAseen(int)), app, SLOT(PSAseen(int)), Qt::QueuedConnection);

    connect(ui->sTabs, SIGNAL(currentChanged(int)), this, SLOT(sTabsChanged(int)), Qt::QueuedConnection);

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

    //Create the overlay widget with a transparent background
    overlay = new QPushButton(ui->pUpdated);
    overlay->setStyleSheet(QString::fromAscii("background-color: transparent; "
                                              "border: none; "));
    overlay->resize(ui->pUpdated->size());
    overlay->setCursor(Qt::PointingHandCursor);

    overlay->resize(overlay->width()-4, overlay->height());

    overlay->show();
    connect(overlay, SIGNAL(clicked()), this, SLOT(onOverlayClicked()));
    connect(this, SIGNAL(openTransferManager(int)), app, SLOT(externalOpenTransferManager(int)));

    if (preferences->logged())
    {
        setUsage();
    }
    else
    {
        regenerateLayout(MegaApi::ACCOUNT_NOT_BLOCKED, olddialog);
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

    minHeightAnimationBlockedError = new QPropertyAnimation();
    maxHeightAnimationBlockedError = new QPropertyAnimation();

    animationGroupBlockedError.addAnimation(minHeightAnimationBlockedError);
    animationGroupBlockedError.addAnimation(maxHeightAnimationBlockedError);
    connect(&animationGroupBlockedError, SIGNAL(finished()), this, SLOT(onAnimationFinishedBlockedError()));

    adjustSize();
}

InfoDialog::~InfoDialog()
{
    removeEventFilter(this);
    delete ui;
    delete gWidget;
    delete activeDownload;
    delete activeUpload;
    delete animation;
    delete filterMenu;
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

void InfoDialog::showEvent(QShowEvent *event)
{
    emit ui->sTabs->currentChanged(ui->sTabs->currentIndex());
    if (ui->bTransferManager->alwaysAnimateOnShow || ui->bTransferManager->neverPainted )
    {
        ui->bTransferManager->showAnimated();
    }

    isShown = true;
    QDialog::showEvent(event);
}

void InfoDialog::setBandwidthOverquotaState(QuotaState state)
{
    transferQuotaState = state;
}

void InfoDialog::enableTransferOverquotaAlert()
{
    if (!transferOverquotaAlertEnabled)
    {
        transferOverquotaAlertEnabled = true;
        emit transferOverquotaMsgVisibilityChange(transferOverquotaAlertEnabled);
    }
    updateDialogState();
}

void InfoDialog::enableTransferAlmostOverquotaAlert()
{
    if (!transferAlmostOverquotaAlertEnabled)
    {
        transferAlmostOverquotaAlertEnabled = true;
        emit almostTransferOverquotaMsgVisibilityChange(transferAlmostOverquotaAlertEnabled);
    }
    updateDialogState();
}

void InfoDialog::hideEvent(QHideEvent *event)
{
#ifdef __APPLE__
    arrow->hide();
#endif

    if (filterMenu && filterMenu->isVisible())
    {
        filterMenu->hide();
    }

    QTimer::singleShot(1000, this, [this] () {
        if (!isShown)
        {
            emit ui->sTabs->currentChanged(-1);
        }
    });


    isShown = false;
    if (ui->bTransferManager->alwaysAnimateOnShow || ui->bTransferManager->neverPainted )
    {
        ui->bTransferManager->shrink(true);
    }
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
    auto accType = preferences->accountType();

    // ---------- Process storage usage
    QString usedStorageString;

    auto totalStorage(preferences->totalStorage());
    auto usedStorage(preferences->usedStorage());

    if (accType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->sStorage->setCurrentWidget(ui->wBusinessStorage);
        ui->wCircularStorage->setValue(0);
        usedStorageString = QString::fromUtf8("<span style='color: #333333; font-size:20px;"
                                              "font-family: Lato; text-decoration:none;'>%1</span>")
                                     .arg(Utilities::getSizeString(preferences->usedStorage()));
    }
    else
    {
        ui->sStorage->setCurrentWidget(ui->wCircularStorage);
        if (totalStorage == 0)
        {
            ui->wCircularStorage->setValue(0ull);
            usedStorageString = Utilities::getSizeString(0ull);
        }
        else
        {
            QString usageColorS;
            switch (preferences->getStorageState())
                {
                    case MegaApi::STORAGE_STATE_GREEN:
                    {
                        ui->wCircularStorage->setState(CircularUsageProgressBar::STATE_OK);
                        usageColorS = QString::fromUtf8("#666666");
                        break;
                    }
                    case MegaApi::STORAGE_STATE_ORANGE:
                    {
                        ui->wCircularStorage->setState(CircularUsageProgressBar::STATE_WARNING);
                        usageColorS = QString::fromUtf8("#F98400");
                        break;
                    }
                    case MegaApi::STORAGE_STATE_PAYWALL:
                    // Fallthrough
                    case MegaApi::STORAGE_STATE_RED:
                    {
                        ui->wCircularStorage->setState(CircularUsageProgressBar::STATE_OVER);
                        usageColorS = QString::fromUtf8("#D90007");
                        break;
                    }
                }
            auto parts (usedStorage ?
                            std::max(Utilities::partPer(usedStorage, totalStorage),
                                     DEFAULT_MIN_PERCENTAGE)
                          : 0);
            ui->wCircularStorage->setValue(parts);
            usedStorageString = QString::fromUtf8("%1 /%2")
                    .arg(QString::fromUtf8("<span style='color:%1;"
                                           "font-family: Lato;"
                                           "text-decoration:none;'>%2</span>")
                         .arg(usageColorS, Utilities::getSizeString(usedStorage)))
                    .arg(QString::fromUtf8("<span style=' font-family: Lato;"
                                           "text-decoration:none;'>&nbsp;%1</span>")
                         .arg(Utilities::getSizeString(totalStorage)));
        }
    }

    ui->lUsedStorage->setText(usedStorageString);

    // ---------- Process transfer usage
    QString usedTransferString;

    auto usedTransfer(preferences->usedBandwidth());

    if (accType == Preferences::ACCOUNT_TYPE_BUSINESS)
    {
        ui->sQuota->setCurrentWidget(ui->wBusinessQuota);
        ui->wCircularStorage->setTotalValueUnknown();
        usedTransferString = QString::fromUtf8("<span style='color: #333333;"
                                                    "font-size:20px; font-family: Lato;"
                                                    "text-decoration:none;'>%1</span>")
                                  .arg(Utilities::getSizeString(usedTransfer));
    }
    else
    {
        ui->sQuota->setCurrentWidget(ui->wCircularQuota);

        QString usageColor;
        // Set color according to state
        switch (transferQuotaState)
        {
            case QuotaState::OK:
            {
                ui->wCircularQuota->setState(CircularUsageProgressBar::STATE_OK);
                usageColor = QString::fromUtf8("#666666");
                break;
            }
            case QuotaState::WARNING:
            {
                ui->wCircularQuota->setState(CircularUsageProgressBar::STATE_WARNING);
                usageColor = QString::fromUtf8("#F98400");
                break;
            }
            case QuotaState::OVERQUOTA:
            // Fallthrough
            case QuotaState::FULL:
            {
                ui->wCircularQuota->setState(CircularUsageProgressBar::STATE_OVER);
                usageColor = QString::fromUtf8("#D90007");
                break;
            }
        }

        if (accType == Preferences::ACCOUNT_TYPE_FREE)
        {

            ui->wCircularQuota->setTotalValueUnknown(transferQuotaState != QuotaState::FULL
                                                        && transferQuotaState != QuotaState::OVERQUOTA);
            usedTransferString = tr("%1 used")
                                 .arg(QString::fromUtf8("<span style='color:%1;"
                                                        "font-family: Lato;"
                                                        "text-decoration:none;'>%2</span>")
                                      .arg(usageColor, Utilities::getSizeString(usedTransfer)));
        }
        else
        {
            auto totalTransfer (preferences->totalBandwidth());
            if (totalTransfer == 0)
            {
                ui->wCircularQuota->setTotalValueUnknown();
                usedTransferString = Utilities::getSizeString(0ull);
            }
            else
            {
                auto parts (usedTransfer ?
                                std::max(Utilities::partPer(usedTransfer, totalTransfer),
                                         DEFAULT_MIN_PERCENTAGE)
                              : 0);

                ui->wCircularQuota->setValue(parts);
                usedTransferString = QString::fromUtf8("%1 /%2")
                                     .arg(QString::fromUtf8("<span style='color:%1;"
                                                            "font-family: Lato;"
                                                            "text-decoration:none;'>%2</span>")
                                          .arg(usageColor, Utilities::getSizeString(usedTransfer)),
                                          QString::fromUtf8("<span style='font-family: Lato;"
                                                            "text-decoration:none;"
                                                            "'>&nbsp;%1</span>")
                                          .arg(Utilities::getSizeString(totalTransfer)));
            }
        }
    }

    ui->lUsedQuota->setText(usedTransferString);
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

void InfoDialog::updateTransfersCount()
{
    remainingUploads = megaApi->getNumPendingUploads();
    remainingDownloads = megaApi->getNumPendingDownloads();

    totalUploads = megaApi->getTotalUploads();
    totalDownloads = megaApi->getTotalDownloads();

    int currentDownload = totalDownloads - remainingDownloads + 1;
    int currentUpload = totalUploads - remainingUploads + 1;

    if (remainingDownloads <= 0 && !remainingDownloadsTimerRunning)
    {
        remainingDownloadsTimerRunning = true;
        QTimer::singleShot(5000, this, [this] () {
            if (remainingDownloads <= 0)
            {
                ui->bTransferManager->setCompletedDownloads(0);
                ui->bTransferManager->setTotalDownloads(0);
            }

            remainingDownloadsTimerRunning = false;
        });
    }
    if (remainingUploads <= 0 && !remainingUploadsTimerRunning)
    {
        remainingUploadsTimerRunning = true;
        QTimer::singleShot(5000, this, [this] () {
            if (remainingUploads <= 0)
            {
                ui->bTransferManager->setCompletedUploads(0);
                ui->bTransferManager->setTotalUploads(0);
            }

            remainingUploadsTimerRunning = false;
        });
    }

    ui->bTransferManager->setCompletedDownloads(qMax(0,qMin(totalDownloads, currentDownload)));
    ui->bTransferManager->setCompletedUploads(qMax(0,qMin(totalUploads,currentUpload)));
    ui->bTransferManager->setTotalDownloads(totalDownloads);
    ui->bTransferManager->setTotalUploads(totalUploads);
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

void InfoDialog::setSyncing(bool value)
{
    this->syncing = value;
}

void InfoDialog::setTransferring(bool value)
{
    this->transferring = value;
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
            setBlockedStateLabel(QString::fromUtf8(""));
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
                setBlockedStateLabel(tr("Blocked file: %1").arg(QString::fromUtf8("<a href=\"local://#%1\">%2</a>")
                                                                .arg(fileBlocked.absoluteFilePath())
                                                                .arg(fileBlocked.fileName())));
            }
            else
            {
                 setBlockedStateLabel(QString::fromUtf8(""));
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
                setBlockedStateLabel(tr("The process is taking longer than expected. Please wait..."));
            }
            else
            {
                setBlockedStateLabel(QString::fromUtf8(""));
            }

            ui->lUploadToMegaDesc->setStyleSheet(QString::fromUtf8("font-size: 14px;"));
            ui->lUploadToMegaDesc->setText(tr("The process is taking longer than expected. Please wait..."));
        }
        else
        {
            if (ui->sActiveTransfers->currentWidget() != ui->pUpdated)
            {
                setBlockedStateLabel(QString::fromUtf8(""));
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
        animateStates(waiting || indexing || syncing);
    }
    else
    {
        if (indexing)
        {
            if (state != STATE_INDEXING)
            {
                state = STATE_INDEXING;
                animateStates(true);
            }
        }
        else if (syncing)
        {
            if (state != STATE_SYNCING)
            {
                state = STATE_SYNCING;
                animateStates(true);
            }
        }
        else if (waiting)
        {
            if (state != STATE_WAITING)
            {
                state = STATE_WAITING;
                animateStates(true);
            }
        }
        else if (transferring)
        {
            if (state != STATE_TRANSFERRING)
            {
                state = STATE_TRANSFERRING;
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
    ui->bTransferManager->setPaused(preferences->getGlobalPaused());
}

void InfoDialog::addSync()
{
    const bool upgradingDissmised{app->showSyncOverquotaDialog()};
    if(upgradingDissmised)
    {
        addSync(INVALID_HANDLE);
        app->createAppMenus();
    }
}

void InfoDialog::onAddSync(mega::MegaSync::SyncType type)
{
    const bool upgradingDissmised{app->showSyncOverquotaDialog()};
    if(upgradingDissmised)
    {
        switch (type)
        {
            case mega::MegaSync::TYPE_TWOWAY:
            {
                addSync(INVALID_HANDLE);
                break;
            }
            case mega::MegaSync::TYPE_BACKUP:
            {
                addBackup();
                break;
            }
            default:
            {
                break;
            }
        }
        app->createAppMenus();
    }
}

void InfoDialog::onAddBackup()
{
    const bool upgradingDissmised{app->showSyncOverquotaDialog()};
    if(upgradingDissmised)
    {
        addBackup();
        app->createAppMenus();
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
    const bool transferOverQuotaEnabled{transferQuotaState == QuotaState::FULL &&
                transferOverquotaAlertEnabled};

    if (storageState == Preferences::STATE_PAYWALL)
    {
        MegaIntegerList* tsWarnings = megaApi->getOverquotaWarningsTs();
        const char *email = megaApi->getMyEmail();

        long long numFiles{preferences->cloudDriveFiles() + preferences->inboxFiles() + preferences->rubbishFiles()};
        QString overDiskText = QString::fromUtf8("<p style='line-height: 20px;'>") + ui->lOverDiskQuotaLabel->text()
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(email))
                .replace(QString::fromUtf8("[B]"), Utilities::getReadableStringFromTs(tsWarnings))
                .replace(QString::fromUtf8("[C]"), QString::number(numFiles))
                .replace(QString::fromUtf8("[D]"), Utilities::getSizeString(preferences->usedStorage()))
                .replace(QString::fromUtf8("[E]"), Utilities::minProPlanNeeded(MegaSyncApp->getPricing(), preferences->usedStorage()))
                + QString::fromUtf8("</p>");
        ui->lOverDiskQuotaLabel->setText(overDiskText);

        int64_t remainDaysOut(0);
        int64_t remainHoursOut(0);
        Utilities::getDaysAndHoursToTimestamp(megaApi->getOverquotaDeadlineTs() * 1000, remainDaysOut, remainHoursOut);
        if (remainDaysOut > 0)
        {
            QString descriptionDays = tr("You have [A][B] days[/A] left to upgrade. After that, your data is subject to deletion.");
            ui->lWarningOverDiskQuota->setText(QString::fromUtf8("<p style='line-height: 20px;'>") + descriptionDays
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style='color: #FF6F00;'>"))
                    .replace(QString::fromUtf8("[B]"), QString::number(remainDaysOut))
                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                    + QString::fromUtf8("</p>"));
        }
        else if (remainDaysOut == 0 && remainHoursOut > 0)
        {
            QString descriptionHours = tr("You have [A][B] hours[/A] left to upgrade. After that, your data is subject to deletion.");
            ui->lWarningOverDiskQuota->setText(QString::fromUtf8("<p style='line-height: 20px;'>") + descriptionHours
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style='color: #FF6F00;'>"))
                    .replace(QString::fromUtf8("[B]"), QString::number(remainHoursOut))
                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                    + QString::fromUtf8("</p>"));
        }
        else
        {
            ui->lWarningOverDiskQuota->setText(tr("You must act immediately to save your data"));
        }


        delete tsWarnings;
        delete [] email;

        ui->sActiveTransfers->setCurrentWidget(ui->pOverDiskQuotaPaywall);
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else if(storageState == Preferences::STATE_OVER_STORAGE)
    {
        const bool transferIsOverQuota{transferQuotaState == QuotaState::FULL};
        const bool userIsFree{preferences->accountType() == Preferences::Preferences::ACCOUNT_TYPE_FREE};
        if(transferIsOverQuota && userIsFree)
        {
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_transfer_full_FREE.png")));
            ui->bOQIcon->setIconSize(QSize(96,96));
        }
        else if(transferIsOverQuota && !userIsFree)
        {
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_transfer_full_PRO.png")));
            ui->bOQIcon->setIconSize(QSize(96,96));
        }
        else
        {
            ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_full.png")));
            ui->bOQIcon->setIconSize(QSize(64,64));
        }
        ui->lOQTitle->setText(tr("Your MEGA account is full."));
        ui->lOQDesc->setText(tr("All file uploads are currently disabled.")
                                + QString::fromUtf8("<br>")
                                + tr("Please upgrade to PRO."));
        ui->bBuyQuota->setText(tr("Buy more space"));
        ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else if(transferOverQuotaEnabled)
    {
        ui->bOQIcon->setIcon(QIcon(QString::fromAscii(":/images/transfer_empty_64.png")));
        ui->bOQIcon->setIconSize(QSize(64,64));
        ui->lOQTitle->setText(tr("Depleted transfer quota."));
        ui->lOQDesc->setText(tr("All downloads are currently disabled.")
                                + QString::fromUtf8("<br>")
                                + tr("Please upgrade to PRO."));
        ui->bBuyQuota->setText(tr("Upgrade"));
        ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else if(storageState == Preferences::STATE_ALMOST_OVER_STORAGE)
    {
        ui->bOQIcon->setIcon(QIcon(QString::fromAscii("://images/storage_almost_full.png")));
        ui->bOQIcon->setIconSize(QSize(64,64));
        ui->lOQTitle->setText(tr("You're running out of storage space."));
        ui->lOQDesc->setText(tr("Upgrade to PRO now before your account runs full and your uploads to MEGA stop."));
        ui->bBuyQuota->setText(tr("Buy more space"));
        ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else if(transferQuotaState == QuotaState::WARNING &&
            transferAlmostOverquotaAlertEnabled)
    {
        ui->bOQIcon->setIcon(QIcon(QString::fromAscii(":/images/transfer_empty_64.png")));
        ui->bOQIcon->setIconSize(QSize(64,64));
        ui->lOQTitle->setText(tr("Limited available transfer quota"));
        ui->lOQDesc->setText(tr("Your queued transfers exceed the current quota available for your IP"
                                " address and can therefore be interrupted."));
        ui->bBuyQuota->setText(tr("Upgrade"));
        ui->sActiveTransfers->setCurrentWidget(ui->pOverquota);
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else if (model->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY)
             || model->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP))
    {
        if (model->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY)
            && model->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP))
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pAllSyncsDisabled);
        }
        else if (model->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP))
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pBackupsDisabled);
        }
        else
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pSyncsDisabled);
        }
        overlay->setVisible(false);
        ui->wPSA->hidePSA();
    }
    else
    {
        remainingUploads = megaApi->getNumPendingUploads();
        remainingDownloads = megaApi->getNumPendingDownloads();

        if (remainingUploads || remainingDownloads || (ui->wListTransfers->getModel() && ui->wListTransfers->getModel()->rowCount(QModelIndex())) || ui->wPSA->isPSAready())
        {
            overlay->setVisible(false);
            ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
            ui->wPSA->showPSA();
        }
        else
        {
            ui->wPSA->hidePSA();
            ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
            if (!waiting && !indexing)
            {
                overlay->setVisible(true);
            }
            else
            {
                overlay->setVisible(false);
            }
        }
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
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

void InfoDialog::on_bUpgradeOverDiskQuota_clicked()
{
    on_bUpgrade_clicked();
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
    QString syncName = dialog->getSyncName();
    delete dialog;
    dialog = NULL;


   MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Adding sync %1 from addSync: ").arg(localFolderPath).toUtf8().constData());

   ActionProgress *addSyncStep = new ActionProgress(true, QString::fromUtf8("Adding sync: %1")
                                                    .arg(localFolderPath));

   //Connect failing signals
   connect(addSyncStep, &ActionProgress::failed, this, [localFolderPath](int errorCode)
   {
       static_cast<MegaApplication *>(qApp)->showAddSyncError(errorCode, localFolderPath);
   }, Qt::QueuedConnection);
   connect(addSyncStep, &ActionProgress::failedRequest, this, [this, localFolderPath](MegaRequest *request, MegaError *error)
   {
       if (error->getErrorCode())
       {
           auto reqCopy = request->copy();
           auto errCopy = error->copy();

           QObject temporary;
           QObject::connect(&temporary, &QObject::destroyed, this, [reqCopy, errCopy, localFolderPath](){

               // we might want to handle this separately (i.e: indicate errors in SyncSettings engine)
               static_cast<MegaApplication *>(qApp)->showAddSyncError(reqCopy, errCopy, localFolderPath);

               delete reqCopy;
               delete errCopy;
               //(syncSettings might have some old values), that's why we don't use syncSetting->getError.
           }, Qt::QueuedConnection);
       }
   }, Qt::DirectConnection); //Note, we need direct connection to use request & error

   controller->addSync(localFolderPath, handle, syncName, addSyncStep);
}

void InfoDialog::addBackup()
{
    // If no backups configured: show wizard, else show single add backup dialog
    int nbBackups(SyncModel::instance()->getNumSyncedFolders(mega::MegaSync::TYPE_BACKUP));

    if (nbBackups > 0)
    {
        QPointer<AddBackupDialog> addBackup = new AddBackupDialog();
        addBackup->setAttribute(Qt::WA_DeleteOnClose);
        addBackup->setWindowModality(Qt::ApplicationModal);

        connect(mSyncController.get(), &SyncController::backupsRootDirHandle, this, [this, addBackup](mega::MegaHandle h)
        {
            if (h == mega::INVALID_HANDLE)
                return;

            mBackupRootHandle = h;
            if (addBackup)
                addBackup->setMyBackupsFolder(QString::fromUtf8(megaApi->getNodePathByNodeHandle(mBackupRootHandle)));
        });

        mSyncController->getBackupsRootDirHandle();

        connect(mSyncController.get(), &SyncController::syncAddStatus, this, [](const int errorCode, const QString errorMsg, QString name)
        {
            if (errorCode != MegaError::API_OK)
            {
                QMegaMessageBox::warning(nullptr, tr("Error adding backup %1").arg(name), errorMsg);
            }
        });

        connect(addBackup, &AddBackupDialog::accepted, this, [this, addBackup]()
        {
            if (mBackupRootHandle == mega::INVALID_HANDLE)
                return;

            if(addBackup)
            {
                QDir dirToBackup (addBackup->getSelectedFolder());
                mSyncController->addSync(QDir::toNativeSeparators(dirToBackup.canonicalPath()),
                                         mBackupRootHandle, dirToBackup.dirName(),
                                         MegaSync::TYPE_BACKUP);
            }
        });

        addBackup->show();
    }
    else
    {
        BackupsWizard *wizard = new BackupsWizard();
        wizard->setAttribute(Qt::WA_DeleteOnClose);
        wizard->setWindowModality(Qt::ApplicationModal);
        wizard->show();
    }
}

#ifdef __APPLE__
void InfoDialog::moveArrow(QPoint p)
{
    arrow->move(p.x()-(arrow->width()/2+1), 2);
    arrow->show();
}
#endif

void InfoDialog::onOverlayClicked()
{
    app->uploadActionClicked();
}

void InfoDialog::on_bTransferManager_clicked()
{
    emit userActivity();
    app->transferManagerActionClicked();
}

void InfoDialog::on_bAddSync_clicked()
{
    showSyncsMenu(ui->bAddSync, MegaSync::TYPE_TWOWAY);
}

void InfoDialog::on_bAddBackup_clicked()
{
    showSyncsMenu(ui->bAddBackup, MegaSync::TYPE_BACKUP);
}

void InfoDialog::showSyncsMenu(QPushButton* b, mega::MegaSync::SyncType type)
{
    if (preferences->logged())
    {
        auto menu (mSyncsMenus[b]);
        if (!menu)
        {
            menu = new SyncsMenu(type, this);
            connect(menu, &SyncsMenu::addSync, this, &InfoDialog::onAddSync);
            mSyncsMenus[b] = menu;
        }
        menu->callMenu(b->mapToGlobal(QPoint(b->width() - 100, b->height() + 3)));
    }
}

void InfoDialog::on_bUpload_clicked()
{
    app->uploadActionClicked();
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

void InfoDialog::updateNotificationsTreeView(QAbstractItemModel *model, QAbstractItemDelegate *delegate)
{
    notificationsReady = true;
    ui->tvNotifications->setModel(model);
    ui->tvNotifications->setItemDelegate(delegate);
    ui->sNotifications->setCurrentWidget(ui->pNotifications);
}

void InfoDialog::reset()
{
    activeDownloadState = activeUploadState = MegaTransfer::STATE_NONE;
    remainingUploads = remainingDownloads = 0;
    totalUploads = totalDownloads = 0;
    leftUploadBytes = completedUploadBytes = 0;
    leftDownloadBytes = completedDownloadBytes = 0;
    uploadActiveTransferPriority = downloadActiveTransferPriority = 0xFFFFFFFFFFFFFFFFULL;
    uploadActiveTransferTag = downloadActiveTransferTag = -1;
    notificationsReady = false;
    ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
    ui->wSortNotifications->setActualFilter(AlertFilterType::ALL_TYPES);

    ui->bTransferManager->reset();

    ui->wBlocked->hide();
    shownBlockedError = false;

    setUnseenNotifications(0);
    if (filterMenu)
    {
        filterMenu->reset();
    }

    transferOverquotaAlertEnabled = false;
    transferAlmostOverquotaAlertEnabled = false;
    transferQuotaState = QuotaState::OK;
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


    updateTransfersCount();
    if (transfer)
    {
        if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
        {
            if (downloadActiveTransferTag == transfer->getTag())
            {
                downloadActiveTransferTag = -1;
            }

            if (e->getErrorCode() != MegaError::API_OK)
            {
                leftDownloadBytes -= transfer->getTotalBytes();
                completedDownloadBytes -= transfer->getTransferredBytes();
                if (circlesShowAllActiveTransfersProgress)
                {
                    ui->bTransferManager->setPercentDownloads(computeRatio(completedDownloadBytes, leftDownloadBytes));
                }
            }

            if (remainingDownloads <= 0)
            {
                leftDownloadBytes = 0;
                completedDownloadBytes = 0;
                ui->bTransferManager->setPercentDownloads(0.0);
            }
        }
        else if (transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            if (uploadActiveTransferTag == transfer->getTag())
            {
                uploadActiveTransferTag = -1;
            }

            if (e->getErrorCode() != MegaError::API_OK)
            {
                leftUploadBytes -= transfer->getTotalBytes();
                completedUploadBytes -= transfer->getTransferredBytes();
                if (circlesShowAllActiveTransfersProgress)
                {
                    ui->bTransferManager->setPercentUploads(computeRatio(completedUploadBytes, leftUploadBytes));
                }
            }

            if (remainingUploads <= 0)
            {
                leftUploadBytes = 0;
                completedUploadBytes = 0;
                ui->bTransferManager->setPercentUploads(0.0);
            }
        }
    }
}

void InfoDialog::onTransferStart(MegaApi*, MegaTransfer* transfer)
{
    updateTransfersCount();
    if (transfer)
    {
        if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
        {
            leftDownloadBytes += transfer->getTotalBytes();
        }
        else if (transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            leftUploadBytes += transfer->getTotalBytes();
        }
    }
}

void InfoDialog::onTransferUpdate(MegaApi*, MegaTransfer* transfer)
{
    if (transfer)
    {
        if (transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
        {
            completedDownloadBytes += transfer->getDeltaSize();
            currentDownloadBytes = transfer->getTotalBytes();
            currentCompletedDownloadBytes = transfer->getTransferredBytes();
            if (circlesShowAllActiveTransfersProgress)
            {
                ui->bTransferManager->setPercentDownloads(computeRatio(completedDownloadBytes, leftDownloadBytes));
            }
            else
            {
                if (downloadActiveTransferTag == -1 || transfer->getPriority() <= downloadActiveTransferPriority
                        || downloadActiveTransferState == MegaTransfer::STATE_PAUSED)
                {
                    downloadActiveTransferTag = transfer->getTag();
                }
                if (downloadActiveTransferTag == transfer->getTag())
                {
                    downloadActiveTransferPriority = transfer->getPriority();
                    downloadActiveTransferState = transfer->getState();
                    ui->bTransferManager->setPercentDownloads(computeRatio(currentCompletedDownloadBytes, currentDownloadBytes));
                }
            }
        }
        else if (transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            completedUploadBytes += transfer->getDeltaSize();
            currentUploadBytes = transfer->getTotalBytes();
            currentCompletedUploadBytes = transfer->getTransferredBytes();
            if (circlesShowAllActiveTransfersProgress)
            {
                ui->bTransferManager->setPercentUploads(computeRatio(completedUploadBytes, leftUploadBytes));
            }
            else
            {
                if (uploadActiveTransferTag == -1 || transfer->getPriority() <= uploadActiveTransferPriority
                         || uploadActiveTransferState == MegaTransfer::STATE_PAUSED)
                {
                    uploadActiveTransferTag = transfer->getTag();
                }
                if (uploadActiveTransferTag == transfer->getTag())
                {
                    uploadActiveTransferPriority = transfer->getPriority();
                    uploadActiveTransferState = transfer->getState();
                    ui->bTransferManager->setPercentUploads(computeRatio(currentCompletedUploadBytes, currentUploadBytes));
                }
            }
        }
    }
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
    if (obj == ui->wStorageUsage && e->type() == QEvent::MouseButtonPress)
    {
        on_bStorageDetails_clicked();
        return true;
    }

#ifdef Q_OS_LINUX
    static bool firstime = true;
    if (getenv("START_MEGASYNC_MINIMIZED") && firstime && (obj == this && e->type() == QEvent::Paint))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Minimizing info dialog (reason: %1)...").arg(e->type()).toUtf8().constData());
        showMinimized();
        firstime = false;
    }

    if (doNotActAsPopup)
    {
        if (obj == this && e->type() == QEvent::Close)
        {
            e->ignore(); //This prevents the dialog from closing
            app->exitApplication();
            return true;
        }
    }
    else if (obj == this)
    {
        static bool in = false;
        if (e->type() == QEvent::Enter)
        {
            in = true;
        }
        if (e->type() == QEvent::Leave)
        {
            in = false;
        }
        if (e->type() == QEvent::WindowDeactivate && !in)
        {
            hide();
            return true;
        }
    }

#endif
#ifdef __APPLE__
    auto current = QOperatingSystemVersion::current();
    if (current <= QOperatingSystemVersion::OSXMavericks) //manage spontaneus mouse press events
    {
        if (obj == this && e->type() == QEvent::MouseButtonPress && e->spontaneous())
        {
            return true;
        }
    }
#endif

    return QDialog::eventFilter(obj, e);
}

void InfoDialog::on_bStorageDetails_clicked()
{
    if (accountDetailsDialog)
    {
        accountDetailsDialog->raise();
        return;
    }

    accountDetailsDialog = new AccountDetailsDialog(this);
    app->updateUserStats(true, true, true, true, USERSTATS_STORAGECLICKED);
    QPointer<AccountDetailsDialog> dialog = accountDetailsDialog;
    dialog->exec();
    if (!dialog)
    {
        return;
    }

    delete accountDetailsDialog;
    accountDetailsDialog = NULL;
}

void InfoDialog::regenerateLayout(int blockState, InfoDialog* olddialog)
{
    int actualAccountState;

    blockState ? actualAccountState = blockState
                  : preferences->logged() ? actualAccountState = STATE_LOGGEDIN
                                          : actualAccountState = STATE_LOGOUT;

    if (actualAccountState == loggedInMode)
    {
        return;
    }

    loggedInMode = actualAccountState;

    QLayout *dialogLayout = layout();
    switch(loggedInMode)
    {
        case STATE_LOGOUT:
        case STATE_LOCKED_EMAIL:
        case STATE_LOCKED_SMS:
        {
            if (!gWidget)
            {
                gWidget = new GuestWidget();

                connect(gWidget, SIGNAL(onPageLogin()), this, SLOT(resetLoggedInMode()));
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

            gWidget->setBlockState(blockState);

            updateOverStorageState(Preferences::STATE_BELOW_OVER_STORAGE);
            setOverQuotaMode(false);
            ui->wPSA->removeAnnounce();

            dialogLayout->removeWidget(ui->wInfoDialogIn);
            ui->wInfoDialogIn->setVisible(false);
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
            #endif

            adjustSize();
            break;
        }

        case STATE_LOGGEDIN:
        {
            if (gWidget)
            {
                gWidget->disableListener();
                gWidget->initialize();

                dialogLayout->removeWidget(gWidget);
                gWidget->setVisible(false);
            }
            dialogLayout->addWidget(ui->wInfoDialogIn);
            ui->wInfoDialogIn->setVisible(true);

            #ifdef __APPLE__
                if (dummy)
                {
                    dummy->hide();
                    delete dummy;
                    dummy = NULL;
                }
            #endif

            adjustSize();
            break;
        }
    }
    app->repositionInfoDialog();

    app->onGlobalSyncStateChanged(NULL);
}

void InfoDialog::drawAvatar(QString email)
{
    ui->bAvatar->drawAvatarFromEmail(email);
}

void InfoDialog::animateStates(bool opt)
{
    if (opt) //Enable animation for scanning/waiting states
    {        
        ui->lUploadToMega->setIcon(Utilities::getCachedPixmap(QString::fromUtf8("://images/init_scanning.png")));
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
        ui->lUploadToMega->setIcon(Utilities::getCachedPixmap(QString::fromUtf8("://images/upload_to_mega.png")));
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

void InfoDialog::resetLoggedInMode()
{
    loggedInMode = STATE_NONE;
}

void InfoDialog::on_tTransfers_clicked()
{
    ui->lTransfers->setStyleSheet(QString::fromUtf8("background-color: #3C434D;"));
    ui->lRecents->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tTransfers->setStyleSheet(QString::fromUtf8("color : #1D1D1D;"));
    ui->tNotifications->setStyleSheet(QString::fromUtf8("color : #989899;"));

    ui->sTabs->setCurrentWidget(ui->pTransfersTab);
}

void InfoDialog::on_tNotifications_clicked()
{
    ui->lTransfers->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lRecents->setStyleSheet(QString::fromUtf8("background-color: #3C434D;"));

    ui->tNotifications->setStyleSheet(QString::fromUtf8("color : #1D1D1D;"));
    ui->tTransfers->setStyleSheet(QString::fromUtf8("color : #989899;"));

    ui->sTabs->setCurrentWidget(ui->pNotificationsTab);
}

void InfoDialog::on_bActualFilter_clicked()
{
    if (!notificationsReady || !filterMenu)
    {
        return;
    }

    QPoint p = ui->wFilterAndSettings->mapToGlobal(QPoint(4, 4));
    filterMenu->move(p);
    filterMenu->show();
}

void InfoDialog::applyFilterOption(int opt)
{
    if (filterMenu && filterMenu->isVisible())
    {
        filterMenu->hide();
    }

    switch (opt)
    {
        case QFilterAlertsModel::FILTER_CONTACTS:
        {
            ui->wSortNotifications->setActualFilter(AlertFilterType::TYPE_CONTACTS);

            if (app->hasNotificationsOfType(QAlertsModel::ALERT_CONTACTS))
            {
                ui->sNotifications->setCurrentWidget(ui->pNotifications);
            }
            else
            {
                ui->lNoNotifications->setText(tr("No notifications for contacts"));
                ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
            }

            break;
        }
        case QFilterAlertsModel::FILTER_SHARES:
        {
            ui->wSortNotifications->setActualFilter(AlertFilterType::TYPE_SHARES);

            if (app->hasNotificationsOfType(QAlertsModel::ALERT_SHARES))
            {
                ui->sNotifications->setCurrentWidget(ui->pNotifications);
            }
            else
            {
                ui->lNoNotifications->setText(tr("No notifications for incoming shares"));
                ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
            }

            break;
        }
        case QFilterAlertsModel::FILTER_PAYMENT:
        {
            ui->wSortNotifications->setActualFilter(AlertFilterType::TYPE_PAYMENTS);

            if (app->hasNotificationsOfType(QAlertsModel::ALERT_PAYMENT))
            {
                ui->sNotifications->setCurrentWidget(ui->pNotifications);
            }
            else
            {
                ui->lNoNotifications->setText(tr("No notifications for payments"));
                ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
            }
            break;
        }
        default:
        {
            ui->wSortNotifications->setActualFilter(AlertFilterType::ALL_TYPES);

            if (app->hasNotifications())
            {
                ui->sNotifications->setCurrentWidget(ui->pNotifications);
            }
            else
            {
                ui->lNoNotifications->setText(tr("No notifications"));
                ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
            }
            break;
        }
    }

    app->applyNotificationFilter(opt);
}

void InfoDialog::on_bNotificationsSettings_clicked()
{
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(QString::fromUtf8("mega://#fm/account/notifications")));
}

void InfoDialog::on_bDiscard_clicked()
{
    if(transferQuotaState == QuotaState::FULL)
    {
        transferOverquotaAlertEnabled = false;
        emit transferOverquotaMsgVisibilityChange(transferOverquotaAlertEnabled);
    }
    else if(transferQuotaState == QuotaState::WARNING)
    {
        transferAlmostOverquotaAlertEnabled = false;
        emit almostTransferOverquotaMsgVisibilityChange(transferAlmostOverquotaAlertEnabled);
    }

    if(storageState == Preferences::STATE_ALMOST_OVER_STORAGE ||
            storageState == Preferences::STATE_OVER_STORAGE)
    {
        updateOverStorageState(Preferences::STATE_OVER_STORAGE_DISMISSED);
        emit dismissStorageOverquota(overQuotaState);
    }
    else
    {
        updateDialogState();
    }
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

void InfoDialog::sTabsChanged(int tab)
{
    static int lasttab = -1;
    if (tab != ui->sTabs->indexOf(ui->pNotificationsTab))
    {
        if (lasttab == ui->sTabs->indexOf(ui->pNotificationsTab))
        {
            if (app->hasNotifications() && !app->notificationsAreFiltered())
            {
                megaApi->acknowledgeUserAlerts();
            }
        }
    }
    lasttab = tab;
}



void InfoDialog::hideBlockedError(bool animated)
{
    if (!shownBlockedError)
    {
        return;
    }
    shownBlockedError = false;
    minHeightAnimationBlockedError->setTargetObject(ui->wBlocked);
    maxHeightAnimationBlockedError->setTargetObject(ui->wBlocked);
    minHeightAnimationBlockedError->setPropertyName("minimumHeight");
    maxHeightAnimationBlockedError->setPropertyName("maximumHeight");
    minHeightAnimationBlockedError->setStartValue(30);
    maxHeightAnimationBlockedError->setStartValue(30);
    minHeightAnimationBlockedError->setEndValue(0);
    maxHeightAnimationBlockedError->setEndValue(0);
    minHeightAnimationBlockedError->setDuration(animated ? 250 : 1);
    maxHeightAnimationBlockedError->setDuration(animated ? 250 : 1);
    animationGroupBlockedError.start();
    ui->wBlocked->show();
}

void InfoDialog::showBlockedError()
{
    if (shownBlockedError)
    {
        return;
    }

    ui->wBlocked->show();
    minHeightAnimationBlockedError->setTargetObject(ui->wBlocked);
    maxHeightAnimationBlockedError->setTargetObject(ui->wBlocked);
    minHeightAnimationBlockedError->setPropertyName("minimumHeight");
    maxHeightAnimationBlockedError->setPropertyName("maximumHeight");
    minHeightAnimationBlockedError->setStartValue(0);
    maxHeightAnimationBlockedError->setStartValue(0);
    minHeightAnimationBlockedError->setEndValue(30);
    maxHeightAnimationBlockedError->setEndValue(30);
    minHeightAnimationBlockedError->setDuration(250);
    maxHeightAnimationBlockedError->setDuration(250);
    animationGroupBlockedError.start();
    shownBlockedError = true;
}

void InfoDialog::onAnimationFinishedBlockedError()
{
    ui->wBlocked->setVisible(shownBlockedError);
}

void InfoDialog::on_bDismissSyncSettings_clicked()
{
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
}

void InfoDialog::on_bOpenSyncSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
}

void InfoDialog::on_bDismissBackupsSettings_clicked()
{
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

void InfoDialog::on_bOpenBackupsSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::BACKUP_TAB);
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

void InfoDialog::on_bDismissAllSyncsSettings_clicked()
{
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

void InfoDialog::on_bOpenAllSyncsSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
    model->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

int InfoDialog::getLoggedInMode() const
{
    return loggedInMode;
}

void InfoDialog::showNotifications()
{
    on_tNotifications_clicked();
}

void InfoDialog::setBlockedStateLabel(QString state)
{
    if (state.isEmpty())
    {
        hideBlockedError(true);
    }
    else
    {
        showBlockedError();
    }

    ui->lSDKblock->setText(state);
}

long long InfoDialog::getUnseenNotifications() const
{
    return unseenNotifications;
}

void InfoDialog::setUnseenNotifications(long long value)
{
    assert(value >= 0);
    unseenNotifications = value > 0 ? value : 0;
    if (!unseenNotifications)
    {
        ui->bNumberUnseenNotifications->hide();
        return;
    }
    ui->bNumberUnseenNotifications->setText(QString::number(unseenNotifications));
    ui->bNumberUnseenNotifications->show();
}

void InfoDialog::setUnseenTypeNotifications(long long all, long long contacts, long long shares, long long payment)
{
    filterMenu->setUnseenNotifications(all, contacts, shares, payment);
}

void InfoDialog::paintEvent(QPaintEvent * e)
{
    app->megaApiLock.reset(app->getMegaApi()->getMegaApiLock(false));
    QDialog::paintEvent(e);
    app->megaApiLock.reset();

#ifdef __APPLE__
    QPainter p(this);
    p.setCompositionMode(QPainter::CompositionMode_Clear);
    p.fillRect(ui->wArrow->rect(), Qt::transparent);
#endif
}

double InfoDialog::computeRatio(long long completed, long long remaining)
{
    return static_cast<double>(completed) / static_cast<double>(remaining);
}
