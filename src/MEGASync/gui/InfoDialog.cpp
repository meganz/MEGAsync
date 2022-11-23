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
#include "TransferManager.h"
#include "MenuItemAction.h"
#include "StalledIssuesModel.h"
#include "platform/Platform.h"
#include "assert.h"
#include "syncs/gui/Backups/BackupsWizard.h"
#include "QMegaMessageBox.h"
#include "TextDecorator.h"

#ifdef _WIN32
#include <chrono>
using namespace std::chrono;
#endif

#include <QtConcurrent/QtConcurrent>

using namespace mega;

static constexpr int DEFAULT_MIN_PERCENTAGE{1};

void InfoDialog::pauseResumeClicked()
{
    app->pauseTransfers();
}

void InfoDialog::generalAreaClicked()
{
    app->transferManagerActionClicked(TransfersWidget::ALL_TRANSFERS_TAB);
}

void InfoDialog::dlAreaClicked()
{
    app->transferManagerActionClicked(TransfersWidget::DOWNLOADS_TAB);
}

void InfoDialog::upAreaClicked()
{
    app->transferManagerActionClicked(TransfersWidget::UPLOADS_TAB);
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

void InfoDialog::showSyncProblems(QString problemText)
{
    QMessageBox qmb;
    qmb.setText(QString::fromUtf8("One or more syncs have encountered issues that need user intervention to resolve.  Please see the details for each one, and adjust either the local or cloud files/folders, or add ignore rules, to resolve these."));
    qmb.setDetailedText(problemText);
    qmb.setMaximumHeight(3000);
    qmb.setMaximumWidth(3000);
    qmb.setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
    qmb.setSizeGripEnabled(true);
    qmb.exec();
}


InfoDialog::InfoDialog(MegaApplication *app, QWidget *parent, InfoDialog* olddialog) :
    QDialog(parent),
    ui(new Ui::InfoDialog),
    mIndexing (false),
    mWaiting (false),
    mSyncing (false),
    mTransferring (false),
    mTransferManager(nullptr),
    mBackupsWizard (nullptr),
    mAddBackupDialog (nullptr),
    mAddSyncDialog (nullptr),
    mPreferences (Preferences::instance()),
    mSyncInfo (SyncInfo::instance()),
    mSyncController (nullptr),
    qtBugFixer(this)
{
    ui->setupUi(this);

    mSyncsMenus[ui->bAddSync] = nullptr;
    mSyncsMenus[ui->bAddBackup] = nullptr;

    filterMenu = new FilterAlertWidget(this);
    connect(filterMenu, SIGNAL(onFilterClicked(int)), this, SLOT(applyFilterOption(int)));

    connect(this, SIGNAL(triggerShowSyncProblems(QString)),this, SLOT(showSyncProblems(QString)), Qt::QueuedConnection);

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

    connect(ui->wSortNotifications, SIGNAL(clicked()), this, SLOT(onActualFilterClicked()));

    connect(app->getTransfersModel(), &TransfersModel::transfersCountUpdated, this, &InfoDialog::updateTransfersCount);
    connect(app->getTransfersModel(), &TransfersModel::transfersProcessChanged, this, &InfoDialog::onTransfersStateChanged);
    connect(app->getTransfersModel(), &TransfersModel::showInFolderFinished, this, &InfoDialog::onShowInFolderFinished);

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

    cloudItem = NULL;
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

    hideSomeIssues();

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

    mState = StatusInfo::TRANSFERS_STATES::STATE_STARTING;
    ui->wStatus->setState(mState);

    megaApi = app->getMegaApi();

    actualAccountType = -1;

    connect(mSyncInfo, SIGNAL(syncDisabledListUpdated()), this, SLOT(updateDialogState()));

    connect(ui->wPSA, SIGNAL(PSAseen(int)), app, SLOT(PSAseen(int)), Qt::QueuedConnection);

    connect(ui->sTabs, SIGNAL(currentChanged(int)), this, SLOT(sTabsChanged(int)), Qt::QueuedConnection);

    on_tTransfers_clicked();

    ui->wListTransfers->setupTransfers();

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

    if (mPreferences->logged())
    {
        setAvatar();
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

    adjustSize();

    mTransferScanCancelUi = new TransferScanCancelUi(ui->sTabs, ui->pTransfersTab);
    connect(mTransferScanCancelUi, &TransferScanCancelUi::cancelTransfers,
            this, &InfoDialog::cancelScanning);

    mResetTransferSummaryWidget.setInterval(2000);
    mResetTransferSummaryWidget.setSingleShot(true);
    connect(&mResetTransferSummaryWidget, &QTimer::timeout, this, &InfoDialog::onResetTransfersSummaryWidget);
}

InfoDialog::~InfoDialog()
{
    removeEventFilter(this);
    delete mBackupsWizard;
    delete mAddBackupDialog;
    delete ui;
    delete gWidget;
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
    mTransferScanCancelUi->update();
}

void InfoDialog::moveEvent(QMoveEvent*)
{
    qtBugFixer.onEndMove();
}

void InfoDialog::setBandwidthOverquotaState(QuotaState state)
{
    transferQuotaState = state;
    setUsage();
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
    ui->bAvatar->setUserEmail(mPreferences->email().toUtf8().constData());
}

void InfoDialog::setUsage()
{
    auto accType = mPreferences->accountType();

    // ---------- Process storage usage
    QString usedStorageString;

    auto totalStorage(mPreferences->totalStorage());
    auto usedStorage(mPreferences->usedStorage());

    if (Utilities::isBusinessAccount())
    {
        ui->sStorage->setCurrentWidget(ui->wBusinessStorage);
        ui->wCircularStorage->setValue(0);
        usedStorageString = QString::fromUtf8("<span style='color: #333333; font-size:20px;"
                                              "font-family: Lato; text-decoration:none;'>%1</span>")
                                     .arg(Utilities::getSizeString(mPreferences->usedStorage()));
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
            switch (mPreferences->getStorageState())
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

    auto usedTransfer(mPreferences->usedBandwidth());

    if (Utilities::isBusinessAccount())
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
            auto totalTransfer (mPreferences->totalBandwidth());
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

void InfoDialog::updateTransfersCount()
{
    if(app->getTransfersModel())
    {
        auto transfersCountUpdated = app->getTransfersModel()->getLastTransfersCount();

        ui->bTransferManager->setDownloads(transfersCountUpdated.completedDownloads(), transfersCountUpdated.totalDownloads);
        ui->bTransferManager->setUploads(transfersCountUpdated.completedUploads(), transfersCountUpdated.totalUploads);

        ui->bTransferManager->setPercentUploads(transfersCountUpdated.completedUploadBytes, transfersCountUpdated.totalUploadBytes);
        ui->bTransferManager->setPercentDownloads(transfersCountUpdated.completedDownloadBytes, transfersCountUpdated.totalDownloadBytes);
    }
}

void InfoDialog::onTransfersStateChanged()
{
    if(app->getTransfersModel())
    {
        auto transfersCountUpdated = app->getTransfersModel()->getLastTransfersCount();

        if(transfersCountUpdated.pendingTransfers() == 0)
        {
            if (!overQuotaState && (ui->sActiveTransfers->currentWidget() != ui->pUpdated))
            {
                updateDialogState();
            }

            mResetTransferSummaryWidget.start();
        }
        else
        {
            mResetTransferSummaryWidget.stop();
        }

        ui->wStatus->update();
    }
}

void InfoDialog::onShowInFolderFinished(bool state)
{
    if(!state)
    {
        QMegaMessageBox::warning(nullptr, tr("Error"), tr("Folder can't be opened. Check that the folder in your local drive hasn't been deleted or moved."), QMessageBox::Ok);
    }
}

void InfoDialog::onResetTransfersSummaryWidget()
{
    ui->bTransferManager->reset();
}

void InfoDialog::setIndexing(bool indexing)
{
    mIndexing = indexing;
}

void InfoDialog::setWaiting(bool waiting)
{
    mWaiting = waiting;
}

void InfoDialog::setSyncing(bool syncing)
{
    mSyncing = syncing;
}

void InfoDialog::setTransferring(bool transferring)
{
    mTransferring = transferring;
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
    if (Utilities::isBusinessAccount())
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
    if (!mPreferences->logged())
    {
        return;
    }

    if (megaApi->isSyncStalled())
    {
        showSomeIssues();
    }
    else
    {
        hideSomeIssues();
    }

}

void InfoDialog::updateState()
{
    if (!mPreferences->logged())
    {
        if (gWidget)
        {
            gWidget->resetFocus();
        }
    }

    if (!mPreferences->logged())
    {
        return;
    }

    auto failState = checkFailedState();
    if (mTransferScanCancelUi && mTransferScanCancelUi->isActive())
    {
        changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_INDEXING);
    }
    else if (mPreferences->getGlobalPaused())
    {
        if(!failState)
        {
            mState = StatusInfo::TRANSFERS_STATES::STATE_PAUSED;
            animateStates(mWaiting || mIndexing || mSyncing);
        }
    }
    else
    {
        if (mIndexing)
        {
            changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_INDEXING);
        }
        else if (mSyncing)
        {
            changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_SYNCING);
        }
        else if (mWaiting)
        {
            changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_WAITING);
        }
        else if (mTransferring)
        {
            changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_TRANSFERRING);
        }
        else
        {
            if(!checkFailedState())
            {
                changeStatusState(StatusInfo::TRANSFERS_STATES::STATE_UPDATED, false);
            }
        }
    }

    if(ui->wStatus->getState() != mState)
    {
        ui->wStatus->setState(mState);
        ui->bTransferManager->setPaused(mPreferences->getGlobalPaused());
        if(mTransferManager)
        {
            mTransferManager->setTransferState(mState);
        }
    }
}

