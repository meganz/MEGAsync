#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "ui_TransferManagerDragBackDrop.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "MegaTransferDelegate.h"
#include "MegaTransferView.h"
#include "OverQuotaDialog.h"
#include "DialogOpener.h"
#include "StalledIssuesDialog.h"
#include "StalledIssuesModel.h"
#include "Platform.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QPalette>
#include <QStyleOptionFocusRect>
#include <QPainter>

using namespace mega;

const int TransferManager::SPEED_REFRESH_PERIOD_MS;
const int TransferManager::STATS_REFRESH_PERIOD_MS;

const char* ALL_TRANSFERS_TITLE = "All transfers";
const char* UPLOADS_TITLE = "Uploads";
const char* DOWNLOADS_TITLE = "Downloads";
const char* COMPLETED_TITLE = "Completed";
const char* FAILED_TITLE = "Failed";
const char* DOCUMENTS_TITLE = "Documents";
const char* VIDEOS_TITLE = "Videos";
const char* AUDIOS_TITLE = "Audio";
const char* OTHERS_TITLE = "Other";
const char* ARCHIVES_TITLE = "Archives";
const char* IMAGES_TITLE = "Images";

const char* LABEL_NUMBER = "NUMBER";
const char* ITS_ON = "itsOn";
const char* SEARCH_TEXT = "searchText";
const char* SEARCH_BUTTON_SELECTED = "selected";

