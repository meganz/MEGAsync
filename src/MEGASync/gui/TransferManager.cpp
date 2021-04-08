#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "platform/Platform.h"

#include "MegaTransferDelegate2.h"

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
    mCurrentTab(COMPLETED_TAB),
    mShadowTab (new QGraphicsDropShadowEffect(nullptr)),
    mShadowSearch (new QGraphicsDropShadowEffect(nullptr)),
    mSpeedRefreshTimer(new QTimer(this)),
    mStatsRefreshTimer(new QTimer(this)),
    mPrevActiveNumber (-1LL),
    mPrevFinishedNumber (-1LL),
    mPrevDlNumber (-1LL),
    mPrevUlNumber (-1LL)
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

    mTabNoItem[ALL_TRANSFERS_TAB] = mUi->wNoTransfers;
    mTabNoItem[DOWNLOADS_TAB]     = mUi->wNoDownloads;
    mTabNoItem[UPLOADS_TAB]       = mUi->wNoUploads;
    mTabNoItem[COMPLETED_TAB]     = mUi->wNoFinished;
    mTabNoItem[SEARCH_TAB]        = mUi->wNoResults;

    mUi->lAllTransfers->hide();
    mUi->lDownloads->hide();
    mUi->lUploads->hide();

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
        QGraphicsDropShadowEffect* shadow = new QGraphicsDropShadowEffect(frame);
        shadow->setBlurRadius(8.);
        shadow->setXOffset(0.);
        shadow->setYOffset(0.);
        shadow->setColor(shadowColor);
        shadow->setEnabled(false);
        frame->setGraphicsEffect(shadow);
    }

    connect(qobject_cast<MegaApplication*>(qApp), &MegaApplication::pauseStateChanged,
            this, &TransferManager::updateState);

    connect(mModel, &QTransfersModel2::transfersInModelChanged,
            this, &TransferManager::onTransfersInModelChanged, Qt::QueuedConnection);

    mSpeedRefreshTimer->setSingleShot(false);
    connect(mSpeedRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshSpeed);

    mStatsRefreshTimer->setSingleShot(false);
    connect(mStatsRefreshTimer, &QTimer::timeout,
            this, &TransferManager::refreshStats);

    onTransfersInModelChanged(true);
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

    mUi->bPause->setVisible(false);

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

    mUi->bPause->setVisible(true);

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

    mUi->bPause->setVisible(true);

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

    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(ACTIVE_STATES);
    mUi->wTransfers->transferTypeFilterChanged({});
    mUi->wTransfers->transferFilterApply();

    mUi->lCurrentContent->setText(tr("All Transfers"));

    updateState();
}

void TransferManager::updatePauseState(bool isPaused, QString toolTipText)
{
    if (isPaused)
    {
        static const QIcon icon(QLatin1String(":/images/play_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Resume " + toolTipText.toUtf8()));
        mUi->lPaused->setText(tr("Paused"));
    }
    else
    {
        static const QIcon icon(QLatin1String(":/images/pause_ico.png"));
        mUi->bPause->setIcon(icon);
        mUi->bPause->setToolTip(tr("Pause " + toolTipText.toUtf8()));
        mUi->lPaused->setText(QString());
    }
}

bool TransferManager::refreshStateStats()
{
    QLabel* label (nullptr);
    bool weHaveTransfers (true);
    long long processedNumber (0LL);

    // First check Finished states
    label = mUi->lCompleted;
    for (auto state : FINISHED_STATES)
    {
        processedNumber += mModel->getNumberOfTransfersForState(state);
    }
    weHaveTransfers = processedNumber;

    if (processedNumber != mPrevFinishedNumber)
    {
        if (processedNumber == 0 && mCurrentTab == COMPLETED_TAB)
        {
            mUi->sTransfers->setCurrentWidget(mTabNoItem[mCurrentTab]);
        }

        label->parentWidget()->setVisible(weHaveTransfers);
        label->setText(QString::number(processedNumber));
        mPrevFinishedNumber = processedNumber;
    }

    // Then active states
    processedNumber = 0LL;
    label = mUi->lAllTransfers;
    for (auto state : ACTIVE_STATES)
    {
        processedNumber += mModel->getNumberOfTransfersForState(state);
    }
    weHaveTransfers |= static_cast<bool>(processedNumber);

    if (processedNumber != mPrevActiveNumber)
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
            if (mPrevActiveNumber <= 0)
            {
                leftFooterWidget = mUi->pSpeedAndClear;
                mSpeedRefreshTimer->start(std::chrono::milliseconds(SPEED_REFRESH_PERIOD_MS));
                label->show();
                if (mCurrentTab == ALL_TRANSFERS_TAB)
                {
                    mUi->sTransfers->setCurrentWidget(mUi->wTransfers);
                }
            }
            label->setText(QString::number(processedNumber));
        }

        mUi->sStatus->setCurrentWidget(leftFooterWidget);
        mPrevActiveNumber = processedNumber;
    }

    return weHaveTransfers;
}

