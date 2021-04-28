#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "platform/Platform.h"

#include "MegaTransferDelegate2.h"
#include "MegaTransferView.h"

#include <QMouseEvent>

using namespace mega;

const int TransferManager::SPEED_REFRESH_PERIOD_MS;
const int TransferManager::STATS_REFRESH_PERIOD_MS;

static const QSet<TransferData::TransferState> ACTIVE_STATES =
{
    TransferData::TransferState::TRANSFER_ACTIVE,
    TransferData::TransferState::TRANSFER_PAUSED,
    TransferData::TransferState::TRANSFER_COMPLETING,
    TransferData::TransferState::TRANSFER_QUEUED,
    TransferData::TransferState::TRANSFER_RETRYING
};
static const TransferData::TransferStates ACTIVE_STATES_MASK =
{
    TransferData::TransferState::TRANSFER_ACTIVE |
    TransferData::TransferState::TRANSFER_PAUSED |
    TransferData::TransferState::TRANSFER_COMPLETING |
    TransferData::TransferState::TRANSFER_QUEUED |
    TransferData::TransferState::TRANSFER_RETRYING
};
static const QSet<TransferData::TransferState> FINISHED_STATES =
{
    TransferData::TransferState::TRANSFER_COMPLETED,
    TransferData::TransferState::TRANSFER_FAILED,
    TransferData::TransferState::TRANSFER_CANCELLED
};
static const TransferData::TransferStates FINISHED_STATES_MASK =
{
    TransferData::TransferState::TRANSFER_COMPLETED |
    TransferData::TransferState::TRANSFER_FAILED |
    TransferData::TransferState::TRANSFER_CANCELLED
};

constexpr long long NB_INIT_VALUE = -1LL;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::TransferManager),
    mMegaApi(megaApi),
    mPreferences(Preferences::instance()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mModel(nullptr),
    mCurrentTab(NO_TAB),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr)),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this))
{
    mUi->setupUi(this);
    mUi->wTransfers->setupTransfers();

    mUi->lTextSearch->installEventFilter(this);

    mModel = mUi->wTransfers->getModel2();

    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    Platform::enableDialogBlur(this);
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);

    mUi->wSearch->hide();
    mUi->wMediaType->hide();
    mUi->fCompleted->hide();

    mUi->sStatus->setCurrentWidget(mUi->pUpToDate);

    QColor shadowColor (188, 188, 188);
    mShadowTab->setParent(mUi->wTransferring);
    mShadowTab->setBlurRadius(8.);
    mShadowTab->setXOffset(0.);
    mShadowTab->setYOffset(0.);
    mShadowTab->setColor(shadowColor);
    mShadowTab->setEnabled(true);

    mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[DOWNLOADS_TAB]     = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[UPLOADS_TAB]       = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[COMPLETED_TAB]     = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[SEARCH_TAB]        = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_OTHER_TAB]    = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_AUDIO_TAB]    = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_VIDEO_TAB]    = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_ARCHIVE_TAB]  = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_DOCUMENT_TAB] = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_IMAGE_TAB]    = NB_INIT_VALUE;
    mNumberOfTransfersPerTab[TYPE_TEXT_TAB]     = NB_INIT_VALUE;

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

    mUi->lAllTransfers->hide();
    mUi->lDownloads->hide();
    mUi->lUploads->hide();
    mUi->tClearCompleted->hide();

    mMediaNumberLabelsGroup[TransferData::TYPE_OTHER]    = mUi->lOtherNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_AUDIO]    = mUi->lMusicNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_VIDEO]    = mUi->lVideosNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_ARCHIVE]  = mUi->lArchivesNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_DOCUMENT] = mUi->lDocumentsNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_IMAGE]    = mUi->lImagesNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_TEXT]     = mUi->lTextNb;

    for (auto mediaLabel : qAsConst(mMediaNumberLabelsGroup))
    {
        mediaLabel->parentWidget()->hide();
    }

    connect(this, &TransferManager::showCompleted,
            mUi->wTransfers, &TransfersWidget::onShowCompleted, Qt::QueuedConnection);

    connect(mModel, &QTransfersModel2::pauseStateChanged,
            mUi->wTransfers, &TransfersWidget::onPauseStateChanged, Qt::QueuedConnection);

    connect(mModel, &QTransfersModel2::transfersInModelChanged,
            this, &TransferManager::onTransfersInModelChanged, Qt::QueuedConnection);

    connect(mModel, &QTransfersModel2::pauseStateChanged,
            this, &TransferManager::onUpdatePauseState, Qt::QueuedConnection);

    connect(mUi->bPause, &QToolButton::clicked,
            mModel, &QTransfersModel2::pauseResumeAllTransfers, Qt::QueuedConnection);

    connect(this, &TransferManager::cancelClearAllTransfers,
            mModel, &QTransfersModel2::cancelClearAllTransfers, Qt::QueuedConnection);

    connect(this, &TransferManager::cancelClearAllRows,
            findChild<MegaTransferView*>(), &MegaTransferView::onCancelClearAllRows,
            Qt::QueuedConnection);

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    mStatsRefreshTimer->setSingleShot(false);
    connect(mStatsRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshStats);

    onTransfersInModelChanged(true);
    onUpdatePauseState(mModel->areAllPaused());
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
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged(FINISHED_STATES_MASK);
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Finished"));
        toggleTab(COMPLETED_TAB);
    }
}