TransferManager::TransferManager(TransfersWidget::TM_TAB tab, MegaApi *megaApi) :
    QDialog(nullptr),
    mUi(new Ui::TransferManager),
    mMegaApi(megaApi),
    mScanningAnimationIndex(1),
    mPreferences(Preferences::instance()),
    mModel(nullptr),
    mSearchFieldReturnPressed(false),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr)),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this)),
    mUiDragBackDrop(new Ui::TransferManagerDragBackDrop),
    mStorageQuotaState(MegaApi::STORAGE_STATE_UNKNOWN),
    mTransferQuotaState(QuotaState::OK),
    mFoundStalledIssues(false)
{
    mUi->setupUi(this);

    mDragBackDrop = new QWidget(this);
    mUiDragBackDrop->setupUi(mDragBackDrop);
    mDragBackDrop->hide();

    mUi->wTransfers->setupTransfers();

    mModel = mUi->wTransfers->getModel();

    Platform::getInstance()->enableDialogBlur(this);

    mUi->wSearch->hide();
    mUi->wMediaType->hide();
    mUi->fCompleted->hide();

    mUi->sStatus->setCurrentWidget(mUi->pUpToDate);

    QColor shadowColor (188, 188, 188);
    mShadowTab->setParent(mUi->wTransferring);
    mShadowTab->setBlurRadius(10.);
    mShadowTab->setXOffset(0.);
    mShadowTab->setYOffset(0.);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);

    mTabFramesToggleGroup[TransfersWidget::ALL_TRANSFERS_TAB] = mUi->fAllTransfers;
    mTabFramesToggleGroup[TransfersWidget::DOWNLOADS_TAB]     = mUi->fDownloads;
    mTabFramesToggleGroup[TransfersWidget::UPLOADS_TAB]       = mUi->fUploads;
    mTabFramesToggleGroup[TransfersWidget::COMPLETED_TAB]     = mUi->fCompleted;
    mTabFramesToggleGroup[TransfersWidget::FAILED_TAB]        = mUi->fFailed;
    mTabFramesToggleGroup[TransfersWidget::SEARCH_TAB]        = mUi->fSearchString;
    mTabFramesToggleGroup[TransfersWidget::TYPE_OTHER_TAB]    = mUi->fOther;
    mTabFramesToggleGroup[TransfersWidget::TYPE_AUDIO_TAB]    = mUi->fAudio;
    mTabFramesToggleGroup[TransfersWidget::TYPE_VIDEO_TAB]    = mUi->fVideos;
    mTabFramesToggleGroup[TransfersWidget::TYPE_ARCHIVE_TAB]  = mUi->fArchives;
    mTabFramesToggleGroup[TransfersWidget::TYPE_DOCUMENT_TAB] = mUi->fDocuments;
    mTabFramesToggleGroup[TransfersWidget::TYPE_IMAGE_TAB]    = mUi->fImages;

    for (auto tabFrame : qAsConst(mTabFramesToggleGroup))
    {
        tabFrame->setProperty(ITS_ON, false);

        auto pushButton = tabFrame->findChild<QPushButton*>();
        if(pushButton)
        {
            pushButton->setProperty(ButtonIconManager::DISABLE_UNCHECK_ON_CLICK, true);
        }
    }

    mTabNoItem[TransfersWidget::ALL_TRANSFERS_TAB] = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::DOWNLOADS_TAB]     = mUi->wNoDownloads;
    mTabNoItem[TransfersWidget::UPLOADS_TAB]       = mUi->wNoUploads;
    mTabNoItem[TransfersWidget::COMPLETED_TAB]     = mUi->wNoFinished;
    mTabNoItem[TransfersWidget::FAILED_TAB]        = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::SEARCH_TAB]        = mUi->wNoResults;
    mTabNoItem[TransfersWidget::TYPE_OTHER_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::TYPE_AUDIO_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::TYPE_VIDEO_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::TYPE_ARCHIVE_TAB]  = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::TYPE_DOCUMENT_TAB] = mUi->wNoTransfers;
    mTabNoItem[TransfersWidget::TYPE_IMAGE_TAB]    = mUi->wNoTransfers;

    mNumberLabelsGroup[TransfersWidget::ALL_TRANSFERS_TAB]    = mUi->lAllTransfers;
    mNumberLabelsGroup[TransfersWidget::DOWNLOADS_TAB]        = mUi->lDownloads;
    mNumberLabelsGroup[TransfersWidget::UPLOADS_TAB]          = mUi->lUploads;
    mNumberLabelsGroup[TransfersWidget::COMPLETED_TAB]        = mUi->lCompleted;
    mNumberLabelsGroup[TransfersWidget::FAILED_TAB]           = mUi->lFailed;
    mNumberLabelsGroup[TransfersWidget::TYPE_OTHER_TAB]       = mUi->lOtherNb;
    mNumberLabelsGroup[TransfersWidget::TYPE_AUDIO_TAB]       = mUi->lAudioNb;
    mNumberLabelsGroup[TransfersWidget::TYPE_VIDEO_TAB]       = mUi->lVideosNb;
    mNumberLabelsGroup[TransfersWidget::TYPE_ARCHIVE_TAB]     = mUi->lArchivesNb;
    mNumberLabelsGroup[TransfersWidget::TYPE_DOCUMENT_TAB]    = mUi->lDocumentsNb;
    mNumberLabelsGroup[TransfersWidget::TYPE_IMAGE_TAB]       = mUi->lImagesNb;

    QMetaEnum tabs = QMetaEnum::fromType<TransfersWidget::TM_TAB>();

    for (int index = 0; index < tabs.keyCount(); index++)
    {
        auto value  = tabs.value(index);

        if(value > TransfersWidget::TYPES_TAB_BASE && value < TransfersWidget::TYPES_LAST)
        {
            TransfersWidget::TM_TAB currentTab = static_cast<TransfersWidget::TM_TAB>(value);
            mNumberLabelsGroup[currentTab]->parentWidget()->hide();
        }
    }

    auto managedButtons = mUi->wLeftPane->findChildren<QAbstractButton*>();
    foreach(auto& button, managedButtons)
    {
        mButtonIconManager.addButton(button);
    }

    managedButtons = mUi->wRightPaneHeader->findChildren<QAbstractButton*>();
    foreach(auto& button, managedButtons)
    {
        mButtonIconManager.addButton(button);
    }

    connect(mModel, &TransfersModel::pauseStateChanged,
            this, &TransferManager::onUpdatePauseState);

    connect(mModel, &TransfersModel::transfersCountUpdated,
            this, &TransferManager::onTransfersDataUpdated);

    connect(mModel, &TransfersModel::pauseStateChangedByTransferResume,
            this, &TransferManager::onPauseStateChangedByTransferResume);

    connect(this, &TransferManager::retryAllTransfers,
            findChild<MegaTransferView*>(), &MegaTransferView::onRetryVisibleTransfers);

    connect(findChild<MegaTransferView*>(), &MegaTransferView::verticalScrollBarVisibilityChanged,
            this, &TransferManager::onVerticalScrollBarVisibilityChanged);

    connect(mUi->wTransfers->getProxyModel(),
            &TransfersManagerSortFilterProxyModel::searchNumbersChanged,
            this, &TransferManager::refreshSearchStats);

    connect(mUi->wTransfers, &TransfersWidget::pauseResumeVisibleRows,
                this, &TransferManager::onPauseResumeVisibleRows);

    connect(mUi->wTransfers, &TransfersWidget::transferPauseResumeStateChanged,
                this, &TransferManager::showQuotaStorageDialogs);

    connect(mUi->wTransfers, &TransfersWidget::changeToAllTransfersTab,
                this, &TransferManager::on_tAllTransfers_clicked);

    connect(mUi->wTransfers,
            &TransfersWidget::loadingViewVisibilityChanged,[this](bool state)
    {
        refreshView();
    });

    connect(mUi->wTransfers,
            &TransfersWidget::disableTransferManager,[this](bool state)
    {
        setDisabled(state);

        if(!state && mSearchFieldReturnPressed)
        {
            mUi->leSearchField->setFocus();
            mSearchFieldReturnPressed = false;
        }
    });

    mFoundStalledIssues = MegaSyncApp->getStalledIssuesModel()->hasStalledIssues();
    checkContentInfo();
    connect(MegaSyncApp->getStalledIssuesModel(),
            &StalledIssuesModel::globalSyncStateChanged,
            this, &TransferManager::onStalledIssuesStateChanged);

    onStalledIssuesStateChanged(MegaSyncApp->getStalledIssuesModel()->hasStalledIssues());

    mScanningTimer.setInterval(60);
    connect(&mScanningTimer, &QTimer::timeout, this, &TransferManager::onScanningAnimationUpdate);

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    auto sizePolicy = mUi->wDownSpeed->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    mUi->wDownSpeed->setSizePolicy(sizePolicy);

    sizePolicy = mUi->wUpSpeed->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    mUi->wUpSpeed->setSizePolicy(sizePolicy);

    // Connect to storage quota signals
    connect(MegaSyncApp, &MegaApplication::storageStateChanged,
            this, &TransferManager::onStorageStateChanged,
            Qt::QueuedConnection);

    setAcceptDrops(true);

    // Init state
    auto storageState = MegaSyncApp->getAppliedStorageState();
    auto transferQuotaState = MegaSyncApp->getTransferQuotaState();
    onStorageStateChanged(storageState);
    onTransferQuotaStateChanged(transferQuotaState);
    mTransferQuotaTimer.setInterval(1000);
    connect(&mTransferQuotaTimer, &QTimer::timeout, this, &TransferManager::onTransferQuotaExceededUpdate);

    updateCurrentOverQuotaLink();
    onUpdatePauseState(mPreferences->getGlobalPaused());

    toggleTab(tab);

    mTransferScanCancelUi = new TransferScanCancelUi(mUi->sTransfers, mTabNoItem[TransfersWidget::ALL_TRANSFERS_TAB]);
    connect(mTransferScanCancelUi, &TransferScanCancelUi::cancelTransfers,
            this, &TransferManager::cancelScanning);

    mUi->wAllResults->installEventFilter(this);
    mUi->wDlResults->installEventFilter(this);
    mUi->wUlResults->installEventFilter(this);
    mUi->lTransfers->installEventFilter(this);
    mUi->wLeftPane->installEventFilter(this);

    mUi->lTextSearch->installEventFilter(this);
    mUi->leSearchField->installEventFilter(this);

#ifdef Q_OS_MACOS
    mUi->leSearchField->setAttribute(Qt::WA_MacShowFocusRect,0);
#else
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

    setAttribute(Qt::WA_DeleteOnClose, true);
}

TransferManager::~TransferManager()
{
    disconnect(findChild<MegaTransferView*>(), &MegaTransferView::verticalScrollBarVisibilityChanged,
            this, &TransferManager::onVerticalScrollBarVisibilityChanged);

    mShadowTab->deleteLater();
    delete mUi;
    delete mTransferScanCancelUi;
}


void TransferManager::pauseModel(bool value)
{
    mModel->pauseModelProcessing(value);
}

void TransferManager::enterBlockingState()
{
    //First of all, show the transfers widget in case the no items page is visible, as the loading view will be visible
    mUi->wTransfers->setScanningWidgetVisible(true);
    enableUserActions(false);
    mTransferScanCancelUi->show();
    refreshStateStats();

    mScanningTimer.start();
}