void TransferManager::refreshTypeStats()
{
    QWidget* widgetToShow (mUi->sTransfers->currentWidget());

    long long number (mModel->getNumberOfTransfersForType(MegaTransfer::TYPE_DOWNLOAD));
    if (number != mPrevDlNumber)
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
        mPrevDlNumber = number;
    }

    number = mModel->getNumberOfTransfersForType(MegaTransfer::TYPE_UPLOAD);
    if (number != mPrevUlNumber)
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
        mPrevUlNumber = number;
    }

    if ((mCurrentTab == UPLOADS_TAB && mPrevUlNumber == 0)
            || (mCurrentTab == DOWNLOADS_TAB && mPrevDlNumber == 0))
    {
        widgetToShow = mTabNoItem[mCurrentTab];
    }
    else if ((mCurrentTab == UPLOADS_TAB && mPrevUlNumber != 0)
             || (mCurrentTab == DOWNLOADS_TAB && mPrevDlNumber != 0))
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
        long long number (mModel->getNumberOfTransfersForFileType(fileType));
        mMediaNumberLabelsGroup[fileType]->parentWidget()->setVisible(number);
        mMediaNumberLabelsGroup[fileType]->setText(QString::number(number));
        if (number == 0
                && mMediaNumberLabelsGroup[fileType]->parentWidget()->property("itsOn").toBool())
        {
            updateFileTypeFilter(fileType);
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
    mUi->wMediaType->setVisible(showMediaTypes);
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
    bool isPaused (false);
    QString bPauseTooltip;

    QWidget* widgetToShow (mUi->wTransfers);

    auto nbRows (mUi->wTransfers->rowCount());


    switch (mCurrentTab)
    {
        case ALL_TRANSFERS_TAB:
        {
            isPaused = mPreferences->getGlobalPaused();
            bPauseTooltip = QLatin1String("All");
            break;
        }
        case DOWNLOADS_TAB:
        {
            isPaused = mPreferences->getDownloadsPaused();
            bPauseTooltip = QLatin1String("Downloads");
            break;
        }
        case UPLOADS_TAB:
        {
            isPaused = mPreferences->getUploadsPaused();
            bPauseTooltip = QLatin1String("Uploads");
            break;
        }
        case SEARCH_TAB:
        {
            mUi->lNbResults->setText(QString(tr("%1 results")).arg(nbRows));
        }
        case COMPLETED_TAB:
        default:
            break;
    }

    updatePauseState(isPaused, bPauseTooltip);

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

void TransferManager::on_bPause_clicked()
{
    emit userActivity();

    switch (mCurrentTab)
    {
        case COMPLETED_TAB:
        case ALL_TRANSFERS_TAB:
        {
            mModel->pauseResumeAllTransfers();
            break;
        }
        case DOWNLOADS_TAB:
        {
            mModel->pauseResumeDownloads();
            break;
        }
        case UPLOADS_TAB:
        {
            mModel->pauseResumeUploads();
            break;
        }
        case SEARCH_TAB:
        default:
        {
            break;
        }
    }
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
    mUi->wTransfers->textFilterChanged(QRegExp(mUi->leSearchField->text(), Qt::CaseInsensitive));

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
    mUi->fSearchString->setProperty("itsOn", false);
    mUi->wTransfers->textFilterChanged(QRegExp());
    mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
    on_tSearchCancel_clicked();
    updateState();
}

void TransferManager::on_bArchives_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_ARCHIVE);
}

void TransferManager::on_bDocuments_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_DOCUMENT);
}

void TransferManager::on_bImages_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_IMAGE);
}

void TransferManager::on_bMusic_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_AUDIO);
}

void TransferManager::on_bVideos_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_VIDEO);
}

void TransferManager::on_bOther_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_OTHER);
}

void TransferManager::on_bText_clicked()
{
    updateFileTypeFilter(TransferData::TYPE_TEXT);
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
        if (mCurrentTab != SEARCH_TAB)
        {
            mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        }

        if (tab != SEARCH_TAB)
        {
            mTabFramesToggleGroup[tab]->setGraphicsEffect(mShadowTab);
        }
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);
        mShadowTab->setEnabled(true);
        mCurrentTab = tab;
    }

    if (tab == SEARCH_TAB)
    {
        currentContentHeaderWidget = mUi->pSearchHeader;
        mShadowTab->setEnabled(false);
    }

    mUi->sCurrentContent->setCurrentWidget(currentContentHeaderWidget);
}

void TransferManager::updateFileTypeFilter(TransferData::FileTypes fileType)
{
    bool showFrame (true);
    if (mFileTypesFilter.contains(fileType))
    {
        showFrame = false;
        mFileTypesFilter.remove(fileType);
    }
    else
    {
        mFileTypesFilter.insert(fileType);
    }

    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    mUi->wTransfers->transferFilterApply();

    QWidget* frame (mMediaNumberLabelsGroup[fileType]->parentWidget());
    frame->setProperty("itsOn", showFrame);
    frame->graphicsEffect()->setEnabled(showFrame);
    mUi->wLeftPane->setStyleSheet(mUi->wLeftPane->styleSheet());
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

void TransferManager::paintEvent(QPaintEvent* event)
{
    mUi->wTransfers->update();
    QDialog::paintEvent(event);
}

//void TransferManager::mouseMoveEvent(QMouseEvent *event)
//{
//    if (event->buttons() & Qt::LeftButton)
//    {
//        if (mDragPosition.x() != -1)
//        {
//            move(event->globalPos() - mDragPosition);
//            event->accept();
//        }
//    }
//}

//void TransferManager::mousePressEvent(QMouseEvent *event)
//{
//    if (event->button() == Qt::LeftButton)
//    {
//        mDragPosition = event->globalPos() - frameGeometry().topLeft();
//        event->accept();
//    }
//}

//void TransferManager::mouseReleaseEvent(QMouseEvent *event)
//{
//    mDragPosition = QPoint(-1, -1);
//}