bool InfoDialog::checkFailedState()
{
    auto isFailed(false);

    if((app->getTransfersModel() && app->getTransfersModel()->failedTransfers()) 
    || (app->getStalledIssuesModel() && app->getStalledIssuesModel()->hasStalledIssues()))
    {
        if(mState != StatusInfo::TRANSFERS_STATES::STATE_FAILED)
        {
            mState = StatusInfo::TRANSFERS_STATES::STATE_FAILED;
            animateStates(false);
        }

        isFailed = true;
    }

    return isFailed;
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

void InfoDialog::updateDialogState()
{
    updateState();
    const bool transferOverQuotaEnabled{transferQuotaState == QuotaState::FULL &&
                transferOverquotaAlertEnabled};

    if (storageState == Preferences::STATE_PAYWALL)
    {
        MegaIntegerList* tsWarnings = megaApi->getOverquotaWarningsTs();
        const char *email = megaApi->getMyEmail();

        long long numFiles{mPreferences->cloudDriveFiles() + mPreferences->vaultFiles() + mPreferences->rubbishFiles()};
        QString contactMessage = tr("We have contacted you by email to [A] on [B] but you still have %n file taking up [D] in your MEGA account, which requires you to have [E].", "", static_cast<int>(numFiles));
        QString overDiskText = QString::fromUtf8("<p style='line-height: 20px;'>") + contactMessage
                .replace(QString::fromUtf8("[A]"), QString::fromUtf8(email))
                .replace(QString::fromUtf8("[B]"), Utilities::getReadableStringFromTs(tsWarnings))
                .replace(QString::fromUtf8("[D]"), Utilities::getSizeString(mPreferences->usedStorage()))
                .replace(QString::fromUtf8("[E]"), Utilities::minProPlanNeeded(MegaSyncApp->getPricing(), mPreferences->usedStorage()))
                + QString::fromUtf8("</p>");
        ui->lOverDiskQuotaLabel->setText(overDiskText);

        int64_t remainDaysOut(0);
        int64_t remainHoursOut(0);
        Utilities::getDaysAndHoursToTimestamp(megaApi->getOverquotaDeadlineTs() * 1000, remainDaysOut, remainHoursOut);
        if (remainDaysOut > 0)
        {
            QString descriptionDays = tr("You have [A]%n day[/A] left to upgrade. After that, your data is subject to deletion.", "", static_cast<int>(remainDaysOut));
            ui->lWarningOverDiskQuota->setText(QString::fromUtf8("<p style='line-height: 20px;'>") + descriptionDays
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style='color: #FF6F00;'>"))
                    .replace(QString::fromUtf8("[/A]"), QString::fromUtf8("</span>"))
                    + QString::fromUtf8("</p>"));
        }
        else if (remainDaysOut == 0 && remainHoursOut > 0)
        {
            QString descriptionHours = tr("You have [A]%n hour[/A] left to upgrade. After that, your data is subject to deletion.", "", static_cast<int>(remainHoursOut));
            ui->lWarningOverDiskQuota->setText(QString::fromUtf8("<p style='line-height: 20px;'>") + descriptionHours
                    .replace(QString::fromUtf8("[A]"), QString::fromUtf8("<span style='color: #FF6F00;'>"))
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
        const bool userIsFree{mPreferences->accountType() == Preferences::Preferences::ACCOUNT_TYPE_FREE};
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
    else if (mSyncInfo->hasUnattendedDisabledSyncs({mega::MegaSync::TYPE_TWOWAY, mega::MegaSync::TYPE_BACKUP}))
    {
        if (mSyncInfo->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY)
            && mSyncInfo->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP))
        {
            ui->sActiveTransfers->setCurrentWidget(ui->pAllSyncsDisabled);
        }
        else if (mSyncInfo->hasUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP))
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
        if(app->getTransfersModel())
        {
            auto transfersCount = app->getTransfersModel()->getTransfersCount();

            if (transfersCount.totalDownloads || transfersCount.totalUploads
                    || ui->wPSA->isPSAready())
            {
                overlay->setVisible(false);
                ui->sActiveTransfers->setCurrentWidget(ui->pTransfers);
                ui->wPSA->showPSA();
            }
            else
            {
                ui->wPSA->hidePSA();
                ui->sActiveTransfers->setCurrentWidget(ui->pUpdated);
                if (!mWaiting && !mIndexing)
                {
                    overlay->setVisible(true);
                }
                else
                {
                    overlay->setVisible(false);
                }
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
    Utilities::openUrl(QUrl(url));
}

void InfoDialog::on_bUpgradeOverDiskQuota_clicked()
{
    on_bUpgrade_clicked();
}

void InfoDialog::openFolder(QString path)
{
    Utilities::openUrl(QUrl::fromLocalFile(path));
}

void InfoDialog::addSync(MegaHandle h)
{
    if (mAddSyncDialog)
    {
        if (h != mega::INVALID_HANDLE)
        {
            mAddSyncDialog->setMegaFolder(h);
        }

        mAddSyncDialog->activateWindow();
        mAddSyncDialog->raise();
        mAddSyncDialog->setFocus();
        return;
    }

    mAddSyncDialog = new BindFolderDialog(app);
    if (h != mega::INVALID_HANDLE)
    {
        mAddSyncDialog->setMegaFolder(h);
    }
    mAddSyncDialog->exec();
    if (!mAddSyncDialog)
    {
        return;
    }

    if (mAddSyncDialog->result() != QDialog::Accepted)
    {
        delete mAddSyncDialog;
        return;
    }

    QString localFolderPath = QDir::toNativeSeparators(QDir(mAddSyncDialog->getLocalFolder()).canonicalPath());
    MegaHandle handle = mAddSyncDialog->getMegaFolder();
    QString syncName = mAddSyncDialog->getSyncName();
    delete mAddSyncDialog;

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromLatin1("Adding sync %1 from addSync: ").arg(localFolderPath).toUtf8().constData());

    setupSyncController();
    mSyncController->addSync(localFolderPath, handle, syncName, mega::MegaSync::TYPE_TWOWAY);
}

void InfoDialog::addBackup()
{
    // If no backups configured: show wizard, else show "add single backup" dialog
    int nbBackups(SyncInfo::instance()->getNumSyncedFolders(mega::MegaSync::TYPE_BACKUP));

    if (nbBackups > 0)
    {
        if (mAddBackupDialog)
        {
            mAddBackupDialog->activateWindow();
            mAddBackupDialog->raise();
            mAddBackupDialog->setFocus();
        }
        else
        {
            setupSyncController();

            mAddBackupDialog = new AddBackupDialog();
            mAddBackupDialog->setAttribute(Qt::WA_DeleteOnClose);
            mAddBackupDialog->setWindowModality(Qt::ApplicationModal);

            mAddBackupDialog->show();

            connect(mAddBackupDialog.data(), &AddBackupDialog::accepted, this, [this]()
            {
                if(mAddBackupDialog)
                {
                    QString dirToBackup (mAddBackupDialog->getSelectedFolder());
                    QString backupName (mAddBackupDialog->getBackupName());
                    mSyncController->addBackup(dirToBackup, backupName);
                }
            });
        }
    }
    else
    {
        if (mBackupsWizard)
        {
            mBackupsWizard->activateWindow();
            mBackupsWizard->raise();
            mBackupsWizard->setFocus();
        }
        else
        {
            mBackupsWizard = new BackupsWizard();
            mBackupsWizard->setAttribute(Qt::WA_DeleteOnClose);
            mBackupsWizard->setWindowModality(Qt::ApplicationModal);
            mBackupsWizard->show();
        }
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
    if (mPreferences->logged())
    {
        auto menu (mSyncsMenus[b]);
        if (!menu)
        {
            menu = createSyncMenu(type, ui->bUpload->isEnabled());
            mSyncsMenus[b] = menu;
        }
        menu->callMenu(b->mapToGlobal(QPoint(b->width() - 100, b->height() + 3)));
    }
}

SyncsMenu* InfoDialog::createSyncMenu(mega::MegaSync::SyncType type, bool isEnabled)
{
    SyncsMenu* menu = new SyncsMenu(type, this);
    connect(menu, &SyncsMenu::addSync, this, &InfoDialog::onAddSync);
    menu->setEnabled(isEnabled);
    return menu;
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
    notificationsReady = false;
    ui->sNotifications->setCurrentWidget(ui->pNoNotifications);
    ui->wSortNotifications->setActualFilter(AlertFilterType::ALL_TYPES);

    ui->bTransferManager->reset();

    hideSomeIssues();
    shownSomeIssuesOccurred = false;

    setUnseenNotifications(0);
    if (filterMenu)
    {
        filterMenu->reset();
    }

    transferOverquotaAlertEnabled = false;
    transferAlmostOverquotaAlertEnabled = false;
    transferQuotaState = QuotaState::OK;

    mSyncController.reset();
}

void InfoDialog::setPSAannouncement(int id, QString title, QString text, QString urlImage, QString textButton, QString linkButton)
{
    ui->wPSA->setAnnounce(id, title, text, urlImage, textButton, linkButton);
}

void InfoDialog::enterBlockingState()
{
    enableUserActions(false);
    ui->bTransferManager->setPauseEnabled(false);
    ui->wTabOptions->setVisible(false);
    mTransferScanCancelUi->show();
    updateState();
}

void InfoDialog::leaveBlockingState(bool fromCancellation)
{
    enableUserActions(true);
    ui->bTransferManager->setPauseEnabled(true);
    ui->wTabOptions->setVisible(true);
    mTransferScanCancelUi->hide(fromCancellation);
    updateState();
}

void InfoDialog::disableCancelling()
{
    mTransferScanCancelUi->disableCancelling();
}

void InfoDialog::setUiInCancellingStage()
{
    mTransferScanCancelUi->setInCancellingStage();
}

void InfoDialog::updateUiOnFolderTransferUpdate(const FolderTransferUpdateEvent &event)
{
    mTransferScanCancelUi->onFolderTransferUpdate(event);
}

void InfoDialog::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);

        if (mPreferences->logged())
        {
            setUsage();
            mState = StatusInfo::TRANSFERS_STATES::STATE_STARTING;
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
            app->tryExitApplication();
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
        else if (e->type() == QEvent::Leave)
        {
            in = false;
        }
        else  if (e->type() == QEvent::WindowDeactivate)
        {
            hide();
            return true;
        }
        else if(e->type() == QEvent::FocusOut)
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
                  : mPreferences->logged() ? actualAccountState = STATE_LOGGEDIN
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

void InfoDialog::onActualFilterClicked()
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
    Utilities::openUrl(QUrl(QString::fromUtf8("mega://#fm/account/notifications")));
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



void InfoDialog::hideSomeIssues()
{
    shownSomeIssuesOccurred = false;
    ui->wSomeIssuesOccurred->hide();
}

void InfoDialog::showSomeIssues()
{
    if (shownSomeIssuesOccurred)
    {
        return;
    }

    ui->wSomeIssuesOccurred->show();
    animationGroupSomeIssues.start();
    shownSomeIssuesOccurred = true;
}

void InfoDialog::on_bDismissSyncSettings_clicked()
{
    mSyncInfo->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
}

void InfoDialog::on_bOpenSyncSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
    mSyncInfo->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_TWOWAY);
}

void InfoDialog::on_bDismissBackupsSettings_clicked()
{
    mSyncInfo->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

void InfoDialog::on_bOpenBackupsSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::BACKUP_TAB);
    mSyncInfo->dismissUnattendedDisabledSyncs(mega::MegaSync::TYPE_BACKUP);
}

void InfoDialog::on_bDismissAllSyncsSettings_clicked()
{
    mSyncInfo->dismissUnattendedDisabledSyncs(SyncInfo::AllHandledSyncTypes);
}

void InfoDialog::on_bOpenAllSyncsSettings_clicked()
{
    MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
    mSyncInfo->dismissUnattendedDisabledSyncs(SyncInfo::AllHandledSyncTypes);
}

int InfoDialog::getLoggedInMode() const
{
    return loggedInMode;
}

void InfoDialog::showNotifications()
{
    on_tNotifications_clicked();
}

void InfoDialog::move(int x, int y)
{
   qtBugFixer.onStartMove();
   QDialog::move(x, y);
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
    QDialog::paintEvent(e);

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

void InfoDialog::enableUserActions(bool value)
{
    ui->bAvatar->setEnabled(value);
    ui->bUpgrade->setEnabled(value);
    ui->bUpload->setEnabled(value);

    // To set the state of the Syncs and Backups button,
    // we have to first create them if they don't exist
    for (auto button : mSyncsMenus.keys())
    {
        auto syncMenu (mSyncsMenus[button]);
        if (!syncMenu)
        {
            auto type (button == ui->bAddSync ? MegaSync::TYPE_TWOWAY : MegaSync::TYPE_BACKUP);
            syncMenu = createSyncMenu(type, value);
            mSyncsMenus[button] = syncMenu;
        }
        syncMenu->setEnabled(value);
        button->setEnabled(syncMenu->getAction()->isEnabled());
    }
}

void InfoDialog::changeStatusState(StatusInfo::TRANSFERS_STATES newState,
                                   bool animate)
{
    if (mState != newState)
    {
        mState = newState;
        animateStates(animate);
    }
}

void InfoDialog::setTransferManager(TransferManager *transferManager)
{
    mTransferManager = transferManager;
    mTransferManager->setTransferState(mState);
}

void InfoDialog::setupSyncController()
{
    if (!mSyncController)
    {
        mSyncController.reset(new SyncController());

        // Connect sync controller signals
        connect(mSyncController.get(), &SyncController::syncAddStatus, this, [](const int errorCode, const QString errorMsg, QString name)
        {
            if (errorCode != MegaError::API_OK)
            {
                QString msg = errorMsg;
                Text::Link link(Utilities::SUPPORT_URL);
                Text::Decorator tc(&link);
                tc.process(msg);
                QMegaMessageBox::warning(nullptr, tr("Error"), tr("Error adding %1:").arg(name)
                                         + QString::fromLatin1("\n")
                                         + msg, QMessageBox::Ok, QMessageBox::NoButton, QMap<QMessageBox::StandardButton, QString>(), Qt::RichText);
            }
        });
    }
}
