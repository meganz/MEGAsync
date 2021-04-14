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

const QSet<int> TransferManager::ACTIVE_STATES =
{
    MegaTransfer::STATE_ACTIVE,
    MegaTransfer::STATE_PAUSED,
    MegaTransfer::STATE_COMPLETING,
    MegaTransfer::STATE_QUEUED,
    MegaTransfer::STATE_RETRYING
};
const QSet<int> TransferManager::FINISHED_STATES =
{
    MegaTransfer::STATE_COMPLETED,
    MegaTransfer::STATE_FAILED,
    MegaTransfer::STATE_CANCELLED
};

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::TransferManager),
    mMegaApi(megaApi),
    mPreferences(Preferences::instance()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mModel(nullptr),
    mCurrentTab(NO_TAB),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr)),
    mShadowSearch (new QGraphicsDropShadowEffect(nullptr)),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this)),
    mNumberOfTransfersPerTab(TYPE_TEXT_TAB + 1, -1LL)
{
    mUi->setupUi(this);
    mUi->wTransfers->setupTransfers();

    mUi->lTextSearch->installEventFilter(this);

    mModel = mUi->wTransfers->getModel2();

    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    Platform::enableDialogBlur(this);

#ifndef __APPLE__
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

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

    mShadowSearch->setParent(mUi->fSearchString);
    mShadowSearch->setBlurRadius(8.);
    mShadowSearch->setXOffset(0.);
    mShadowSearch->setYOffset(0.);
    mShadowSearch->setColor(shadowColor);

    mUi->fSearchString->setGraphicsEffect(mShadowSearch);

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
        QWidget* frame = mediaLabel->parentWidget();
        frame->hide();
//        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(frame);
//        shadow->setBlurRadius(8.);
//        shadow->setXOffset(0.);
//        shadow->setYOffset(0.);
//        shadow->setColor(shadowColor);
//        shadow->setEnabled(false);
//        frame->setGraphicsEffect(shadow);
    }

//    connect(qobject_cast<MegaApplication*>(qApp), &MegaApplication::pauseStateChanged,
//            this, &TransferManager::updateState);


//    mUi->wTransfers->setParent(this);

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

    connect(this, &TransferManager::cancelClearAllRows,
            findChild<MegaTransferView*>(), &MegaTransferView::onCancelClearAllRows);

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
    emit userActivity();

    toggleTab(COMPLETED_TAB);

//    mUi->bPause->setVisible(false);

    mUi->wTransfers->transferStateFilterChanged(FINISHED_STATES);
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Finished"));

    updateState();
}

void TransferManager::on_tDownloads_clicked()
{
    emit userActivity();

    toggleTab(DOWNLOADS_TAB);

//    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES);
    mUi->wTransfers->transferTypeFilterChanged({MegaTransfer::TYPE_DOWNLOAD});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Downloads"));

    updateState();
}

void TransferManager::on_tUploads_clicked()
{
    emit userActivity();

    toggleTab(UPLOADS_TAB);

//    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES);
    mUi->wTransfers->transferTypeFilterChanged({MegaTransfer::TYPE_UPLOAD});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Uploads"));

    updateState();
}

void TransferManager::on_tAllTransfers_clicked()
{
    emit userActivity();

    toggleTab(ALL_TRANSFERS_TAB);

//    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES);
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("All Transfers"));

    updateState();
}

void TransferManager::onUpdatePauseState(bool isPaused)
{
    if (isPaused)
    {
        static const QIcon icon(QLatin1String(":/images/play_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Resume All"));
        mUi->lPaused->setText(tr("Paused"));
    }
    else
    {
        static const QIcon icon(QLatin1String(":/images/pause_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Pause All"));
        mUi->lPaused->setText(QString());
    }
}

