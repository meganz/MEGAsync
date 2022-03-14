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

using namespace mega;

const int TransferManager::SPEED_REFRESH_PERIOD_MS;
const int TransferManager::STATS_REFRESH_PERIOD_MS;

constexpr long long NB_INIT_VALUE = 0LL;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::TransferManager),
    mUiDragBackDrop(new Ui::TransferManagerDragBackDrop),
    mMegaApi(megaApi),
    mPreferences(Preferences::instance()),
    mModel(nullptr),
    mCurrentTab(NO_TAB),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr)),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this))
{
    mUi->setupUi(this);

    mDragBackDrop = new QWidget(this);
    mUiDragBackDrop->setupUi(mDragBackDrop);
    mDragBackDrop->hide();

    mUi->wTransfers->setupTransfers();
#ifndef Q_OS_MACOS
    setWindowFlags(windowFlags() | Qt::Window | Qt::FramelessWindowHint);
#endif
    setAttribute(Qt::WA_DeleteOnClose, true);

    mUi->lTextSearch->installEventFilter(this);

    mModel = mUi->wTransfers->getModel();

    Platform::enableDialogBlur(this);
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);

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

    mTabFramesToggleGroup[ALL_TRANSFERS_TAB] = mUi->fAllTransfers;
    mTabFramesToggleGroup[DOWNLOADS_TAB]     = mUi->fDownloads;
    mTabFramesToggleGroup[UPLOADS_TAB]       = mUi->fUploads;
    mTabFramesToggleGroup[COMPLETED_TAB]     = mUi->fCompleted;
    mTabFramesToggleGroup[SEARCH_TAB]        = mUi->fSearchString;
    mTabFramesToggleGroup[TYPE_OTHER_TAB]    = mUi->fOther;
    mTabFramesToggleGroup[TYPE_AUDIO_TAB]    = mUi->fMusic;
    mTabFramesToggleGroup[TYPE_VIDEO_TAB]    = mUi->fVideos;
    mTabFramesToggleGroup[TYPE_ARCHIVE_TAB]  = mUi->fArchives;
    mTabFramesToggleGroup[TYPE_DOCUMENT_TAB] = mUi->fDocuments;
    mTabFramesToggleGroup[TYPE_IMAGE_TAB]    = mUi->fImages;
    mTabFramesToggleGroup[TYPE_TEXT_TAB]     = mUi->fText;

    for (auto tabFrame : qAsConst(mTabFramesToggleGroup))
    {
        tabFrame->setProperty("itsOn", false);
    }

    mTabNoItem[ALL_TRANSFERS_TAB] = mUi->wNoTransfers;
    mTabNoItem[DOWNLOADS_TAB]     = mUi->wNoDownloads;
    mTabNoItem[UPLOADS_TAB]       = mUi->wNoUploads;
    mTabNoItem[COMPLETED_TAB]     = mUi->wNoFinished;
    mTabNoItem[SEARCH_TAB]        = mUi->wNoResults;
    mTabNoItem[TYPE_OTHER_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TYPE_AUDIO_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TYPE_VIDEO_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TYPE_ARCHIVE_TAB]  = mUi->wNoTransfers;
    mTabNoItem[TYPE_DOCUMENT_TAB] = mUi->wNoTransfers;
    mTabNoItem[TYPE_IMAGE_TAB]    = mUi->wNoTransfers;
    mTabNoItem[TYPE_TEXT_TAB]     = mUi->wNoTransfers;

    mNumberLabelsGroup[ALL_TRANSFERS_TAB]    = mUi->lAllTransfers;
    mNumberLabelsGroup[DOWNLOADS_TAB]        = mUi->lDownloads;
    mNumberLabelsGroup[UPLOADS_TAB]          = mUi->lUploads;
    mNumberLabelsGroup[COMPLETED_TAB]        = mUi->lCompleted;
    mNumberLabelsGroup[TYPE_OTHER_TAB]       = mUi->lOtherNb;
    mNumberLabelsGroup[TYPE_AUDIO_TAB]       = mUi->lMusicNb;
    mNumberLabelsGroup[TYPE_VIDEO_TAB]       = mUi->lVideosNb;
    mNumberLabelsGroup[TYPE_ARCHIVE_TAB]     = mUi->lArchivesNb;
    mNumberLabelsGroup[TYPE_DOCUMENT_TAB]    = mUi->lDocumentsNb;
    mNumberLabelsGroup[TYPE_IMAGE_TAB]       = mUi->lImagesNb;
    mNumberLabelsGroup[TYPE_TEXT_TAB]        = mUi->lTextNb;

    QMetaEnum tabs = QMetaEnum::fromType<TM_TAB>();

    for (int index = 0; index < tabs.keyCount(); index++)
    {
        auto value  = tabs.value(index);

        if(value > TYPES_TAB_BASE && value < TYPES_LAST)
        {
            TM_TAB tab = static_cast<TM_TAB>(value);
            mNumberLabelsGroup[tab]->parentWidget()->hide();
        }
    }

    connect(this, &TransferManager::showCompleted,
            mUi->wTransfers, &TransfersWidget::onShowCompleted);


    connect(mModel, &TransfersModel::pauseStateChanged,
            mUi->wTransfers, &TransfersWidget::onPauseStateChanged);
    connect(mModel, &TransfersModel::pauseStateChanged,
            this, &TransferManager::onUpdatePauseState);

    connect(mModel, &TransfersModel::transfersCountUpdated,
            this, &TransferManager::onTransfersDataUpdated);

    connect(mModel, &TransfersModel::pauseStateChangedByTransferResume,
            this, &TransferManager::onPauseStateChangedByTransferResume);

    connect(this, &TransferManager::clearCompletedTransfers,
            findChild<MegaTransferView*>(), &MegaTransferView::onClearCompletedVisibleTransfers);

    connect(mUi->wTransfers->getProxyModel(),
            &TransfersManagerSortFilterProxyModel::searchNumbersChanged,
            this, &TransferManager::refreshSearchStats);

    connect(mUi->wTransfers->getProxyModel(),
            &TransfersManagerSortFilterProxyModel::cancelableTransfersChanged,
            this, &TransferManager::checkCancelAllButtonVisibility);

    connect(mUi->wTransfers,
            &TransfersWidget::disableTransferManager,[this](bool state){
        setDisabled(state);
    });

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    // Connect to storage quota signals
    connect(qobject_cast<MegaApplication*>(qApp), &MegaApplication::storageStateChanged,
            this, &TransferManager::onStorageStateChanged,
            Qt::QueuedConnection);

    setAcceptDrops(true);

    // Init state
    onUpdatePauseState(mModel->areAllPaused());
    auto storageState = MegaSyncApp->getAppliedStorageState();
    auto transferQuotaState = MegaSyncApp->getTransferQuotaState();
    onStorageStateChanged(storageState);
    onTransferQuotaStateChanged(transferQuotaState);

    if(storageState == MegaApi::STORAGE_STATE_PAYWALL || storageState ==
             MegaApi::STORAGE_STATE_RED || transferQuotaState == QuotaState::FULL)
    {
        mUi->lPaused->hide();
    }

    setActiveTab(ALL_TRANSFERS_TAB);
    //Update stats
    onTransfersDataUpdated();

    // Refresh Style, QSS is glitchy on first start???
    auto tabFrame (mTabFramesToggleGroup[mCurrentTab]);
    tabFrame->style()->unpolish(tabFrame);
    tabFrame->style()->polish(tabFrame);
    const auto children (tabFrame->findChildren<QWidget*>());
    for (auto w : children)
    {
        w->style()->unpolish(w);
        w->style()->polish(w);
    }
}

