#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "ui_TransferManagerDragBackDrop.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "MegaTransferDelegate.h"
#include "MegaTransferView.h"
#include "OverQuotaDialog.h"

#include <QMouseEvent>
#include <QScrollBar>
#include <QPalette>

using namespace mega;

const int TransferManager::SPEED_REFRESH_PERIOD_MS;
const int TransferManager::STATS_REFRESH_PERIOD_MS;

const char* LABEL_NUMBER = "NUMBER";
const char* ITS_ON = "itsOn";

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
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
    mTransferQuotaState(QuotaState::OK)
{
    mUi->setupUi(this);

    mDragBackDrop = new QWidget(this);
    mUiDragBackDrop->setupUi(mDragBackDrop);
    mDragBackDrop->hide();

    mUi->wTransfers->setupTransfers();

#ifdef Q_OS_MACOS
    mUi->leSearchField->setAttribute(Qt::WA_MacShowFocusRect,0);
#else
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

    setAttribute(Qt::WA_DeleteOnClose, true);

    mUi->lTextSearch->installEventFilter(this);
    mUi->leSearchField->installEventFilter(this);

    mModel = mUi->wTransfers->getModel();

    Platform::enableDialogBlur(this);

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
            TransfersWidget::TM_TAB tab = static_cast<TransfersWidget::TM_TAB>(value);
            mNumberLabelsGroup[tab]->parentWidget()->hide();
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
            &TransfersWidget::disableTransferManager,[this](bool state){
        setDisabled(state);

        if(!state && mSearchFieldReturnPressed)
        {
            mUi->leSearchField->setFocus();
            mSearchFieldReturnPressed = false;
        }
    });

    mScanningTimer.setInterval(60);
    connect(&mScanningTimer, &QTimer::timeout, this, &TransferManager::onScanningAnimationUpdate);

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    auto sizePolicy = mUi->bDownSpeed->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    mUi->bDownSpeed->setSizePolicy(sizePolicy);

    sizePolicy = mUi->bUpSpeed->sizePolicy();
    sizePolicy.setRetainSizeWhenHidden(true);
    mUi->bUpSpeed->setSizePolicy(sizePolicy);

    // Connect to storage quota signals
    connect(qobject_cast<MegaApplication*>(qApp), &MegaApplication::storageStateChanged,
            this, &TransferManager::onStorageStateChanged,
            Qt::QueuedConnection);

    setAcceptDrops(true);

    // Init state
    onUpdatePauseState(mPreferences->getGlobalPaused());

    auto storageState = MegaSyncApp->getAppliedStorageState();
    auto transferQuotaState = MegaSyncApp->getTransferQuotaState();
    onStorageStateChanged(storageState);
    onTransferQuotaStateChanged(transferQuotaState);

    setActiveTab(TransfersWidget::ALL_TRANSFERS_TAB);
    //Update stats
    onTransfersDataUpdated();

    // Refresh Style, QSS is glitchy on first start???
    auto tabFrame (mTabFramesToggleGroup[mUi->wTransfers->getCurrentTab()]);
    tabFrame->style()->unpolish(tabFrame);
    tabFrame->style()->polish(tabFrame);
    const auto children (tabFrame->findChildren<QWidget*>());
    for (auto w : children)
    {
        w->style()->unpolish(w);
        w->style()->polish(w);
    }

    mTransferScanCancelUi = new TransferScanCancelUi(mUi->sTransfers);
    connect(mTransferScanCancelUi, &TransferScanCancelUi::cancelTransfers,
            this, &TransferManager::cancelScanning);
}

void TransferManager::pauseModel(bool value)
{
    mModel->pauseModelProcessing(value);
}

void TransferManager::enterBlockingState()
{
    enableUserActions(false);
    mTransferScanCancelUi->show();
}

void TransferManager::leaveBlockingState()
{
    enableUserActions(true);
    mTransferScanCancelUi->hide();
}

void TransferManager::disableCancelling()
{
    mTransferScanCancelUi->disableCancelling();
}

void TransferManager::onPauseStateChangedByTransferResume()
{
    onUpdatePauseState(false);
}

void TransferManager::setActiveTab(int t)
{
    switch (t)
    {
        case TransfersWidget::DOWNLOADS_TAB:
            on_tDownloads_clicked();
            break;
        case TransfersWidget::UPLOADS_TAB:
            on_tUploads_clicked();
            break;
        case TransfersWidget::COMPLETED_TAB:
            on_tCompleted_clicked();
            break;
        case TransfersWidget::SEARCH_TAB:
            on_tSearchIcon_clicked();
            break;
        case TransfersWidget::ALL_TRANSFERS_TAB:
        default:
            on_tAllTransfers_clicked();
            break;
    }
}