void TransferManager::on_tDownloads_clicked()
{
    if (mCurrentTab != DOWNLOADS_TAB)
    {
        emit userActivity();
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES_MASK);
            mUi->wTransfers->transferTypeFilterChanged(TransferData::TransferType::TRANSFER_DOWNLOAD
                                                       | TransferData::TransferType::TRANSFER_LTCPDOWNLOAD);
            mUi->wTransfers->fileTypeFilterChanged({});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Downloads"));
        toggleTab(DOWNLOADS_TAB);
    }
}

void TransferManager::on_tUploads_clicked()
{
    if (mCurrentTab != UPLOADS_TAB)
    {
        emit userActivity();
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES_MASK);
            mUi->wTransfers->transferTypeFilterChanged(TransferData::TransferType::TRANSFER_UPLOAD);
            mUi->wTransfers->fileTypeFilterChanged({});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Uploads"));
        toggleTab(UPLOADS_TAB);
    }
}

void TransferManager::on_tAllTransfers_clicked()
{
    if (mCurrentTab != ALL_TRANSFERS_TAB)
    {
        emit userActivity();
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES_MASK);
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({});
            mUi->wTransfers->transferFilterApply();
        });
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

bool TransferManager::refreshStateStats()
{
    QLabel* label (nullptr);
    bool weHaveTransfers (true);
    long long processedNumber (0LL);

    // First check Finished states -----------------------------------------------------------------
    label = mUi->lCompleted;
    for (auto state : FINISHED_STATES)
    {
        processedNumber += mModel->getNumberOfTransfersForState(state);
    }
    weHaveTransfers = processedNumber;

    // Update if the value changed
    if (processedNumber != mNumberOfTransfersPerTab[COMPLETED_TAB])
    {
        label->parentWidget()->setVisible(weHaveTransfers || mCurrentTab == COMPLETED_TAB);
        label->setVisible(weHaveTransfers);
        label->setText(QString::number(processedNumber));

        mNumberOfTransfersPerTab[COMPLETED_TAB] = processedNumber;
    }

    // Then Active states --------------------------------------------------------------------------
    processedNumber = 0LL;
    label = mUi->lAllTransfers;
    for (auto state : ACTIVE_STATES)
    {
        processedNumber += mModel->getNumberOfTransfersForState(state);
    }
    weHaveTransfers |= static_cast<bool>(processedNumber);

    if (processedNumber != mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB])
    {
        QWidget* leftFooterWidget (nullptr);

        // If we don't have transfers, stop refresh timer and show "Up to date",
        // and if current tab is ALL TRANSFERS, show empty.
        if (processedNumber == 0)
        {
            leftFooterWidget = mUi->pUpToDate;
            mSpeedRefreshTimer->stop();
            label->hide();

            // Force number to -1 to re-init timer when a transfer comes
            mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] = NB_INIT_VALUE;
        }
        else
        {
            // If we didn't have transfers, launch timer and show speed.
            if (mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] <= 0)
            {
                leftFooterWidget = mUi->pSpeedAndClear;
                mSpeedRefreshTimer->start(std::chrono::milliseconds(SPEED_REFRESH_PERIOD_MS));
                label->show();
            }

            label->setText(QString::number(processedNumber));
            mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] = processedNumber;
        }

        mUi->sStatus->setCurrentWidget(leftFooterWidget);
    }
    return weHaveTransfers;
}