void TransferManager::leaveBlockingState(bool fromCancellation)
{
    enableUserActions(true);
    mUi->wTransfers->setScanningWidgetVisible(false);
    mTransferScanCancelUi->hide(fromCancellation);
    refreshStateStats();
    refreshView();

    mScanningTimer.stop();
    mScanningAnimationIndex = 1;
}

void TransferManager::disableCancelling()
{
    mTransferScanCancelUi->disableCancelling();
}

void TransferManager::setUiInCancellingStage()
{
    mTransferScanCancelUi->setInCancellingStage();
}

void TransferManager::onFolderTransferUpdate(const FolderTransferUpdateEvent &event)
{
    mTransferScanCancelUi->onFolderTransferUpdate(event);
}

void TransferManager::onPauseStateChangedByTransferResume()
{
    onUpdatePauseState(false);
}

void TransferManager::updateCurrentSearchText()
{
    auto text = mUi->bSearchString->property(SEARCH_TEXT).toString();
    mUi->bSearchString->setText(text);
    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                              .elidedText(text,
                                          Qt::ElideMiddle,
                                          mUi->lTextSearch->width()-1));
}

void TransferManager::updateCurrentCategoryTitle()
{
    switch (mUi->wTransfers->getCurrentTab())
    {
        case TransfersWidget::UPLOADS_TAB:
             mUi->lCurrentContent->setText(tr(UPLOADS_TITLE));
            return;
        case TransfersWidget::DOWNLOADS_TAB:
             mUi->lCurrentContent->setText(tr(DOWNLOADS_TITLE));
            return;
        case TransfersWidget::COMPLETED_TAB:
            mUi->lCurrentContent->setText(tr(COMPLETED_TITLE));
            return;
        case TransfersWidget::FAILED_TAB:
             mUi->lCurrentContent->setText(tr(FAILED_TITLE));
            return;
        case TransfersWidget::TYPE_DOCUMENT_TAB:
             mUi->lCurrentContent->setText(tr(DOCUMENTS_TITLE));
            return;
        case TransfersWidget::TYPE_VIDEO_TAB:
             mUi->lCurrentContent->setText(tr(VIDEOS_TITLE));
            return;
        case TransfersWidget::TYPE_AUDIO_TAB:
             mUi->lCurrentContent->setText(tr(AUDIOS_TITLE));
            return;
        case TransfersWidget::TYPE_OTHER_TAB:
             mUi->lCurrentContent->setText(tr(OTHERS_TITLE));
            return;
        case TransfersWidget::TYPE_ARCHIVE_TAB:
             mUi->lCurrentContent->setText(tr(ARCHIVES_TITLE));
            return;
        case TransfersWidget::TYPE_IMAGE_TAB:
             mUi->lCurrentContent->setText(tr(IMAGES_TITLE));
             return;
        case TransfersWidget::ALL_TRANSFERS_TAB:
        default:
             mUi->lCurrentContent->setText(tr(ALL_TRANSFERS_TITLE));
    }
}

void TransferManager::updateCurrentOverQuotaLink()
{
    QString moreAboutLink(QLatin1String("<a href=\"https://help.mega.io/plans-storage/space-storage/transfer-quota\"><font color=#333333>%1</font></a>"));
    mUi->lTransferOverQuotaMoreAbout->setText(moreAboutLink.arg(tr("More about transfer quota")));
}

void TransferManager::filterByTab(TransfersWidget::TM_TAB tab)
{
    switch(tab)
    {
        case TransfersWidget::UPLOADS_TAB:
             mUi->wTransfers->filtersChanged(TransferData::TRANSFER_UPLOAD, TransferData::PENDING_STATES_MASK, {});
             return;
        case TransfersWidget::DOWNLOADS_TAB:
             mUi->wTransfers->filtersChanged((TransferData::TRANSFER_DOWNLOAD
                                         | TransferData::TRANSFER_LTCPDOWNLOAD),
                                        TransferData::PENDING_STATES_MASK, {});
             return;
        case TransfersWidget::COMPLETED_TAB:
             mUi->wTransfers->filtersChanged({}, TransferData::TRANSFER_COMPLETED, {});
             return;
        case TransfersWidget::FAILED_TAB:
             mUi->wTransfers->filtersChanged({}, TransferData::TRANSFER_FAILED, {});
            return;
        case TransfersWidget::TYPE_DOCUMENT_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_DOCUMENT});
            return;
        case TransfersWidget::TYPE_VIDEO_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_VIDEO});
            return;
        case TransfersWidget::TYPE_AUDIO_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_AUDIO});
            return;
        case TransfersWidget::TYPE_OTHER_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_OTHER});
            return;
        case TransfersWidget::TYPE_ARCHIVE_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_ARCHIVE});
            return;
        case TransfersWidget::TYPE_IMAGE_TAB:
             mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_IMAGE});
             return;
        case TransfersWidget::SEARCH_TAB:
             return;
        case TransfersWidget::ALL_TRANSFERS_TAB:
        default:
             mUi->wTransfers->filtersChanged({}, TransferData::PENDING_STATES_MASK, {});
    }
}

void TransferManager::on_tCompleted_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::COMPLETED_TAB)
    {
        emit userActivity();
        toggleTab(TransfersWidget::COMPLETED_TAB);
    }
}

void TransferManager::on_tDownloads_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::DOWNLOADS_TAB)
    {
        emit userActivity();
        toggleTab(TransfersWidget::DOWNLOADS_TAB);
    }
}

void TransferManager::on_tUploads_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::UPLOADS_TAB)
    {
        emit userActivity();
        toggleTab(TransfersWidget::UPLOADS_TAB);
    }
}

void TransferManager::on_tAllTransfers_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::ALL_TRANSFERS_TAB)
    {
        emit userActivity();
        toggleTab(TransfersWidget::ALL_TRANSFERS_TAB);
    }
}

void TransferManager::on_tFailed_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::FAILED_TAB)
    {
        emit userActivity();
        checkContentInfo();
        toggleTab(TransfersWidget::FAILED_TAB);
    }
}