void TransferManager::pauseModel(bool value)
{
    mModel->pauseModelProcessing(value);
}

void TransferManager::onPauseStateChangedByTransferResume()
{
    onUpdatePauseState(false);
}

void TransferManager::setActiveTab(int t)
{
    switch (t)
    {
        case DOWNLOADS_TAB:
            on_tDownloads_clicked();
            break;
        case UPLOADS_TAB:
            on_tUploads_clicked();
            break;
        case COMPLETED_TAB:
            on_tCompleted_clicked();
            break;
        case SEARCH_TAB:
            on_tSearchIcon_clicked();
            break;
        case ALL_TRANSFERS_TAB:
        default:
            on_tAllTransfers_clicked();
            break;
    }
}

TransferManager::~TransferManager()
{
    delete mUi;
}

void TransferManager::on_tCompleted_clicked()
{
    if (mCurrentTab != COMPLETED_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged({}, TransferData::FINISHED_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("Finished"));
        toggleTab(COMPLETED_TAB);
    }
}

void TransferManager::on_tDownloads_clicked()
{
    if (mCurrentTab != DOWNLOADS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged((TransferData::TRANSFER_DOWNLOAD
                                         | TransferData::TRANSFER_LTCPDOWNLOAD),
                                        TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("Downloads"));
        toggleTab(DOWNLOADS_TAB);
    }
}