TransferManager::~TransferManager()
{
    delete mUi;
    delete mTransferScanCancelUi;
}

void TransferManager::on_tCompleted_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::COMPLETED_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged({}, TransferData::TRANSFER_COMPLETED, {});
        mUi->lCurrentContent->setText(tr("Finished"));
        toggleTab(TransfersWidget::COMPLETED_TAB);
    }
}

void TransferManager::on_tDownloads_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::DOWNLOADS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged((TransferData::TRANSFER_DOWNLOAD
                                         | TransferData::TRANSFER_LTCPDOWNLOAD),
                                        TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("Downloads"));
        toggleTab(TransfersWidget::DOWNLOADS_TAB);
    }
}

void TransferManager::on_tUploads_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::UPLOADS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged(TransferData::TRANSFER_UPLOAD, TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("Uploads"));
        toggleTab(TransfersWidget::UPLOADS_TAB);
    }
}

void TransferManager::on_tAllTransfers_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::ALL_TRANSFERS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged({}, TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("All transfers"));

        toggleTab(TransfersWidget::ALL_TRANSFERS_TAB);
    }
}

void TransferManager::on_tFailed_clicked()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::FAILED_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged({}, TransferData::TRANSFER_FAILED, {});
        mUi->lCurrentContent->setText(tr("Failed"));

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

    mUi->lPaused->setVisible(isPaused && !mUi->lStorageOverQuota->isVisible() && !mUi->pTransferOverQuota->isVisible());
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
    if(!isPaused && (mTransferQuotaState == QuotaState::FULL || mTransferQuotaState == QuotaState::OVERQUOTA
            || (mStorageQuotaState == MegaApi::STORAGE_STATE_PAYWALL
            || mStorageQuotaState == MegaApi::STORAGE_STATE_RED)))
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
    if (processedNumber == 0 && failedNumber == 0)
    {
        leftFooterWidget = mUi->pUpToDate;
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
                mSpeedRefreshTimer->start(std::chrono::milliseconds(SPEED_REFRESH_PERIOD_MS));
            }
        }

        if(processedNumber != 0)
        {
            leftFooterWidget = mUi->pSpeedAndClear;

            countLabel->show();
            countLabel->setText(countLabelText);
        }
        else
        {
            countLabel->hide();
            countLabel->clear();
        }

        if(failedNumber != 0)
        {
            leftFooterWidget = mUi->pSomeIssues;
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
            long long number (mModel->getNumberOfTransfersForFileType(fileType));

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
            QuotaState tQuotaState (qobject_cast<MegaApplication*>(qApp)->getTransferQuotaState());
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
        if(transfersView)
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
            mUi->pTransferOverQuota->setVisible(mStorageQuotaState != MegaApi::STORAGE_STATE_PAYWALL
                                                && mStorageQuotaState != MegaApi::STORAGE_STATE_RED);
            break;
        }
        case QuotaState::OK:
        case QuotaState::WARNING:
        default:
        {
            mUi->pTransferOverQuota->hide();
            mUi->tSeePlans->setVisible(mStorageQuotaState == MegaApi::STORAGE_STATE_PAYWALL
                                       || mStorageQuotaState == MegaApi::STORAGE_STATE_RED);
            break;
        }
    }

    checkPauseButtonVisibilityIfPossible();
}

void TransferManager::checkPauseButtonVisibilityIfPossible()
{
    mUi->lPaused->setVisible(mModel->areAllPaused() && !mUi->lStorageOverQuota->isVisible() && !mUi->pTransferOverQuota->isVisible());
}