void TransferManager::onUpdatePauseState(bool isPaused)
{
    if (isPaused)
    {
        mUi->bPause->setToolTip(tr("Resume all transfers"));

        if(!mUi->bPause->isChecked())
        {
           mUi->bPause->blockSignals(true);
           mUi->bPause->setChecked(true);
           mUi->bPause->blockSignals(false);
        }
    }
    else
    {
        mUi->bPause->setToolTip(tr("Pause all transfers"));

        if(mUi->bPause->isChecked())
        {
            mUi->bPause->blockSignals(true);
            mUi->bPause->setChecked(false);
            mUi->bPause->blockSignals(false);
        }
    }

    checkPauseButtonVisibilityIfPossible();
}

void TransferManager::onPauseResumeVisibleRows(bool isPaused)
{
    showQuotaStorageDialogs(isPaused);

    auto transfersView = findChild<MegaTransferView*>();
    if(mUi->wTransfers->getCurrentTab() == TransfersWidget::ALL_TRANSFERS_TAB)
    {
        mModel->pauseResumeAllTransfers(isPaused);
        onUpdatePauseState(isPaused);
    }
    else
    {
        if(transfersView)
        {
            transfersView->onPauseResumeVisibleRows(isPaused);
        }
    }

    if(transfersView)
    {
        //Use to repaint and update the transfers state
        transfersView->update();
    }
}

void TransferManager::showQuotaStorageDialogs(bool isPaused)
{
    if(!isPaused && hasOverQuotaErrors())
    {
        if(mStorageQuotaState == MegaApi::STORAGE_STATE_PAYWALL
          || mStorageQuotaState == MegaApi::STORAGE_STATE_RED)
        {
            MegaSyncApp->checkOverStorageStates();
        }
        else
        {
            MegaSyncApp->checkOverQuotaStates();
        }
    }
}

void TransferManager::refreshStateStats()
{
    QLabel* countLabel (nullptr);
    QString countLabelText;
    long long processedNumber (0LL);

    // First check Finished states -----------------------------------------------------------------
    countLabel = mNumberLabelsGroup[TransfersWidget::COMPLETED_TAB];

    processedNumber = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
    countLabelText = processedNumber > 0 ? QString::number(processedNumber) : QString();

    // Update if the value changed
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        if (mUi->wTransfers->getCurrentTab() != TransfersWidget::COMPLETED_TAB && processedNumber == 0)
        {
            countLabel->parentWidget()->hide();
            countLabel->clear();
        }
        else
        {
            countLabel->parentWidget()->show();
            countLabel->setVisible(processedNumber > 0);
            countLabel->setText(countLabelText);
        }
    }

    // The check Failed states -----------------------------------------------------------------
    countLabel = mNumberLabelsGroup[TransfersWidget::FAILED_TAB];

    long long failedNumber(mTransfersCount.totalFailedTransfers());
    countLabelText = failedNumber > 0 ? QString::number(failedNumber) : QString();

    // Update if the value changed
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        if (mUi->wTransfers->getCurrentTab() != TransfersWidget::FAILED_TAB && failedNumber == 0)
        {
            countLabel->parentWidget()->hide();
            countLabel->clear();
        }
        else
        {
            countLabel->parentWidget()->show();
            countLabel->setVisible(failedNumber > 0);
            countLabel->setText(countLabelText);
        }
    }

    // Then Active states --------------------------------------------------------------------------
    countLabel = mNumberLabelsGroup[TransfersWidget::ALL_TRANSFERS_TAB];

    processedNumber = mTransfersCount.pendingDownloads + mTransfersCount.pendingUploads;
    countLabelText = processedNumber > 0 ? QString::number(processedNumber) : QString();

    QWidget* leftFooterWidget (nullptr);

    // If we don't have transfers, stop refresh timer and show "Up to date",
    // and if current tab is ALL TRANSFERS, show empty.
    if (processedNumber == 0 && failedNumber == 0 && !MegaSyncApp->getStalledIssuesModel()->hasStalledIssues())
    {
        if(mTransferScanCancelUi && mTransferScanCancelUi->isActive())
        {
            leftFooterWidget = mUi->pScanning;
        }
        else
        {
            leftFooterWidget = mUi->pUpToDate;
        }

        mSpeedRefreshTimer->stop();
        countLabel->hide();
        countLabel->clear();
    }
    else
    {
        // If we didn't have transfers, launch timer and show speed.
        if (countLabel->text().isEmpty())
        {
            if(!mSpeedRefreshTimer->isActive())
            {
                mSpeedRefreshTimer->start(SPEED_REFRESH_PERIOD_MS);
            }
        }

        if(mTransferScanCancelUi && mTransferScanCancelUi->isActive())
        {
            leftFooterWidget = mUi->pScanning;
        }
        else if(failedNumber != 0 || MegaSyncApp->getStalledIssuesModel()->hasStalledIssues())
        {
            leftFooterWidget = mUi->pSomeIssues;
            mUi->bSomeIssues->setText(tr("Issue found", "", failedNumber));
        }
        else if(processedNumber != 0)
        {
            leftFooterWidget = mUi->pSpeedAndClear;
        }

        if(processedNumber != 0)
        {
            countLabel->show();
            countLabel->setText(countLabelText);
        }
        else
        {
            countLabel->hide();
            countLabel->clear();
        }
    }

    if(leftFooterWidget && leftFooterWidget != mUi->sStatus->currentWidget())
    {
        mUi->sStatus->setCurrentWidget(leftFooterWidget);
    }
}

void TransferManager::refreshTypeStats()
{
    auto downloadTransfers = mTransfersCount.pendingDownloads;

    auto countLabel = mNumberLabelsGroup[TransfersWidget::DOWNLOADS_TAB];
    QString countLabelText(downloadTransfers > 0 ? QString::number(downloadTransfers) : QString());

    // First check Downloads -----------------------------------------------------------------------
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        countLabel->setText(countLabelText);
    }

    countLabel->setVisible(!countLabelText.isEmpty());

    auto uploadTransfers = mTransfersCount.pendingUploads;

    countLabel = mNumberLabelsGroup[TransfersWidget::UPLOADS_TAB];
    countLabelText = uploadTransfers > 0 ? QString::number(uploadTransfers) : QString();

    // Then Uploads --------------------------------------------------------------------------------
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        countLabel->setText(countLabelText);
    }
    countLabel->setVisible(!countLabelText.isEmpty());
}