void TransferManager::on_tUploads_clicked()
{
    if (mCurrentTab != UPLOADS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged(TransferData::TRANSFER_UPLOAD, TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("Uploads"));
        toggleTab(UPLOADS_TAB);
    }
}

void TransferManager::on_tAllTransfers_clicked()
{
    if (mCurrentTab != ALL_TRANSFERS_TAB)
    {
        emit userActivity();
        mUi->wTransfers->filtersChanged({}, TransferData::ACTIVE_STATES_MASK, {});
        mUi->lCurrentContent->setText(tr("All Transfers"));

        toggleTab(ALL_TRANSFERS_TAB);
    }
}

void TransferManager::onUpdatePauseState(bool isPaused)
{
    if (isPaused)
    {
        static const QIcon icon(QLatin1String(":/images/sidebar_resume_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Resume All"));
        mUi->lPaused->setText(tr("Paused"));
    }
    else
    {
        static const QIcon icon(QLatin1String(":/images/sidebar_pause_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Pause All"));
        mUi->lPaused->setText(QString());
    }
}

void TransferManager::checkCancelAllButtonVisibility()
{
    auto proxy (mUi->wTransfers->getProxyModel());

    auto sizePolicy = mUi->bCancelClearAll->sizePolicy();
    if(!sizePolicy.retainSizeWhenHidden())
    {
        sizePolicy.setRetainSizeWhenHidden(true);
        mUi->bCancelClearAll->setSizePolicy(sizePolicy);
    }

    //Get the most updated transferCount
    mTransfersCount = mModel->getTransfersCount();
    if((mTransfersCount.completedDownloads() + mTransfersCount.completedUploadBytes) == 0 && !proxy->isAnyCancelable())
    {
        mUi->bCancelClearAll->setVisible(false);
    }
    else
    {
        mUi->bCancelClearAll->setVisible(true);
    }
}

void TransferManager::refreshStateStats()
{
    QLabel* countLabel (nullptr);
    QString countLabelText;
    long long processedNumber (0LL);

    // First check Finished states -----------------------------------------------------------------
    countLabel = mNumberLabelsGroup[COMPLETED_TAB];

    processedNumber = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
    countLabelText = processedNumber > 0 ? QString::number(processedNumber) : QString();

    // Update if the value changed
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        if (mCurrentTab != COMPLETED_TAB && processedNumber == 0)
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

    // Then Active states --------------------------------------------------------------------------
    countLabel = mNumberLabelsGroup[ALL_TRANSFERS_TAB];

    processedNumber = mTransfersCount.pendingDownloads + mTransfersCount.pendingUploads;
    countLabelText = processedNumber > 0 ? QString::number(processedNumber) : QString();

    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        QWidget* leftFooterWidget (nullptr);

        // If we don't have transfers, stop refresh timer and show "Up to date",
        // and if current tab is ALL TRANSFERS, show empty.
        if (processedNumber == 0)
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
                leftFooterWidget = mUi->pSpeedAndClear;
                if(!mSpeedRefreshTimer->isActive())
                {
                   mSpeedRefreshTimer->start(std::chrono::milliseconds(SPEED_REFRESH_PERIOD_MS));
                }
            }

            countLabel->show();
            countLabel->setText(countLabelText);
        }

        if(leftFooterWidget)
        {
            mUi->sStatus->setCurrentWidget(leftFooterWidget);
        }
    }
}

