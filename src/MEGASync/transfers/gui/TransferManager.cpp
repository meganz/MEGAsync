#include "TransferManager.h"

#include "MegaApplication.h"
#include "MegaTransferView.h"
#include "Platform.h"
#include "StalledIssuesModel.h"
#include "ui_TransferManager.h"
#include "ui_TransferManagerDragBackDrop.h"

#include <QMouseEvent>
#include <QPainter>
#include <QPalette>
#include <QScrollBar>
#include <QStyleOptionFocusRect>

using namespace mega;
using namespace std::chrono_literals;

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

TransferManager::TransferManager(MegaApi* megaApi):
    QDialog(nullptr),
    mUi(new Ui::TransferManager),
    mMegaApi(megaApi),
    mPreferences(Preferences::instance()),
    mModel(nullptr),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this)),
    mUiDragBackDrop(new Ui::TransferManagerDragBackDrop),
    mStorageQuotaState(MegaApi::STORAGE_STATE_UNKNOWN),
    mTransferQuotaState(QuotaState::OK),
    mFoundStalledIssues(false)
{
    mUi->setupUi(this);
    createSearchChips();

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

    mTabFramesToggleGroup[TransfersWidget::ALL_TRANSFERS_TAB] = mUi->fAllTransfers;
    mTabFramesToggleGroup[TransfersWidget::DOWNLOADS_TAB]     = mUi->fDownloads;
    mTabFramesToggleGroup[TransfersWidget::UPLOADS_TAB]       = mUi->fUploads;
    mTabFramesToggleGroup[TransfersWidget::COMPLETED_TAB]     = mUi->fCompleted;
    mTabFramesToggleGroup[TransfersWidget::FAILED_TAB]        = mUi->fFailed;
    mTabFramesToggleGroup[TransfersWidget::SEARCH_TAB]        = mUi->fSearchString;
    mTabFramesToggleGroup[TransfersWidget::TYPE_OTHER_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_OTHER_TAB);
    mTabFramesToggleGroup[TransfersWidget::TYPE_AUDIO_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_AUDIO_TAB);
    mTabFramesToggleGroup[TransfersWidget::TYPE_VIDEO_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_VIDEO_TAB);
    mTabFramesToggleGroup[TransfersWidget::TYPE_ARCHIVE_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_ARCHIVE_TAB);
    mTabFramesToggleGroup[TransfersWidget::TYPE_DOCUMENT_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_DOCUMENT_TAB);
    mTabFramesToggleGroup[TransfersWidget::TYPE_IMAGE_TAB] =
        mUi->wMediaType->getFrame(TransfersWidget::TYPE_IMAGE_TAB);

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
    mNumberLabelsGroup[TransfersWidget::TYPE_OTHER_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_OTHER_TAB);
    mNumberLabelsGroup[TransfersWidget::TYPE_AUDIO_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_AUDIO_TAB);
    mNumberLabelsGroup[TransfersWidget::TYPE_VIDEO_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_VIDEO_TAB);
    mNumberLabelsGroup[TransfersWidget::TYPE_ARCHIVE_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_ARCHIVE_TAB);
    mNumberLabelsGroup[TransfersWidget::TYPE_DOCUMENT_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_DOCUMENT_TAB);
    mNumberLabelsGroup[TransfersWidget::TYPE_IMAGE_TAB] =
        mUi->wMediaType->getLabel(TransfersWidget::TYPE_IMAGE_TAB);

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

    auto managedButtons = mUi->wRightPaneHeader->findChildren<QAbstractButton*>();
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

    connect(mUi->wTransfers, &TransfersWidget::sortCriterionChanged,
        this, &TransferManager::onSortCriterionChanged);

    connect(mUi->wTransfers, &TransfersWidget::loadingViewVisibilityChanged, this, &TransferManager::refreshView);
    connect(mUi->wTransfers,&TransfersWidget::disableTransferManager,this, &TransferManager::disableTransferManager);

    connect(MegaSyncApp->getStalledIssuesModel(),
            &StalledIssuesModel::stalledIssuesChanged,
            this, &TransferManager::onStalledIssuesStateChanged);

    onStalledIssuesStateChanged();

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    // Connect to storage quota signals
    connect(MegaSyncApp, &MegaApplication::storageStateChanged,
            this, &TransferManager::onStorageStateChanged,
            Qt::QueuedConnection);

    setAcceptDrops(true);

    // Init state
    auto storageState = MegaSyncApp->getAppliedStorageState();
    auto transferQuotaState = MegaSyncApp->getTransferQuota()->quotaState();
    onStorageStateChanged(storageState);
    onTransferQuotaStateChanged(transferQuotaState);

    onUpdatePauseState(mPreferences->getGlobalPaused());

    mTransferScanCancelUi = new TransferScanCancelUi(mUi->sTransfers, mTabNoItem[TransfersWidget::ALL_TRANSFERS_TAB]);
    connect(mTransferScanCancelUi, &TransferScanCancelUi::cancelTransfers,
            this, &TransferManager::cancelScanning);

    // Search
    connect(mUi->leSearch, &SearchLineEdit::search, this, &TransferManager::onSearch);
    mUi->leSearch->addCustomWidget(mUi->pTransfers);

#ifndef Q_OS_MACOS
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

    setAttribute(Qt::WA_DeleteOnClose, true);

    startRequestTaskbarPinningTimer();
}