void TransferManager::refreshFileTypesStats()
{
    QMetaEnum tabs = QMetaEnum::fromType<TransfersWidget::TM_TAB>();

    for (int index = 0; index < tabs.keyCount(); index++)
    {
        auto value  = tabs.value(index);

        if(value > TransfersWidget::TYPES_TAB_BASE && value < TransfersWidget::TYPES_LAST)
        {
            Utilities::FileType fileType = static_cast<Utilities::FileType>(value - TransfersWidget::TYPES_TAB_BASE);
            unsigned long long number (mModel->getNumberOfTransfersForFileType(fileType));

            QString countLabelText(number > 0 ? QString::number(number) : QString());

            TransfersWidget::TM_TAB tab = static_cast<TransfersWidget::TM_TAB>(value);
            QLabel* label (mNumberLabelsGroup[tab]);
            if (mUi->wTransfers->getCurrentTab() != tab && number == 0)
            {
                if(label->parentWidget()->isVisible())
                {
                    label->parentWidget()->hide();
                    label->clear();
                }
            }
            else
            {
                label->parentWidget()->show();
                label->setVisible(number);
                label->setText(countLabelText);
            }
        }
    }
}

void TransferManager::onTransfersDataUpdated()
{
    mTransfersCount = mModel->getTransfersCount();

    long long oldNumber(0);
    QLabel* label (mNumberLabelsGroup[mUi->wTransfers->getCurrentTab()]);
    if(label)
    {
        oldNumber = label->text().toLongLong();
    }

    // Refresh stats
    refreshTypeStats();
    refreshFileTypesStats();
    refreshSearchStats();
    refreshStateStats();
    checkActionAndMediaVisibility();
    refreshView();
}

void TransferManager::onStorageStateChanged(int storageState)
{
    mStorageQuotaState = storageState;

    switch (mStorageQuotaState)
    {
        case MegaApi::STORAGE_STATE_PAYWALL:
        case MegaApi::STORAGE_STATE_RED:
        {
            mUi->tSeePlans->show();
            mUi->lStorageOverQuota->show();
            break;
        }
        case MegaApi::STORAGE_STATE_GREEN:
        case MegaApi::STORAGE_STATE_ORANGE:
        case MegaApi::STORAGE_STATE_UNKNOWN:
        default:
        {
            mUi->lStorageOverQuota->hide();
            QuotaState tQuotaState (MegaSyncApp->getTransferQuotaState());
            mUi->tSeePlans->setVisible(tQuotaState == QuotaState::FULL);

            break;
        }
    }

    //TransferQuota is not visible when storage state error is set
    //This is why we need to check the current transfer quota state
    //in case we need to show the transferquota errors again
    onTransferQuotaStateChanged(mTransferQuotaState);

    checkPauseButtonVisibilityIfPossible();
}

void TransferManager::onVerticalScrollBarVisibilityChanged(bool state)
{
    QPointer<TransferManager> currentTransferManager = this;

    if(currentTransferManager)
    {
        auto transfersView = dynamic_cast<MegaTransferView*>(sender());
        if(transfersView && transfersView->isVisible())
        {
            if(state)
            {
                int sliderWidth = transfersView->getVerticalScrollBarWidth();
                mUi->wRightPanelScrollMargin->changeSize(sliderWidth,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
            }
            else
            {
                mUi->wRightPanelScrollMargin->changeSize(0,0,QSizePolicy::Fixed, QSizePolicy::Preferred);
            }

            if(mUi->wRightPaneHeaderLayout)
            {
                mUi->wRightPaneHeaderLayout->invalidate();
            }
        }
    }
}

void TransferManager::onTransferQuotaStateChanged(QuotaState transferQuotaState)
{
    mTransferQuotaState = transferQuotaState;

    switch (mTransferQuotaState)
    {
        case QuotaState::FULL:
        case QuotaState::OVERQUOTA:
        {
            mUi->tSeePlans->show();
            showTransferQuotaBanner(mStorageQuotaState != MegaApi::STORAGE_STATE_PAYWALL
                                                && mStorageQuotaState != MegaApi::STORAGE_STATE_RED);
            break;
        }
        case QuotaState::OK:
        case QuotaState::WARNING:
        default:
        {
            showTransferQuotaBanner(false);
            break;
        }
    }

    checkPauseButtonVisibilityIfPossible();
}

void TransferManager::checkPauseButtonVisibilityIfPossible()
{
    mUi->lPaused->setVisible(mModel->areAllPaused() && !hasOverQuotaErrors());
}

void TransferManager::showTransferQuotaBanner(bool state)
{
    mUi->pTransferOverQuota->setVisible(state);

    if(Preferences::instance()->accountType() != Preferences::ACCOUNT_TYPE_FREE)
    {
        mUi->lTransferOverQuotaInfo->hide();
        mUi->lTransferOverQuotaMoreAbout->hide();
    }

    if(state)
    {
        mTransferQuotaTimer.start();
        onTransferQuotaExceededUpdate();
    }
    else
    {
        mTransferQuotaTimer.stop();
    }
}

void TransferManager::onTransferQuotaExceededUpdate()
{
    QString bannerText = tr("You can't continue downloading as you don't have enough transfer quota left for this IP address."
                             "\nTo get more quota, upgrade to a Pro account or wait for [A] until more free quota becomes available on your IP address.");
    auto text = bannerText.replace(QString(QLatin1String("[A]")), MegaSyncApp->getTransferQuota()->getTransferQuotaDeadline().toString(QLatin1String("hh:mm:ss")));
    mUi->lTransferOverQuotaInfo->setText(text);
}


void TransferManager::refreshSpeed()
{
    mUi->wUpSpeed->setVisible(mTransfersCount.pendingUploads);
    if(mTransfersCount.pendingUploads)
    {
        auto upSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentUploadSpeed()));
        mUi->lUpSpeed->setText(Utilities::getSizeString(upSpeed) + QLatin1Literal("/s"));
    }

    mUi->wDownSpeed->setVisible(mTransfersCount.pendingDownloads);
    if(mTransfersCount.pendingDownloads)
    {
        auto dlSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentDownloadSpeed()));
        mUi->lDownSpeed->setText(Utilities::getSizeString(dlSpeed) + QLatin1Literal("/s"));
    }
}