void TransferManager::refreshSpeed()
{
    mUi->bUpSpeed->setVisible(mTransfersCount.pendingUploads);
    if(mTransfersCount.pendingUploads)
    {
        auto upSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentUploadSpeed()));
        mUi->bUpSpeed->setText(Utilities::getSizeString(upSpeed) + QLatin1Literal("/s"));
    }

    mUi->bDownSpeed->setVisible(mTransfersCount.pendingDownloads);
    if(mTransfersCount.pendingDownloads)
    {
        auto dlSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentDownloadSpeed()));
        mUi->bDownSpeed->setText(Utilities::getSizeString(dlSpeed) + QLatin1Literal("/s"));
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

        if(mUi->tDlResults->property(LABEL_NUMBER).toLongLong() != nbDl)
        {
            QString downloadText(tr("Downloads"));
            mUi->tDlResults->setText(QString(downloadText + QString::fromUtf8("\t\t\t\t") + QString::number(nbDl)));
            mUi->tDlResults->setProperty(LABEL_NUMBER, nbDl);
        }

        if(mUi->tUlResults->property(LABEL_NUMBER).toLongLong() != nbUl)
        {
            QString uploadText(tr("Uploads"));
            mUi->tUlResults->setText(QString(uploadText + QString::fromUtf8("\t\t\t\t") + QString::number(nbUl)));
            mUi->tUlResults->setProperty(LABEL_NUMBER, nbUl);
        }

        int intNbAll = static_cast<int>(nbAll);
        mUi->lNbResults->setText(QString(tr("%1 result(s) found","", intNbAll)).arg(nbAll));
        mUi->lNbResults->setProperty("results", static_cast<bool>(nbAll));
        mUi->lNbResults->style()->unpolish(mUi->lNbResults);
        mUi->lNbResults->style()->polish(mUi->lNbResults);

        QString allText(tr("All"));
        mUi->tAllResults->setText(allText + QString::fromUtf8("\t\t\t\t") + QString::number(nbAll));

        mUi->searchByTextTypeSelector->setVisible(nbDl != 0 && nbUl != 0);

        bool showTypeFilters (mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB);
        mUi->tDlResults->setVisible(showTypeFilters);
        mUi->tUlResults->setVisible(showTypeFilters);
        mUi->tAllResults->setVisible(showTypeFilters);

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
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
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

    //Use to repaint and update the transfers state
    auto transfersView = findChild<MegaTransferView*>();
    if(transfersView)
    {
        transfersView->update();
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
        mUi->bSearchString->setText(mUi->bSearchString->fontMetrics()
                                    .elidedText(pattern,
                                                Qt::ElideMiddle,
                                                mUi->bSearchString->width()));
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
    mUi->tAllResults->setChecked(true);
    mUi->tAllResults->hide();
    mUi->tDlResults->hide();
    mUi->tUlResults->hide();
    mUi->wSearch->show();

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
    applyTextSearch(mUi->lTextSearch->text());
}

void TransferManager::on_tSearchCancel_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pTransfers);
    mUi->leSearchField->setText(tr("Search"));
}

void TransferManager::on_tClearSearchResult_clicked()
{
    mUi->wSearch->hide();
    mUi->bSearchString->setText(QString());
    if (mUi->wTransfers->getCurrentTab() == TransfersWidget::SEARCH_TAB)
    {
        mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
        mUi->sCurrentContentInfo->setCurrentWidget(mUi->pStatusHeaderInfo);
        on_tAllTransfers_clicked();
    }
}

void TransferManager::on_tAllResults_clicked()
{
    mUi->wTransfers->textFilterTypeChanged({});
}

void TransferManager::on_tDlResults_clicked()
{
    mUi->wTransfers->textFilterTypeChanged(TransferData::TRANSFER_DOWNLOAD
                                    | TransferData::TRANSFER_LTCPDOWNLOAD);
}

void TransferManager::on_tUlResults_clicked()
{
    mUi->wTransfers->textFilterTypeChanged(TransferData::TRANSFER_UPLOAD);
}

void TransferManager::on_bArchives_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_ARCHIVE_TAB, Utilities::FileType::TYPE_ARCHIVE, tr("Archives"));
}

void TransferManager::on_bDocuments_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_DOCUMENT_TAB, Utilities::FileType::TYPE_DOCUMENT, tr("Documents"));
}

void TransferManager::on_bImages_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_IMAGE_TAB, Utilities::FileType::TYPE_IMAGE, tr("Images"));
}

void TransferManager::on_bAudio_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_AUDIO_TAB, Utilities::FileType::TYPE_AUDIO, tr("Audio"));
}

void TransferManager::on_bVideos_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_VIDEO_TAB, Utilities::FileType::TYPE_VIDEO, tr("Videos"));
}

void TransferManager::on_bOther_clicked()
{
    onFileTypeButtonClicked(TransfersWidget::TYPE_OTHER_TAB, Utilities::FileType::TYPE_OTHER, tr("Other"));
}

void TransferManager::onFileTypeButtonClicked(TransfersWidget::TM_TAB tab, Utilities::FileType fileType, const QString& tabLabel)
{
  if (mUi->wTransfers->getCurrentTab() != tab)
  {
        mUi->wTransfers->filtersChanged({}, {}, {fileType});
        mUi->lCurrentContent->setText(tabLabel);
        toggleTab(tab);
  }
}

void TransferManager::on_bOpenLinks_clicked()
{
    qobject_cast<MegaApplication*>(qApp)->importLinks();
}

void TransferManager::on_tCogWheel_clicked()
{
    qobject_cast<MegaApplication*>(qApp)->openSettings();
}

void TransferManager::on_bDownload_clicked()
{
    qobject_cast<MegaApplication*>(qApp)->downloadActionClicked();
}

void TransferManager::on_bUpload_clicked()
{
    qobject_cast<MegaApplication*>(qApp)->uploadActionClickedFromWindow(this);
}