bool TransferManager::refreshStateStats()
{
    QLabel* label (nullptr);
    bool weHaveTransfers (true);
    bool show (true);
    long long processedNumber (0LL);

    // First check Finished states
    label = mUi->lCompleted;
    for (auto state : FINISHED_STATES)
    {
        processedNumber += mModel->getNumberOfTransfersForState(state);
    }
    weHaveTransfers = processedNumber;

    if (processedNumber != mNumberOfTransfersPerTab[COMPLETED_TAB])
    {
        if (processedNumber <= 0)
        {
            if (mCurrentTab == COMPLETED_TAB)
            {
                mUi->sTransfers->setCurrentWidget(mTabNoItem[mCurrentTab]);
                //            on_tAllTransfers_clicked();
            }
            else
            {
                show = false;
            }
        }
        else if (mCurrentTab == COMPLETED_TAB)
        {
            mUi->sTransfers->setCurrentWidget(mUi->wTransfers);
        }

        mUi->tClearCompleted->setVisible(((mCurrentTab == COMPLETED_TAB) && processedNumber)
                                         || ((mCurrentTab >= TYPES_TAB_BASE)
                                         && mModel->getNumberOfFinishedForFileType(
                                             static_cast<TransferData::FileTypes>(mCurrentTab - TYPES_TAB_BASE))));

        label->parentWidget()->setVisible(show);
        label->setVisible(processedNumber);
        label->setText(QString::number(processedNumber));
        label->updateGeometry();

        mNumberOfTransfersPerTab[COMPLETED_TAB] = processedNumber;
    }

    // Then active states
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
        if (processedNumber == 0)
        {
            leftFooterWidget = mUi->pUpToDate;
            mSpeedRefreshTimer->stop();
            label->hide();

            if (mCurrentTab == ALL_TRANSFERS_TAB)
            {
                mUi->sTransfers->setCurrentWidget(mTabNoItem[mCurrentTab]);
            }
        }
        else
        {
            if (mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] <= 0)
            {
                leftFooterWidget = mUi->pSpeedAndClear;
                mSpeedRefreshTimer->start(std::chrono::milliseconds(SPEED_REFRESH_PERIOD_MS));
                label->show();

            }
            if (mCurrentTab == ALL_TRANSFERS_TAB)
            {
                mUi->sTransfers->setCurrentWidget(mUi->wTransfers);
            }
            label->setText(QString::number(processedNumber));
        }
        label->updateGeometry();

        mUi->sStatus->setCurrentWidget(leftFooterWidget);
        mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB] = processedNumber;
    }
    return weHaveTransfers;
}

void TransferManager::refreshTypeStats()
{
    QWidget* widgetToShow (mUi->sTransfers->currentWidget());

    long long number (mModel->getNumberOfTransfersForType(MegaTransfer::TYPE_DOWNLOAD));
    if (number != mNumberOfTransfersPerTab[DOWNLOADS_TAB])
    {
        if (number == 0)
        {
            if (mCurrentTab == DOWNLOADS_TAB)
            {
                mUi->sTransfers->setCurrentWidget(mTabNoItem[mCurrentTab]);
            }
            mUi->lDownloads->hide();
        }
        else
        {
            mUi->lDownloads->show();
            mUi->lDownloads->setText(QString::number(number));
        }
        mUi->lDownloads->updateGeometry();
        mNumberOfTransfersPerTab[DOWNLOADS_TAB] = number;
    }

    number = mModel->getNumberOfTransfersForType(MegaTransfer::TYPE_UPLOAD);
    if (number != mNumberOfTransfersPerTab[UPLOADS_TAB])
    {
        if (number == 0)
        {
            if (mCurrentTab == UPLOADS_TAB)
            {
                mUi->sTransfers->setCurrentWidget(mTabNoItem[mCurrentTab]);
            }
            mUi->lUploads->hide();
        }
        else
        {
            mUi->lUploads->show();
            mUi->lUploads->setText(QString::number(number));
        }
        mUi->lUploads->updateGeometry();
        mNumberOfTransfersPerTab[UPLOADS_TAB] = number;
    }

    if ((mCurrentTab == UPLOADS_TAB && mNumberOfTransfersPerTab[UPLOADS_TAB] == 0)
            || (mCurrentTab == DOWNLOADS_TAB && mNumberOfTransfersPerTab[DOWNLOADS_TAB] == 0))
    {
        widgetToShow = mTabNoItem[mCurrentTab];
    }
    else if ((mCurrentTab == UPLOADS_TAB && mNumberOfTransfersPerTab[UPLOADS_TAB] != 0)
             || (mCurrentTab == DOWNLOADS_TAB && mNumberOfTransfersPerTab[DOWNLOADS_TAB] != 0))
    {
        widgetToShow = mUi->wTransfers;
    }

    if (mUi->sTransfers->currentWidget() != widgetToShow)
    {
        mUi->sTransfers->setCurrentWidget(widgetToShow);
    }
}

void TransferManager::refreshFileTypesStats()
{
    const auto fileTypes (mMediaNumberLabelsGroup.keys());
    for (auto fileType : fileTypes)
    {
        int tab (TYPES_TAB_BASE + fileType);
        long long number (mModel->getNumberOfTransfersForFileType(fileType));
        if (mNumberOfTransfersPerTab[tab] != number)
        {
            bool show (false);
            QLabel* label (mMediaNumberLabelsGroup[fileType]);
            if (number > 0 || mCurrentTab == tab)
            {
                show = true;
            }

            label->setVisible(number);
            label->setText(QString::number(number));
            label->parentWidget()->setVisible(show);

            mNumberOfTransfersPerTab[tab] = number;
        }
    }
}

void TransferManager::onTransfersInModelChanged(bool weHaveTransfers)
{
    if (weHaveTransfers)
    {
        mStatsRefreshTimer->start(std::chrono::milliseconds(STATS_REFRESH_PERIOD_MS));
    }
    else
    {
        mStatsRefreshTimer->stop();
    }
    refreshTypeStats();
    refreshFileTypesStats();

    bool showMediaTypes(false);
    showMediaTypes = refreshStateStats();
    mUi->wMediaType->setVisible(showMediaTypes || mCurrentTab >= TYPES_TAB_BASE);
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
    if (!refreshStateStats())
    {
        onTransfersInModelChanged(false);
    }
}