void TransferManager::refreshTypeStats()
{
    // First check Downloads -----------------------------------------------------------------------
    auto number (mModel->getNumberOfTransfersForType(TransferData::TransferType::TRANSFER_DOWNLOAD));
    if (number != mNumberOfTransfersPerTab[DOWNLOADS_TAB])
    {
        mUi->lDownloads->setVisible(number);
        mUi->lDownloads->setText(QString::number(number));
        mNumberOfTransfersPerTab[DOWNLOADS_TAB] = number;
    }

    // Then Uploads --------------------------------------------------------------------------------
    number = mModel->getNumberOfTransfersForType(TransferData::TransferType::TRANSFER_UPLOAD);
    if (number != mNumberOfTransfersPerTab[UPLOADS_TAB])
    {
        mUi->lUploads->setVisible(number);
        mUi->lUploads->setText(QString::number(number));
        mNumberOfTransfersPerTab[UPLOADS_TAB] = number;
    }
}

void TransferManager::refreshFileTypesStats()
{
    // Loop through file types and update if needed
    const auto fileTypes (mMediaNumberLabelsGroup.keys());
    for (auto fileType : fileTypes)
    {
        TM_TAB tab (static_cast<TM_TAB>(TYPES_TAB_BASE + fileType));
        long long number (mModel->getNumberOfTransfersForFileType(fileType));
        if (number != mNumberOfTransfersPerTab[tab])
        {
            QLabel* label (mMediaNumberLabelsGroup[fileType]);
            label->setVisible(number);
            label->setText(QString::number(number));
            label->parentWidget()->setVisible(number || mCurrentTab == tab);
            mNumberOfTransfersPerTab[tab] = number;
        }
    }
}

void TransferManager::onTransfersInModelChanged(bool weHaveTransfers)
{
    // (De)activate stats refresh if we have transfers (or not)
    if (weHaveTransfers)
    {
        mStatsRefreshTimer->start(std::chrono::milliseconds(STATS_REFRESH_PERIOD_MS));
    }
    else
    {
        mStatsRefreshTimer->stop();
        mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[DOWNLOADS_TAB]     = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[UPLOADS_TAB]       = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[COMPLETED_TAB]     = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[SEARCH_TAB]        = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_OTHER_TAB]    = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_AUDIO_TAB]    = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_VIDEO_TAB]    = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_ARCHIVE_TAB]  = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_DOCUMENT_TAB] = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_IMAGE_TAB]    = NB_INIT_VALUE;
        mNumberOfTransfersPerTab[TYPE_TEXT_TAB]     = NB_INIT_VALUE;
    }

    // Refresh stats
    refreshTypeStats();
    refreshFileTypesStats();
    refreshSearchStats();
    refreshStateStats();
    refreshView();
}

void TransferManager::refreshSpeed()
{
    mUi->bUpSpeed->setText(Utilities::getSizeString(mMegaApi->getCurrentUploadSpeed())
                           + QLatin1Literal("/s"));
    mUi->bDownSpeed->setText(Utilities::getSizeString(mMegaApi->getCurrentDownloadSpeed())
                             + QLatin1Literal("/s"));
}

void TransferManager::refreshStats()
{
    refreshTypeStats();
    refreshFileTypesStats();
    refreshSearchStats();
    if (!refreshStateStats())
    {
        onTransfersInModelChanged(false);
    }
    refreshView();
}

void TransferManager::refreshSearchStats()
{
    // Update search results number
    if (mCurrentTab == SEARCH_TAB)
    {    // Get number of transfers in the view
        auto nbRows (mUi->wTransfers->rowCount());
        if (nbRows != mNumberOfTransfersPerTab[SEARCH_TAB])
        {
            mUi->lNbResults->setText(QString(tr("%1 results")).arg(nbRows));
            mNumberOfTransfersPerTab[SEARCH_TAB] = nbRows;
        }
    }
}

void TransferManager::disableGetLink(bool disable)
{
    mUi->wTransfers->disableGetLink(disable);
}

void TransferManager::on_bClearAll_clicked()
{
    QPointer<TransferManager> dialog = QPointer<TransferManager>(this);

    if (QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel all transfers?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No)
            != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    emit cancelClearAllTransfers();
}

void TransferManager::on_tClearCompleted_clicked()
{
    emit cancelClearAllRows(false, true);
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
        mUi->wTransfers->transferFilterReset();
        mUi->wTransfers->textFilterChanged(pattern);

        mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                                  .elidedText(pattern,
                                              Qt::ElideMiddle,
                                              mUi->lTextSearch->width() - 24));
        // Add number of found results
        mUi->wSearch->show();

        toggleTab(SEARCH_TAB);
    }
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
    mUi->wTransfers->textFilterChanged(QString());
    mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
    on_tAllTransfers_clicked();
}