void TransferManager::on_bCancelAll_clicked()
{
    auto transfersView = findChild<MegaTransferView*>();
    if(transfersView)
    {
        if(transfersView->onCancelAllTransfers())
        {
            on_tAllTransfers_clicked();

            //Use to repaint and update the transfers state
            transfersView->update();
        }
    };
}

void TransferManager::on_leSearchField_returnPressed()
{
    emit mUi->tSearchIcon->clicked();
}

void TransferManager::toggleTab(TransfersWidget::TM_TAB newTab)
{
    if (mUi->wTransfers->getCurrentTab() != newTab)
    {
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

        auto pushButton = mTabFramesToggleGroup[newTab]->findChild<QPushButton*>();
        if(pushButton)
        {
            pushButton->setChecked(true);
        }

        //The rest of cases
        if (mUi->wTransfers->getCurrentTab() == TransfersWidget::COMPLETED_TAB
                || mUi->wTransfers->getCurrentTab() == TransfersWidget::FAILED_TAB
                || (mUi->wTransfers->getCurrentTab() > TransfersWidget::TYPES_TAB_BASE && mUi->wTransfers->getCurrentTab() < TransfersWidget::TYPES_LAST))
        {
            long long transfers(0);

            if(mUi->wTransfers->getCurrentTab() == TransfersWidget::COMPLETED_TAB)
            {
                transfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
            }
            else if(mUi->wTransfers->getCurrentTab() == TransfersWidget::FAILED_TAB)
            {
                transfers = mTransfersCount.totalFailedTransfers();
                mUi->tActionButton->setText(tr("Retry all"));
            }
            else
            {
                Utilities::FileType fileType = static_cast<Utilities::FileType>(mUi->wTransfers->getCurrentTab() - TransfersWidget::TYPES_TAB_BASE);
                transfers = mModel->getNumberOfTransfersForFileType(fileType);
            }

            if(transfers == 0)
            {
                auto countLabel(mNumberLabelsGroup[mUi->wTransfers->getCurrentTab()]);
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
            mUi->sCurrentContentInfo->setCurrentWidget(mUi->pStatusHeaderInfo);
            mUi->wTransfers->textFilterChanged(QString());
        }

        mUi->wTransfers->setCurrentTab(newTab);

        refreshView();

        // Reload QSS because it is glitchy
        mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
    }
}

void TransferManager::refreshView()
{
    if (mUi->wTransfers->getCurrentTab() != TransfersWidget::NO_TAB)
    {
        QWidget* widgetToShow (mUi->wTransfers);

        if(mUi->wTransfers->getCurrentTab() != TransfersWidget::SEARCH_TAB)
        {
            auto countLabel = mNumberLabelsGroup[mUi->wTransfers->getCurrentTab()];

            if (countLabel->text().isEmpty())
            {
                widgetToShow = mTabNoItem[mUi->wTransfers->getCurrentTab()];
            }
        }

        updateTransferWidget(widgetToShow);
        checkActionAndMediaVisibility();
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
    else if(obj == mUi->leSearchField && event->type() == QEvent::KeyPress)
    {
        auto keyEvent = dynamic_cast<QKeyEvent*>(event);
        if(keyEvent && keyEvent->key() == Qt::Key_Escape)
        {
            event->accept();
            on_leSearchField_editingFinished();
            focusNextChild();
            return true;
        }
        else if(keyEvent && keyEvent->key() == Qt::Key_Return)
        {
            mSearchFieldReturnPressed = true;
        }
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
        emit aboutToClose();
        QDialog::closeEvent(event);
    }
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        setActiveTab(mUi->wTransfers->getCurrentTab());
        onUpdatePauseState(mUi->wTransfers->getProxyModel()->getPausedTransfers());
    }
    QDialog::changeEvent(event);
}

void TransferManager::mouseReleaseEvent(QMouseEvent *event)
{
    mUi->wTransfers->mouseRelease(event->globalPos());

    QDialog::mouseReleaseEvent(event);
}

void TransferManager::setTransferState(const StatusInfo::TRANSFERS_STATES &transferState)
{
    if(transferState == StatusInfo::TRANSFERS_STATES::STATE_INDEXING)
    {
        mScanningTimer.start();
        mUi->sStatus->setCurrentWidget(mUi->pScanning);
    }
    else
    {
        mScanningTimer.stop();
        mScanningAnimationIndex = 1;
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
    if (!mTransferScanCancelUi || !mTransferScanCancelUi->isActive())
    {
        if (mUi->sTransfers->currentWidget() != widgetToShow)
        {
            mUi->sTransfers->setCurrentWidget(widgetToShow);
        }
    }
}