void TransferManager::refreshTypeStats()
{
    auto downloadTransfers = mTransfersCount.pendingDownloads;

    auto countLabel = mNumberLabelsGroup[DOWNLOADS_TAB];
    QString countLabelText(downloadTransfers > 0 ? QString::number(downloadTransfers) : QString());

    // First check Downloads -----------------------------------------------------------------------
    if (countLabel->text().isEmpty() || countLabelText != countLabel->text())
    {
        countLabel->setText(countLabelText);
    }

    countLabel->setVisible(!countLabelText.isEmpty());


    auto uploadTransfers = mTransfersCount.pendingUploads;

    countLabel = mNumberLabelsGroup[UPLOADS_TAB];
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
    QMetaEnum tabs = QMetaEnum::fromType<TM_TAB>();

    for (int index = 0; index < tabs.keyCount(); index++)
    {
        auto value  = tabs.value(index);

        if(value > TYPES_TAB_BASE && value < TYPES_LAST)
        {
            Utilities::FileType fileType = static_cast<Utilities::FileType>(value - TYPES_TAB_BASE);
            long long number (mModel->getNumberOfTransfersForFileType(fileType));

            QString countLabelText(number > 0 ? QString::number(number) : QString());

            TM_TAB tab = static_cast<TM_TAB>(value);
            QLabel* label (mNumberLabelsGroup[tab]);
            if (mCurrentTab != tab && number == 0)
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
    switch (storageState)
    {
        case MegaApi::STORAGE_STATE_PAYWALL:
        case MegaApi::STORAGE_STATE_RED:
        {
            mUi->tSeePlans->show();
            mUi->lStorageOverQuota->show();
            mUi->lPaused->setVisible(false);
            break;
        }
        case MegaApi::STORAGE_STATE_GREEN:
        case MegaApi::STORAGE_STATE_ORANGE:
        case MegaApi::STORAGE_STATE_UNKNOWN:
        default:
        {
            mUi->lStorageOverQuota->hide();
            mUi->lPaused->setVisible(mModel->areAllPaused());
            QuotaState tQuotaState (qobject_cast<MegaApplication*>(qApp)->getTransferQuotaState());
            mUi->tSeePlans->setVisible(tQuotaState == QuotaState::FULL);
            break;
        }
    }
}

void TransferManager::onTransferQuotaStateChanged(QuotaState transferQuotaState)
{
    switch (transferQuotaState)
    {
        case QuotaState::FULL:
        {
            mUi->tSeePlans->show();
            mUi->lTransferOverQuota->show();
            mUi->lPaused->setVisible(false);
            break;
        }
        case QuotaState::OK:
        case QuotaState::WARNING:
        default:
        {
            mUi->lTransferOverQuota->hide();
            mUi->lPaused->setVisible(mModel->areAllPaused());
            int storageState (qobject_cast<MegaApplication*>(qApp)->getAppliedStorageState());
            mUi->tSeePlans->setVisible(storageState == MegaApi::STORAGE_STATE_PAYWALL
                                       || storageState == MegaApi::STORAGE_STATE_RED);
            break;
        }
    }
}

void TransferManager::refreshSpeed()
{
    auto upSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentUploadSpeed()));
    auto dlSpeed (static_cast<unsigned long long>(mMegaApi->getCurrentDownloadSpeed()));
    mUi->bUpSpeed->setText(Utilities::getSizeString(upSpeed) + QLatin1Literal("/s"));
    mUi->bDownSpeed->setText(Utilities::getSizeString(dlSpeed) + QLatin1Literal("/s"));
}

void TransferManager::refreshSearchStats()
{
    if(mCurrentTab == SEARCH_TAB)
    {
        // Update search results number
        auto proxy (mUi->wTransfers->getProxyModel());
        long long nbDl (proxy->getNumberOfItems(TransferData::TRANSFER_DOWNLOAD));
        long long nbUl (proxy->getNumberOfItems(TransferData::TRANSFER_UPLOAD));
        long long nbAll (nbDl + nbUl);

        if(mUi->tDlResults->property("NUMBER").toLongLong() != nbDl)
        {
            mUi->tDlResults->setText(QString(tr("Downloads\t\t\t\t%1")).arg(nbDl));
            mUi->tDlResults->setProperty("NUMBER", nbDl);
        }

        if(mUi->tUlResults->property("NUMBER").toLongLong() != nbUl)
        {
            mUi->tUlResults->setText(QString(tr("Uploads\t\t\t\t%1")).arg(nbUl));
            mUi->tDlResults->setProperty("NUMBER", nbUl);
        }

        if(mUi->tAllResults->property("NUMBER").toLongLong() != nbAll)
        {
            mUi->lNbResults->setText(QString(tr("%1 results found")).arg(nbAll));
            mUi->lNbResults->setProperty("results", bool(nbAll));
            mUi->lNbResults->style()->unpolish(mUi->lNbResults);
            mUi->lNbResults->style()->polish(mUi->lNbResults);
            mUi->tAllResults->setText(QString(tr("All\t\t\t\t%1")).arg(nbAll));
            mUi->tAllResults->setProperty("NUMBER", nbAll);
        }

        mUi->searchByTextTypeSelector->setVisible(nbDl != 0 && nbUl != 0);

        bool showTypeFilters (mCurrentTab == SEARCH_TAB);
        mUi->tDlResults->setVisible(showTypeFilters);
        mUi->tUlResults->setVisible(showTypeFilters);
        mUi->tAllResults->setVisible(showTypeFilters);

        QWidget* widgetToShow (mUi->wTransfers);

        if (nbAll == 0)
        {
            widgetToShow = mTabNoItem[mCurrentTab];
        }

        if (mUi->sTransfers->currentWidget() != widgetToShow)
        {
            mUi->sTransfers->setCurrentWidget(widgetToShow);
        }
    }
}