void TransferManager::refreshSearchStats()
{
    if(mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB)
    {
        // Update search results number
        auto proxy (mUi->wTransfers->getProxyModel());
        int nbDl (proxy->getNumberOfItems(TransferData::TRANSFER_DOWNLOAD));
        int nbUl (proxy->getNumberOfItems(TransferData::TRANSFER_UPLOAD));
        int nbAll (nbDl + nbUl);

        if(mUi->lDlResultsCounter->text().toLongLong() != nbDl)
        {
            mUi->lDlResultsCounter->setText(QString::number(nbDl));
        }

        if(mUi->lUlResultsCounter->text().toLongLong() != nbUl)
        {
            mUi->lUlResultsCounter->setText(QString::number(nbUl));
        }

        mUi->lAllResultsCounter->setText(QString::number(nbAll));

        if(nbDl != 0 && nbUl != 0)
        {
            mUi->searchByTextTypeSelector->setVisible(true);
            mUi->lNbResults->setVisible(false);
        }
        else
        {
            int intNbAll = static_cast<int>(nbAll);
            mUi->lNbResults->setText(QString(tr("%1 result found","", intNbAll)).arg(nbAll));
            mUi->lNbResults->setProperty("results", static_cast<bool>(nbAll));
            mUi->lNbResults->style()->unpolish(mUi->lNbResults);
            mUi->lNbResults->style()->polish(mUi->lNbResults);
            mUi->lNbResults->setVisible(true);
            mUi->searchByTextTypeSelector->setVisible(false);
        }

        bool showTypeFilters (mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB);
        mUi->wDlResults->setVisible(showTypeFilters);
        mUi->wUlResults->setVisible(showTypeFilters);
        mUi->wAllResults->setVisible(showTypeFilters);

        QWidget* widgetToShow (mUi->wTransfers);

        if (nbAll == 0)
        {
            widgetToShow = mTabNoItem[mUi->wTransfers->getCurrentTab()];
        }

        updateTransferWidget(widgetToShow);
    }
}

void TransferManager::on_tActionButton_clicked()
{
    if(mUi->wTransfers->getCurrentTab() == TransfersWidget::FAILED_TAB)
    {
        emit retryAllTransfers();
        checkActionAndMediaVisibility();
    }
}

void TransferManager::on_tSeePlans_clicked()
{
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    Utilities::openUrl(QUrl(url));
}

void TransferManager::on_bPause_toggled()
{
    auto newState = !mModel->areAllPaused();
    pauseResumeTransfers(newState);

    showQuotaStorageDialogs(newState);
}

void TransferManager::pauseResumeTransfers(bool isPaused)
{
    mModel->pauseResumeAllTransfers(isPaused);
    onUpdatePauseState(isPaused);
}

void TransferManager::onStalledIssuesStateChanged(bool state)
{
    mFoundStalledIssues = state;
    checkContentInfo();
}

void TransferManager::checkContentInfo()
{
    if(mFoundStalledIssues)
    {
        mUi->sCurrentContentInfo->setCurrentWidget(mUi->pStalledIssuesHeaderInfo);
    }
    else
    {
        mUi->sCurrentContentInfo->setCurrentWidget(mUi->pStatusHeaderInfo);
    }
}

void TransferManager::on_bSearch_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pSearch);
    mUi->leSearchField->setText(QString());
}

void TransferManager::on_leSearchField_editingFinished()
{
    if(mUi->leSearchField->text().isEmpty())
    {
       mUi->wTitleAndSearch->setCurrentWidget(mUi->pTransfers);
    }
}

void TransferManager::on_tSearchIcon_clicked()
{
    QString pattern (mUi->leSearchField->text());
    if(pattern.isEmpty())
    {
        on_tClearSearchResult_clicked();
    }
    else
    {
        mUi->bSearchString->setText(pattern);
        mUi->bSearchString->setProperty(SEARCH_TEXT, pattern);
        applyTextSearch(pattern);
    }
}

void TransferManager::applyTextSearch(const QString& text)
{
    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                              .elidedText(text,
                                          Qt::ElideMiddle,
                                          mUi->lTextSearch->width()));
    // Add number of found results
    mUi->wAllResults->setProperty(SEARCH_BUTTON_SELECTED, true);
    mUi->wDlResults->setProperty(SEARCH_BUTTON_SELECTED, false);
    mUi->wUlResults->setProperty(SEARCH_BUTTON_SELECTED, false);

    mUi->wAllResults->hide();
    mUi->wDlResults->hide();
    mUi->wUlResults->hide();
    mUi->wSearch->show();

    //This is done to refresh the stylesheet as depends on the object properties
    mUi->pSearchHeaderInfo->setStyleSheet(mUi->pSearchHeaderInfo->styleSheet());

    mUi->wTransfers->transferFilterReset();

    //It is important to call it after resetting the filter, as the reset removes the text
    //search
    mUi->wTransfers->textFilterChanged(text);
    toggleTab(TransfersWidget::SEARCH_TAB);
}

void TransferManager::enableUserActions(bool enabled)
{
    mUi->wLeftPane->setEnabled(enabled);
    mUi->wRightPaneHeader->setEnabled(enabled);
}

void TransferManager::on_bSearchString_clicked()
{
    applyTextSearch(mUi->bSearchString->property(SEARCH_TEXT).toString());
}

void TransferManager::on_tSearchCancel_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pTransfers);
    mUi->leSearchField->setPlaceholderText(tr("Search"));
}

void TransferManager::on_tClearSearchResult_clicked()
{
    mUi->wSearch->hide();
    mUi->bSearchString->setText(QString());
    if (mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB)
    {
        mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
        checkContentInfo();
        on_tAllTransfers_clicked();
    }
}

void TransferManager::showAllResults()
{
    mUi->wAllResults->setProperty(SEARCH_BUTTON_SELECTED, true);
    mUi->wDlResults->setProperty(SEARCH_BUTTON_SELECTED, false);
    mUi->wUlResults->setProperty(SEARCH_BUTTON_SELECTED, false);

    mUi->pSearchHeaderInfo->setStyleSheet(mUi->pSearchHeaderInfo->styleSheet());

    mUi->wTransfers->textFilterTypeChanged({});
}

void TransferManager::showDownloadResults()
{
    mUi->wAllResults->setProperty(SEARCH_BUTTON_SELECTED, false);
    mUi->wDlResults->setProperty(SEARCH_BUTTON_SELECTED, true);
    mUi->wUlResults->setProperty(SEARCH_BUTTON_SELECTED, false);

    mUi->pSearchHeaderInfo->setStyleSheet(mUi->pSearchHeaderInfo->styleSheet());

    mUi->wTransfers->textFilterTypeChanged(TransferData::TRANSFER_DOWNLOAD
                                    | TransferData::TRANSFER_LTCPDOWNLOAD);
}