void TransferManager::on_bArchives_clicked()
{
    if (mCurrentTab != TYPE_ARCHIVE_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_ARCHIVE});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Archives"));
        toggleTab(TYPE_ARCHIVE_TAB);
    }
}

void TransferManager::on_bDocuments_clicked()
{
    if (mCurrentTab != TYPE_DOCUMENT_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_DOCUMENT});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Documents"));
        toggleTab(TYPE_DOCUMENT_TAB);
    }
}

void TransferManager::on_bImages_clicked()
{
    if (mCurrentTab != TYPE_IMAGE_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_IMAGE});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Images"));
        toggleTab(TYPE_IMAGE_TAB);
    }
}

void TransferManager::on_bMusic_clicked()
{
    if (mCurrentTab != TYPE_AUDIO_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_AUDIO});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Music"));
        toggleTab(TYPE_AUDIO_TAB);
    }
}

void TransferManager::on_bVideos_clicked()
{
    if (mCurrentTab != TYPE_VIDEO_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_VIDEO});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Videos"));
        toggleTab(TYPE_VIDEO_TAB);
    }
}

void TransferManager::on_bOther_clicked()
{
    if (mCurrentTab != TYPE_OTHER_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_OTHER});
            mUi->wTransfers->transferFilterApply();
        });
        mUi->lCurrentContent->setText(tr("Other"));
        toggleTab(TYPE_OTHER_TAB);
    }
}

void TransferManager::on_bText_clicked()
{
    if (mCurrentTab != TYPE_TEXT_TAB)
    {
        mThreadPool->push([=]
        {
            mUi->wTransfers->transferStateFilterChanged({});
            mUi->wTransfers->transferTypeFilterChanged({});
            mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_TEXT});
            mUi->wTransfers->transferFilterApply();
        });
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

void TransferManager::on_leSearchField_returnPressed()
{
    emit mUi->tSearchIcon->clicked();
}

void TransferManager::toggleTab(TM_TAB tab)
{
    if (mCurrentTab != tab)
    {
        // Activate frame
        if (mCurrentTab != NO_TAB)
        {
            mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        }
        mTabFramesToggleGroup[tab]->setGraphicsEffect(mShadowTab);
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);


        // Show pause button on tab except completed tab,
        // and set Clear All button string,
        // Emit wether we are showing completed or not
        if (tab == COMPLETED_TAB)
        {
            mUi->tClearCompleted->setText(tr("Clear All"));
            mUi->bPause->setVisible(false);
            emit showCompleted(true);
        }
        else if (mCurrentTab == COMPLETED_TAB)
        {
            mUi->tClearCompleted->setText(tr("Clear Completed"));
            mUi->bPause->setVisible(true);
            emit showCompleted(false);
        }

        // Set current header widget: search or not
        if (tab == SEARCH_TAB)
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pSearchHeader);

        }
        else if (mCurrentTab == SEARCH_TAB)
        {
            mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
        }

        mCurrentTab = tab;
    }

    refreshStats();

    // Reload QSS because it can be glitchy
    mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
}

void TransferManager::refreshView()
{
    if (mCurrentTab != NO_TAB)
    {
        QWidget* widgetToShow (mUi->wTransfers);

        if (mNumberOfTransfersPerTab[mCurrentTab] <= 0)
        {
            widgetToShow = mTabNoItem[mCurrentTab];
        }

        if (mUi->sTransfers->currentWidget() != widgetToShow)
        {
            mUi->sTransfers->setCurrentWidget(widgetToShow);
        }

        // Show "Clear All/Completed" if there are any completed transfers
        // (only for completed tab and individual media tabs)
        if ((mCurrentTab == COMPLETED_TAB && mNumberOfTransfersPerTab[COMPLETED_TAB])
                || (mCurrentTab >= TYPES_TAB_BASE && mModel->getNumberOfFinishedForFileType(
                        static_cast<TransferData::FileType>(mCurrentTab - TYPES_TAB_BASE))))
        {
            mUi->tClearCompleted->show();
        }
        else
        {
            mUi->tClearCompleted->hide();
        }

        // Hide Media groupbox if no transfers (active or finished)
        if (mCurrentTab >= TYPES_TAB_BASE
                || (mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB]
                    + mNumberOfTransfersPerTab[COMPLETED_TAB] > 0))
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
        onUpdatePauseState(mModel->areAllPaused());
    }
    QDialog::changeEvent(event);
}