void TransferManager::disableGetLink(bool disable)
{
    mUi->wTransfers->disableGetLink(disable);
}

void TransferManager::on_tClearCompleted_clicked()
{
    emit clearCompletedTransfers();
    mUi->tClearCompleted->hide();
}

void TransferManager::on_tSeePlans_clicked()
{
    QString url = QString::fromUtf8("mega://#pro");
    Utilities::getPROurlWithParameters(url);
    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
}

void TransferManager::on_bPause_clicked()
{
    auto newState = !mModel->areAllPaused();
    mModel->pauseResumeAllTransfers(newState);
    onUpdatePauseState(newState);
}

void TransferManager::on_bSearch_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pSearch);
    mUi->leSearchField->setText(QString());
}

void TransferManager::on_tSearchIcon_clicked()
{
    QString pattern (mUi->leSearchField->text());

    if (pattern != QString())
    {
        mUi->bSearchString->setText(mUi->bSearchString->fontMetrics()
                                    .elidedText(pattern,
                                                Qt::ElideMiddle,
                                                mUi->bSearchString->width() - 24));
        applyTextSearch(pattern);
    }
}

void TransferManager::applyTextSearch(const QString& text)
{
    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                              .elidedText(text,
                                          Qt::ElideMiddle,
                                          mUi->lTextSearch->width() - 24));
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

    toggleTab(SEARCH_TAB);
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
    if (mCurrentTab == SEARCH_TAB)
    {
        mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
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
    if (mCurrentTab != TYPE_ARCHIVE_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_ARCHIVE});
        mUi->lCurrentContent->setText(tr("Archives"));
        toggleTab(TYPE_ARCHIVE_TAB);
    }
}

void TransferManager::on_bDocuments_clicked()
{
    if (mCurrentTab != TYPE_DOCUMENT_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_DOCUMENT});
        mUi->lCurrentContent->setText(tr("Documents"));
        toggleTab(TYPE_DOCUMENT_TAB);
    }
}

void TransferManager::on_bImages_clicked()
{
    if (mCurrentTab != TYPE_IMAGE_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_IMAGE});
        mUi->lCurrentContent->setText(tr("Images"));
        toggleTab(TYPE_IMAGE_TAB);
    }
}

void TransferManager::on_bMusic_clicked()
{
    if (mCurrentTab != TYPE_AUDIO_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_AUDIO});
        mUi->lCurrentContent->setText(tr("Music"));
        toggleTab(TYPE_AUDIO_TAB);
    }
}

void TransferManager::on_bVideos_clicked()
{
    if (mCurrentTab != TYPE_VIDEO_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_VIDEO});
        mUi->lCurrentContent->setText(tr("Videos"));
        toggleTab(TYPE_VIDEO_TAB);
    }
}

void TransferManager::on_bOther_clicked()
{
    if (mCurrentTab != TYPE_OTHER_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_OTHER});
        mUi->lCurrentContent->setText(tr("Other"));
        toggleTab(TYPE_OTHER_TAB);
    }
}

void TransferManager::on_bText_clicked()
{
    if (mCurrentTab != TYPE_TEXT_TAB)
    {
        mUi->wTransfers->filtersChanged({}, {}, {Utilities::FileType::TYPE_TEXT});
        mUi->lCurrentContent->setText(tr("Text"));
        toggleTab(TYPE_TEXT_TAB);
    }
}

void TransferManager::on_bImportLinks_clicked()
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
    qobject_cast<MegaApplication*>(qApp)->uploadActionClicked();
}

void TransferManager::on_bCancelClearAll_clicked()
{
    mUi->wTransfers->cancelClearAll();
}

void TransferManager::on_leSearchField_returnPressed()
{
    emit mUi->tSearchIcon->clicked();
}