void TransferManager::showUploadResults()
{
    mUi->wAllResults->setProperty(SEARCH_BUTTON_SELECTED, false);
    mUi->wDlResults->setProperty(SEARCH_BUTTON_SELECTED, false);
    mUi->wUlResults->setProperty(SEARCH_BUTTON_SELECTED, true);

    mUi->pSearchHeaderInfo->setStyleSheet(mUi->pSearchHeaderInfo->styleSheet());

    mUi->wTransfers->textFilterTypeChanged(TransferData::TRANSFER_UPLOAD);
}

void TransferManager::on_bArchives_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_ARCHIVE_TAB, Utilities::FileType::TYPE_ARCHIVE);
}

void TransferManager::on_bDocuments_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_DOCUMENT_TAB, Utilities::FileType::TYPE_DOCUMENT);
}

void TransferManager::on_bImages_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_IMAGE_TAB, Utilities::FileType::TYPE_IMAGE);
}

void TransferManager::on_bAudio_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_AUDIO_TAB, Utilities::FileType::TYPE_AUDIO);
}

void TransferManager::on_bVideos_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_VIDEO_TAB, Utilities::FileType::TYPE_VIDEO);
}

void TransferManager::on_bOther_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_OTHER_TAB, Utilities::FileType::TYPE_OTHER);
}

void TransferManager::onFileTypeButtonClicked(TransfersWidget::TM_TAB tab, Utilities::FileType fileType)
{
    if (mUi->wTransfers->getCurrentTab() != tab)
    {
        mUi->wTransfers->filtersChanged({}, {}, {fileType});
        toggleTab(tab);
    }
}

void TransferManager::on_bOpenLinks_clicked()
{
    MegaSyncApp->importLinksFromWidget(this);
}

void TransferManager::on_tCogWheel_clicked()
{
    MegaSyncApp->openSettings();
}

void TransferManager::on_bDownload_clicked()
{
    MegaSyncApp->downloadActionClickedFromWidget(this);
}

void TransferManager::on_bUpload_clicked()
{
    MegaSyncApp->uploadActionClickedFromWidget(this);
}

void TransferManager::on_leSearchField_returnPressed()
{
    emit mUi->tSearchIcon->clicked();
}

void TransferManager::toggleTab(int newTab)
{
    toggleTab(static_cast<TransfersWidget::TM_TAB>(newTab));
}

void TransferManager::toggleTab(TransfersWidget::TM_TAB newTab)
{
    if (mUi->wTransfers->getCurrentTab() != newTab)
    {
        filterByTab(newTab);

        //First, update the data
        onTransfersDataUpdated();

        // De-activate old tab frame
        if (mUi->wTransfers->getCurrentTab() != TransfersWidget::NO_TAB)
        {
            mTabFramesToggleGroup[mUi->wTransfers->getCurrentTab()]->setProperty(ITS_ON, false);
            auto pushButton = mTabFramesToggleGroup[mUi->wTransfers->getCurrentTab()]->findChild<QPushButton*>();
            if(pushButton)
            {
                pushButton->setChecked(false);
            }
        }

        // Activate new tab frame
        mTabFramesToggleGroup[newTab]->setProperty(ITS_ON, true);
        mTabFramesToggleGroup[newTab]->setGraphicsEffect(mShadowTab);

        // Reload QSS because it is glitchy
        mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());

        auto pushButton = mTabFramesToggleGroup[newTab]->findChild<QPushButton*>();
        if(pushButton)
        {
            pushButton->setChecked(true);
        }

        auto previousTab = mUi->wTransfers->getCurrentTab();
        mUi->wTransfers->setCurrentTab(newTab);

        //The rest of cases
        if (previousTab == TransfersWidget::COMPLETED_TAB
                || previousTab == TransfersWidget::FAILED_TAB
                || (previousTab > TransfersWidget::TYPES_TAB_BASE && previousTab < TransfersWidget::TYPES_LAST))
        {
            long long transfers(0);

            if(previousTab == TransfersWidget::COMPLETED_TAB)
            {
                transfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
            }
            else if(previousTab == TransfersWidget::FAILED_TAB)
            {
                transfers = mTransfersCount.totalFailedTransfers();
            }
            else
            {
                Utilities::FileType fileType = static_cast<Utilities::FileType>(previousTab - TransfersWidget::TYPES_TAB_BASE);
                transfers = mModel->getNumberOfTransfersForFileType(fileType);
            }

            if(transfers == 0)
            {
                auto countLabel(mNumberLabelsGroup[previousTab]);
                if(countLabel->parentWidget()->isVisible())
                {
                    countLabel->parentWidget()->hide();
                }
            }
        }

        //In case the media group // actions buttons must be hidden
        checkActionAndMediaVisibility();

        // Set current header widget: search or not
        if (newTab == TransfersWidget::SEARCH_TAB)
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pSearchHeader);
            mUi->sCurrentContentInfo->setCurrentWidget(mUi->pSearchHeaderInfo);
        }
        else
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
            checkContentInfo();
            updateCurrentCategoryTitle();
            mUi->wTransfers->textFilterChanged(QString());
        }
    }
}

void TransferManager::refreshView()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::NO_TAB)
    {
        if(mUi->wTransfers->getCurrentTab() != TransfersWidget::SEARCH_TAB)
        {
            QWidget* widgetToShow (mUi->wTransfers);

            if(!mUi->wTransfers->isLoadingViewSet())
            {
                if(mUi->wTransfers->getProxyModel()->isEmpty())
                {
                    widgetToShow = mTabNoItem[mUi->wTransfers->getCurrentTab()];
                }
            }

            updateTransferWidget(widgetToShow);
        }
    }
}

void TransferManager::checkActionAndMediaVisibility()
{
    auto completedTransfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
    auto allTransfers = mTransfersCount.pendingDownloads + mTransfersCount.pendingUploads;
    auto failedTransfers = mTransfersCount.totalFailedTransfers();

    // Show "Clear All/Completed" if there are any completed transfers
    // (only for completed tab and individual media tabs)
    if (mUi->wTransfers->getCurrentTab() == TransfersWidget::FAILED_TAB && failedTransfers > 0)
    {
        mUi->tActionButton->show();
    }
    else
    {
        mUi->tActionButton->hide();
    }

    // Hide Media groupbox if no transfers (active or finished)
    if (mUi->wTransfers->getCurrentTab() >= TransfersWidget::TYPES_TAB_BASE
            || ((allTransfers +
                completedTransfers + failedTransfers) > 0))
    {
        mUi->wMediaType->show();
    }
    else
    {
        mUi->wMediaType->hide();
    }
}