TransferManager::~TransferManager()
{
    disconnect(findChild<MegaTransferView*>(), &MegaTransferView::verticalScrollBarVisibilityChanged,
            this, &TransferManager::onVerticalScrollBarVisibilityChanged);

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
}

void TransferManager::leaveBlockingState(bool fromCancellation)
{
    enableUserActions(true);
    mUi->wTransfers->setScanningWidgetVisible(false);
    mTransferScanCancelUi->hide(fromCancellation);
    refreshStateStats();
    refreshView();
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

QString TransferManager::getResumeAllTransfersTooltip()
{
    return tr("Resume all transfers");
}

QString TransferManager::getPauseAllTransfersTooltip()
{
    return tr("Pause all transfers");
}

void TransferManager::onUpdatePauseState(bool isPaused)
{
    if (isPaused)
    {
        mUi->bPause->setToolTip(getResumeAllTransfersTooltip());

        if(!mUi->bPause->isChecked())
        {
           mUi->bPause->blockSignals(true);
           mUi->bPause->setChecked(true);
           mUi->bPause->blockSignals(false);
        }
    }
    else
    {
        mUi->bPause->setToolTip(getPauseAllTransfersTooltip());

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
    uint processedNumber (0);

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

    auto failedNumber(mTransfersCount.totalFailedTransfers());
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
    if (processedNumber == 0 && failedNumber == 0 && MegaSyncApp->getStalledIssuesModel()->isEmpty())
    {
        if(mTransferScanCancelUi && mTransferScanCancelUi->isActive())
        {
            leftFooterWidget = mUi->wScanning;
            mUi->bScanning->setState(StatusInfo::TRANSFERS_STATES::STATE_INDEXING);
        }
        else
        {
            leftFooterWidget = mUi->pUpToDate;
            mUi->bScanning->setState(StatusInfo::TRANSFERS_STATES::STATE_UPDATED);
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
            leftFooterWidget = mUi->wScanning;
            mUi->bScanning->setState(StatusInfo::TRANSFERS_STATES::STATE_INDEXING);
        }
        else
        {
            mUi->bScanning->setState(StatusInfo::TRANSFERS_STATES::STATE_UPDATED);

            if (processedNumber != 0)
            {
                leftFooterWidget = mUi->pSpeedAndClear;
            }
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

            TransfersWidget::TM_TAB tab = static_cast<TransfersWidget::TM_TAB>(value);
            if (mUi->wTransfers->getCurrentTab() != tab && number == 0)
            {
                mUi->wMediaType->resetCounter(tab);
            }
            else
            {
                mUi->wMediaType->showIfGroupboxVisible(tab, number);
            }
        }
    }
}

void TransferManager::onTransfersDataUpdated()
{
    mTransfersCount = mModel->getTransfersCount();

    // Refresh stats
    refreshTypeStats();
    refreshFileTypesStats();
    refreshSearchStats();
    refreshStateStats();
    refreshView();
}

void TransferManager::onStorageStateChanged(int storageState)
{
    mStorageQuotaState = storageState;

    mUi->pStatusHeaderInfo->setStorageQuotaState(storageState);
    mUi->pStatusHeaderInfo->updateStorageBannerText();

    // TransferQuota is not visible when storage state error is set
    // This is why we need to check the current transfer quota state
    // in case we need to show the transferquota errors again
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
        // Fallthrough
        case QuotaState::OVERQUOTA:
        {
            mUi->pStatusHeaderInfo->showTransferBanner(
                mStorageQuotaState != MegaApi::STORAGE_STATE_PAYWALL &&
                mStorageQuotaState != MegaApi::STORAGE_STATE_RED);
            break;
        }
        case QuotaState::OK:
        // Fallthrough
        case QuotaState::WARNING:
        // Fallthrough
        default:
        {
            mUi->pStatusHeaderInfo->showTransferBanner(false);
            break;
        }
    }

    mUi->wAccountInfo->setTransferOverquota(mTransferQuotaState != QuotaState::OK);
    checkPauseButtonVisibilityIfPossible();
}

void TransferManager::checkPauseButtonVisibilityIfPossible()
{
    mModel->areAllPaused() && !hasOverQuotaErrors() ?
        mUi->pStatusHeaderInfo->showAllPausedBanner() :
        mUi->pStatusHeaderInfo->hideAllPausedBanner();
}

void TransferManager::onSortCriterionChanged(int sortBy, Qt::SortOrder order)
{
    mTabSortCriterion.insert(mUi->wTransfers->getCurrentTab(), qMakePair(sortBy ,order));
}

void TransferManager::refreshSpeed()
{
    mUi->wUpSpeed->setVisible(mTransfersCount.pendingUploads);
    if(mTransfersCount.pendingUploads)
    {
        auto upSpeed (mMegaApi->getCurrentUploadSpeed());
        mUi->lUpSpeed->setText(Utilities::getSizeString(upSpeed) + QLatin1String("/s"));
    }

    mUi->wDownSpeed->setVisible(mTransfersCount.pendingDownloads);
    if(mTransfersCount.pendingDownloads)
    {
        auto dlSpeed (mMegaApi->getCurrentDownloadSpeed());
        mUi->lDownSpeed->setText(Utilities::getSizeString(dlSpeed) + QLatin1String("/s"));
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
        mUi->wAllResults->setCounter(nbAll);
        mUi->wDlResults->setCounter(nbDl);
        mUi->wUlResults->setCounter(nbUl);

        if(nbDl != 0 && nbUl != 0)
        {
            mUi->searchByTextTypeSelector->setVisible(true);
            mUi->lNbResults->setVisible(false);
        }
        else
        {
            int intNbAll = static_cast<int>(nbAll);
            mUi->lNbResults->setText(QString(tr("%1 result found", "", intNbAll)).arg(nbAll));
            mUi->lNbResults->setVisible(true);
            mUi->searchByTextTypeSelector->setVisible(false);
        }

        bool showTypeFilters (mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB);

        mUi->wAllResults->setVisible(showTypeFilters);
        mUi->wDlResults->setVisible(showTypeFilters);
        mUi->wUlResults->setVisible(showTypeFilters);
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

void TransferManager::onSearch(const QString& text)
{
    mUi->bSearchString->setProperty(SEARCH_TEXT, text);
    applyTextSearch(text);
}

void TransferManager::onStalledIssuesStateChanged()
{
    mFoundStalledIssues = !MegaSyncApp->getStalledIssuesModel()->isEmpty();
    checkContentInfo();
    refreshStateStats();
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

void TransferManager::applyTextSearch(const QString& text)
{
    mUi->wAllResults->setSelected(true);
    mUi->wAllResults->hide();
    mUi->wDlResults->hide();
    mUi->wUlResults->hide();

    mUi->wSearch->show();
    mUi->bSearchString->setText(text);

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
    mUi->wLeftPaneTop->setEnabled(enabled);
    mUi->wRightPaneHeader->setEnabled(enabled);
}

void TransferManager::on_bSearchString_clicked()
{
    applyTextSearch(mUi->bSearchString->property(SEARCH_TEXT).toString());
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
    mUi->wTransfers->textFilterTypeChanged({});
}

void TransferManager::showDownloadResults()
{
    mUi->wTransfers->textFilterTypeChanged(TransferData::TRANSFER_DOWNLOAD
                                    | TransferData::TRANSFER_LTCPDOWNLOAD);
}

void TransferManager::showUploadResults()
{
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
    MegaSyncApp->importLinks();
}

void TransferManager::on_tCogWheel_clicked()
{
    MegaSyncApp->openSettings();
}

void TransferManager::on_bDownload_clicked()
{
    MegaSyncApp->downloadActionClicked();
}

void TransferManager::on_bUpload_clicked()
{
    MegaSyncApp->uploadActionClicked();
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

        auto tabFound = mTabSortCriterion.find(newTab);
        if (tabFound != mTabSortCriterion.end())
        {
            mUi->wTransfers->setSortCriterion(tabFound->first, tabFound->second);
        }

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
            uint transfers(0);

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

        //In case the media group // actions buttons must be hidden
        checkActionAndMediaVisibility();
    }
}

void TransferManager::disableTransferManager(bool state)
{
    setDisabled(state);
}

void TransferManager::checkActionAndMediaVisibility()
{
    auto completedTransfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
    auto allTransfers = mTransfersCount.pendingDownloads + mTransfersCount.pendingUploads;
    auto failedTransfers = mTransfersCount.totalFailedTransfers();

    // Show "Retry all" if there are any completed transfers
    // (only for failed tag)
    if (mUi->wTransfers->getCurrentTab() == TransfersWidget::FAILED_TAB && failedTransfers > 0)
    {
        auto proxy (mUi->wTransfers->getProxyModel());
        mUi->tActionButton->setVisible(!proxy->areAllFailsPermanent());
    }
    else
    {
        mUi->tActionButton->setVisible(false);
    }

    // Hide Media groupbox if no transfers (active or finished)
    if (mUi->wTransfers->getCurrentTab() >= TransfersWidget::TYPES_TAB_BASE ||
        ((allTransfers + completedTransfers + failedTransfers) > 0))
    {
        mUi->wMediaType->show();
    }
    else
    {
        mUi->wMediaType->hide();
    }
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

bool TransferManager::event(QEvent* event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        updateCurrentCategoryTitle();
        onUpdatePauseState(mUi->wTransfers->getProxyModel()->getPausedTransfers());
    }
    return QDialog::event(event);
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
    if (transferState != StatusInfo::TRANSFERS_STATES::STATE_INDEXING)
    {
        refreshStateStats();
    }
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

void TransferManager::startRequestTaskbarPinningTimer()
{
    auto preferences = Preferences::instance();
    if (preferences->logged() &&
        !preferences->isOneTimeActionUserDone(Preferences::ONE_TIME_ACTION_REQUEST_PIN_TASKBAR))
    {
        mTaskbarPinningRequestTimer = new QTimer(this);
        connect(mTaskbarPinningRequestTimer,
                &QTimer::timeout,
                this,
                &TransferManager::onRequestTaskbarPinningTimeout);

        mTaskbarPinningRequestTimer->start(500ms);
    }
}

void TransferManager::onRequestTaskbarPinningTimeout()
{
    mTaskbarPinningRequestTimer->stop();
    Platform::getInstance()->pinOnTaskbar();
}

void TransferManager::createSearchChips()
{
    mUi->wAllResults->setTitle(tr("All"));
    mUi->wDlResults->setTitle(tr("Downloads"));
    mUi->wUlResults->setTitle(tr("Uploads"));

    mUi->wAllResults->hideIcon();
    mUi->wDlResults->hideIcon();
    mUi->wUlResults->hideIcon();

    QObject::connect(mUi->wAllResults,
                     &TabSelector::clicked,
                     this,
                     &TransferManager::showAllResults);
    QObject::connect(mUi->wDlResults,
                     &TabSelector::clicked,
                     this,
                     &TransferManager::showDownloadResults);
    QObject::connect(mUi->wUlResults,
                     &TabSelector::clicked,
                     this,
                     &TransferManager::showUploadResults);
}