void TransferManager::toggleTab(TM_TAB newTab)
{
    if (mCurrentTab != newTab)
    {
        // De-activate old tab frame
        if (mCurrentTab != NO_TAB)
        {
            mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        }

        // Activate new tab frame
        mTabFramesToggleGroup[newTab]->setProperty("itsOn", true);
        mTabFramesToggleGroup[newTab]->setGraphicsEffect(mShadowTab);

        // Show pause button on tab except completed tab,
        // and set Clear All button string,
        // Emit wether we are showing completed or not
        if (newTab == COMPLETED_TAB)
        {
            mUi->tClearCompleted->setText(tr("Clear All"));
            mUi->bPause->hide();
            emit showCompleted(true);
        }
        else if (mCurrentTab == COMPLETED_TAB)
        {
            auto completedTransfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
            if(completedTransfers == 0)
            {
                auto countLabel(mNumberLabelsGroup[mCurrentTab]);
                if(countLabel->parentWidget()->isVisible())
                {
                    countLabel->parentWidget()->hide();
                }
            }

            mUi->tClearCompleted->setText(tr("Clear Completed"));
            mUi->bPause->show();
            emit showCompleted(false);
        }
        else if(mCurrentTab > TYPES_TAB_BASE && mCurrentTab < TYPES_LAST)
        {
            Utilities::FileType fileType = static_cast<Utilities::FileType>(mCurrentTab - TYPES_TAB_BASE);
            long long number (mModel->getNumberOfTransfersForFileType(fileType));

            if (number == 0)
            {
                auto countLabel(mNumberLabelsGroup[mCurrentTab]);
                if(countLabel->parentWidget()->isVisible())
                {
                    countLabel->parentWidget()->hide();
                }
            }
        }

        // Set current header widget: search or not
        if (newTab == SEARCH_TAB)
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pSearchHeader);
        }
        else
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
            mUi->wTransfers->textFilterChanged(QString());
        }

        mCurrentTab = newTab;

        refreshView();

        // Reload QSS because it is glitchy
        mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
    }
}

void TransferManager::refreshView()
{
    if (mCurrentTab != NO_TAB)
    {
        QWidget* widgetToShow (mUi->wTransfers);

        if(mCurrentTab != SEARCH_TAB)
        {
            auto countLabel = mNumberLabelsGroup[mCurrentTab];

            if (countLabel->text().isEmpty())
            {
                widgetToShow = mTabNoItem[mCurrentTab];
            }
        }

        if (mUi->sTransfers->currentWidget() != widgetToShow)
        {
            mUi->sTransfers->setCurrentWidget(widgetToShow);
        }

        auto completedTransfers = mTransfersCount.completedDownloads() + mTransfersCount.completedUploads();
        auto allTransfers = mTransfersCount.pendingDownloads + mTransfersCount.pendingUploads;

        // Show "Clear All/Completed" if there are any completed transfers
        // (only for completed tab and individual media tabs)
        if ((mCurrentTab == COMPLETED_TAB && completedTransfers > 0)
                || (mCurrentTab >= TYPES_TAB_BASE && mModel->getNumberOfFinishedForFileType(
                        static_cast<Utilities::FileType>(mCurrentTab - TYPES_TAB_BASE))))
        {
            mUi->tClearCompleted->show();
        }
        else
        {
            mUi->tClearCompleted->hide();
        }

        // Hide Media groupbox if no transfers (active or finished)
        if (mCurrentTab >= TYPES_TAB_BASE
                || (allTransfers +
                    completedTransfers > 0))
        {
            mUi->wMediaType->show();
        }
        else
        {
            mUi->wMediaType->hide();
        }
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
    return false;
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        setActiveTab(mCurrentTab);
        onUpdatePauseState(mUi->wTransfers->getProxyModel()->getPausedTransfers());
    }
    QDialog::changeEvent(event);
}

void TransferManager::dropEvent(QDropEvent* event)
{
    mDragBackDrop->hide();
    QDialog::dropEvent(event);

    QQueue<QString> pathsToAdd;
    QList<QUrl> urlsToAdd = event->mimeData()->urls();
    foreach(auto& urlToAdd, urlsToAdd)
    {
        pathsToAdd.append(urlToAdd.toLocalFile());
    }

    MegaSyncApp->shellUpload(pathsToAdd);
}

void TransferManager::dragEnterEvent(QDragEnterEvent *event)
{
    if(event->mimeData()->hasUrls())
    {
        event->accept();
        mDragBackDrop->show();
        mDragBackDrop->resize(size());
    }

    QDialog::dragEnterEvent(event);
}

void TransferManager::dragLeaveEvent(QDragLeaveEvent *event)
{
    mDragBackDrop->hide();

    QDialog::dragLeaveEvent(event);
}