bool TransferManager::eventFilter(QObject *obj, QEvent *event)
{
    if (obj == mUi->lTextSearch
            && event->type() == QEvent::Resize
            && mUi->sCurrentContent->currentWidget() == mUi->pSearchHeader)
    {
        auto newWidth (static_cast<QResizeEvent*>(event)->size().width());
        mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                                  .elidedText(mUi->leSearchField->text(),
                                              Qt::ElideMiddle,
                                              newWidth - 24));
    }
    else if(obj == mUi->lTransfers && event->type() == QEvent::Resize)
    {
        mUi->lTransfers->setText(mUi->lTransfers->fontMetrics()
                                    .elidedText(mUi->lTransfers->text(),
                                                Qt::ElideMiddle,
                                                mUi->lTransfers->width()));
    }
    else if(obj == mUi->leSearchField && event->type() == QEvent::KeyPress)
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if(keyEvent && keyEvent->key() == Qt::Key_Escape)
        {
            event->accept();
            on_tSearchCancel_clicked();
            focusNextChild();
            return true;
        }
        else if(keyEvent && keyEvent->key() == Qt::Key_Return)
        {
            mSearchFieldReturnPressed = true;
        }
    }
    else if(obj == mUi->wAllResults || obj == mUi->wDlResults || obj == mUi->wUlResults)
    {
        if(event->type() == QEvent::MouseButtonRelease)
        {
            if(obj == mUi->wAllResults)
            {
                showAllResults();
            }
            else if(obj == mUi->wDlResults)
            {

                showDownloadResults();
            }
            else if(obj == mUi->wUlResults)
            {
                showUploadResults();
            }
        }
        else if(event->type() == QEvent::KeyRelease)
        {
            auto widget = dynamic_cast<QWidget*>(obj);
            if(widget)
            {
                auto keyEvent = dynamic_cast<QKeyEvent*>(event);
                if(keyEvent && widget->hasFocus())
                {
                    if(keyEvent->key() == Qt::Key_Space)
                    {
                        QApplication::postEvent(widget, new QEvent(QEvent::MouseButtonRelease));
                    }
                }
            }
        }
        else if(event->type() == QEvent::Paint)
        {
            auto widget = dynamic_cast<QWidget*>(obj);
            if(widget && widget->hasFocus())
            {
                QPainter painter(widget);
                QStyleOptionFocusRect option;
                if(option.state |= QStyle::State_KeyboardFocusChange)
                {
                    option.init(widget);
                    option.backgroundColor = palette().color(QPalette::Window);

                    style()->drawPrimitive(QStyle::PE_FrameFocusRect, &option, &painter,
                                           this);
                    return true;
                }
            }
        }
    }
    else if(obj == mUi->wLeftPane && event->type() == QEvent::Polish)
    {
        //Set the style for the first time as you need to update it as it depends on properties
        mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
    }

    return QDialog::eventFilter(obj, event);
}

void TransferManager::closeEvent(QCloseEvent *event)
{
    auto proxy (mUi->wTransfers->getProxyModel());

    if(proxy->isModelProcessing())
    {
        connect(proxy, &TransfersManagerSortFilterProxyModel::modelChanged, this, [this](){
            close();
        });
        event->ignore();
    }
    else
    {
        QDialog::closeEvent(event);
    }
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateCurrentCategoryTitle();
        updateCurrentSearchText();
        updateCurrentOverQuotaLink();
        onUpdatePauseState(mUi->wTransfers->getProxyModel()->getPausedTransfers());
    }
    QDialog::changeEvent(event);
}

void TransferManager::mouseReleaseEvent(QMouseEvent *event)
{
    mUi->wTransfers->mouseRelease(event->globalPos());

    QDialog::mouseReleaseEvent(event);
}

bool TransferManager::hasOverQuotaErrors()
{
    return (mTransferQuotaState == QuotaState::FULL || mTransferQuotaState == QuotaState::OVERQUOTA
            || (mStorageQuotaState == MegaApi::STORAGE_STATE_PAYWALL
            || mStorageQuotaState == MegaApi::STORAGE_STATE_RED));
}

void TransferManager::setTransferState(const StatusInfo::TRANSFERS_STATES &transferState)
{
    if(transferState == StatusInfo::TRANSFERS_STATES::STATE_INDEXING)
    {
        mScanningTimer.start();
    }
    else
    {
        if(mScanningTimer.isActive())
        {
            mScanningTimer.stop();
            mScanningAnimationIndex = 1;
        }
        refreshStateStats();
    }
}

void TransferManager::onScanningAnimationUpdate()
{
    mUi->bScanning->setIcon(StatusInfo::scanningIcon(mScanningAnimationIndex));
}

void TransferManager::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->acceptProposedAction();
        event->accept();
        mDragBackDrop->show();
        mDragBackDrop->resize(size());
    }

    QDialog::dragEnterEvent(event);
}

void TransferManager::dropEvent(QDropEvent* event)
{
    mDragBackDrop->hide();

    event->acceptProposedAction();
    QDialog::dropEvent(event);

    QQueue<QString> pathsToAdd;
    QList<QUrl> urlsToAdd = event->mimeData()->urls();
    foreach(auto& urlToAdd, urlsToAdd)
    {
        auto file = urlToAdd.toLocalFile();
#ifdef __APPLE__
        QFileInfo fileInfo(file);
        if (fileInfo.isDir())
        {
            file.remove(file.length()-1,1);
        }
#endif

        pathsToAdd.append(file);
    }

    MegaSyncApp->shellUpload(pathsToAdd);
}

void TransferManager::dragLeaveEvent(QDragLeaveEvent *event)
{
    mDragBackDrop->hide();

    QDialog::dragLeaveEvent(event);
}

void TransferManager::updateTransferWidget(QWidget* widgetToShow)
{
    if ( !mTransferScanCancelUi || !mTransferScanCancelUi->isActive())
    {
        if (mUi->sTransfers->currentWidget() != widgetToShow)
        {
            mUi->sTransfers->setCurrentWidget(widgetToShow);

            //Update the headers and the pause/resume and cancel/clear buttons
            if(widgetToShow == mUi->wTransfers)
            {
                mUi->wTransfers->updateHeaders();
            }
        }
    }
}