void TransferManager::updateState()
{
//    bool isPaused (false);
    auto nbRows (mUi->wTransfers->rowCount());

    QWidget* widgetToShow (mUi->wTransfers);

    if (mCurrentTab == SEARCH_TAB)
    {
        mUi->lNbResults->setText(QString(tr("%1 results")).arg(nbRows));
    }

    if (nbRows == 0)
    {
        widgetToShow = mTabNoItem[mCurrentTab];
    }

    if (mUi->sTransfers->currentWidget() != widgetToShow)
    {
        mUi->sTransfers->setCurrentWidget(widgetToShow);
    }

    mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
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

    mModel->cancelAllTransfers();
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
    toggleTab(SEARCH_TAB);

    mUi->bSearchString->setText(mUi->bSearchString->fontMetrics()
                                .elidedText(mUi->leSearchField->text(),
                                            Qt::ElideMiddle,
                                            mUi->bSearchString->width() - 24));
    mUi->wTransfers->transferFilterReset();
    mUi->wTransfers->textFilterChanged(mUi->leSearchField->text());

    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics()
                              .elidedText(mUi->leSearchField->text(),
                                          Qt::ElideMiddle,
                                          mUi->lTextSearch->width() - 24));
    // Add number of found results
    mUi->wSearch->show();
    updateState();
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
    toggleTab(TYPE_ARCHIVE_TAB);

//    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_ARCHIVE});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Archives"));

    updateState();
}

void TransferManager::on_bDocuments_clicked()
{
    toggleTab(TYPE_DOCUMENT_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_DOCUMENT});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Documents"));

    updateState();
}

void TransferManager::on_bImages_clicked()
{
    toggleTab(TYPE_IMAGE_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_IMAGE});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Images"));

    updateState();
}

void TransferManager::on_bMusic_clicked()
{
    toggleTab(TYPE_AUDIO_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_AUDIO});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Music"));

    updateState();
}

void TransferManager::on_bVideos_clicked()
{
    toggleTab(TYPE_VIDEO_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_VIDEO});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Videos"));

    updateState();
}

void TransferManager::on_bOther_clicked()
{
    toggleTab(TYPE_OTHER_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_OTHER});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Other"));

    updateState();
}

void TransferManager::on_bText_clicked()
{
    toggleTab(TYPE_TEXT_TAB);

    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->fileTypeFilterChanged({TransferData::TYPE_TEXT});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("Text"));

    updateState();
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

void TransferManager::toggleTab(TM_TABS tab)
{
    QWidget* currentContentHeaderWidget (mUi->pStatusHeader);
    if (mCurrentTab != tab)
    {
        if (mCurrentTab != SEARCH_TAB && mCurrentTab != NO_TAB)
        {
            mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        }

        if (tab != SEARCH_TAB)
        {
            mTabFramesToggleGroup[tab]->setGraphicsEffect(mShadowTab);
        }
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);
        mShadowTab->setEnabled(true);

        if (mCurrentTab >= TYPES_TAB_BASE
                && (mNumberOfTransfersPerTab[ALL_TRANSFERS_TAB]
                    + mNumberOfTransfersPerTab[COMPLETED_TAB] == 0))
        {
            mUi->wMediaType->hide();
        }

        if ((mCurrentTab == COMPLETED_TAB || mCurrentTab >= TYPES_TAB_BASE)
                && mNumberOfTransfersPerTab[mCurrentTab] == 0)
        {
            mTabFramesToggleGroup[mCurrentTab]->hide();
        }

        if (tab == COMPLETED_TAB || tab >= TYPES_TAB_BASE)
        {
            bool showClearButton (false);
            if (tab == COMPLETED_TAB)
            {
                mUi->tClearCompleted->setText(tr("Clear All"));
                showClearButton = mNumberOfTransfersPerTab[COMPLETED_TAB];
            }
            else
            {
                mUi->tClearCompleted->setText(tr("Clear Completed"));
                showClearButton = mModel->getNumberOfFinishedForFileType(
                                      static_cast<TransferData::FileTypes>(tab - TYPES_TAB_BASE));
            }
            mUi->tClearCompleted->setVisible(showClearButton);
        }
        else
        {
            mUi->tClearCompleted->hide();
        }

        if (tab == SEARCH_TAB)
        {
            currentContentHeaderWidget = mUi->pSearchHeader;
            mShadowTab->setEnabled(false);
        }

        emit showCompleted(tab == COMPLETED_TAB);

        mUi->bPause->setVisible(tab != COMPLETED_TAB);

        mCurrentTab = tab;
    }

    mUi->sCurrentContent->setCurrentWidget(currentContentHeaderWidget);
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
    }
    QDialog::changeEvent(event);
}
