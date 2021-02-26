#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "platform/Platform.h"

// test
#include "MegaTransferDelegate2.h"

#include <QMouseEvent>

using namespace mega;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::TransferManager),
    mMegaApi(megaApi),
    mPreferences(Preferences::instance()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mActiveStates{MegaTransfer::STATE_ACTIVE,
                  MegaTransfer::STATE_PAUSED,
                  MegaTransfer::STATE_COMPLETING,
                  MegaTransfer::STATE_QUEUED,
                  MegaTransfer::STATE_RETRYING},
    mFinishedStates {MegaTransfer::STATE_COMPLETED,
                     MegaTransfer::STATE_FAILED,
                     MegaTransfer::STATE_CANCELLED},
    mCurrentTab(COMPLETED_TAB)
{
    mUi->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);

    mUi->wSearch->hide();

#ifndef __APPLE__
    Qt::WindowFlags flags =  Qt::Window;
    this->setWindowFlags(flags);
#endif

    mUi->wTransfers->setupTransfers();
    mUi->sStatus->setCurrentWidget(mUi->pUpToDate);



    Platform::enableDialogBlur(this);

    mTabFramesToggleGroup[ALL_TRANSFERS_TAB] = mUi->fAllTransfers;
    mTabFramesToggleGroup[DOWNLOADS_TAB]     = mUi->fDownloads;
    mTabFramesToggleGroup[UPLOADS_TAB]       = mUi->fUploads;
    mTabFramesToggleGroup[COMPLETED_TAB]     = mUi->fCompleted;

    mUi->fCompleted->setVisible(false);

    mMediaNumberLabelsGroup[TransferData::TYPE_OTHER]    = mUi->lOtherNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_AUDIO]    = mUi->lMusicNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_VIDEO]    = mUi->lVideosNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_ARCHIVE]  = mUi->lArchivesNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_DOCUMENT] = mUi->lDocumentsNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_IMAGE]    = mUi->lImagesNb;
    mMediaNumberLabelsGroup[TransferData::TYPE_TEXT]     = mUi->lTextNb;

    for (auto mediaLabel : mMediaNumberLabelsGroup)
    {
        mediaLabel->parentWidget()->setVisible(false);
    }
    mUi->wMediaType->setVisible(false);

    QObject::connect(mUi->bImportLinks, SIGNAL(clicked()), qApp, SLOT(importLinks()));
    QObject::connect(mUi->tCogWheel, SIGNAL(clicked()), qApp, SLOT(openSettings()));
    QObject::connect(mUi->bDownload, SIGNAL(clicked()), qApp, SLOT(downloadActionClicked()));
    QObject::connect(mUi->bUpload, SIGNAL(clicked()), qApp, SLOT(uploadActionClicked()));

    QObject::connect(mUi->leSearchField, SIGNAL(returnPressed()), mUi->tSearchIcon, SIGNAL(clicked()));

    QTransfersModel2* model (mUi->wTransfers->getModel2());

    connect(model, &QTransfersModel2::nbOfTransfersPerFileTypeChanged,
            this, &TransferManager::onNbOfTransfersPerFileTypeChanged);
    connect(model, &QTransfersModel2::nbOfTransfersPerStateChanged,
            this, &TransferManager::onNbOfTransfersPerStateChanged);
    connect(model, &QTransfersModel2::nbOfTransfersPerTypeChanged,
            this, &TransferManager::onNbOfTransfersPerTypeChanged);

    QObject::connect(qApp, SIGNAL(pauseStateChanged()), this, SLOT(updateState()));

    on_tAllTransfers_clicked();
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

    mUi->wTransfers->transferStateFilterChanged(mFinishedStates);
    mUi->wTransfers->transferTypeFilterChanged({});

    mUi->lCurrentContent->setText(tr("Finished"));

    updateState();
}

void TransferManager::on_tDownloads_clicked()
{
    emit userActivity();

    toggleTab(DOWNLOADS_TAB);

    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(mActiveStates);
    mUi->wTransfers->transferTypeFilterChanged({MegaTransfer::TYPE_DOWNLOAD});

    mUi->lCurrentContent->setText(tr("Downloads"));

    updateState();
}

void TransferManager::on_tUploads_clicked()
{
    emit userActivity();

    toggleTab(UPLOADS_TAB);

    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(mActiveStates);
    mUi->wTransfers->transferTypeFilterChanged({MegaTransfer::TYPE_UPLOAD});

    mUi->lCurrentContent->setText(tr("Uploads"));

    updateState();
}

void TransferManager::on_tAllTransfers_clicked()
{
    emit userActivity();

    toggleTab(ALL_TRANSFERS_TAB);

    mUi->bPause->setVisible(true);

    mUi->wTransfers->transferStateFilterChanged(mActiveStates);
    mUi->wTransfers->transferTypeFilterChanged({});

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

void TransferManager::onNbOfTransfersPerStateChanged(int state, long long number)
{
    QLabel *label (nullptr);
    bool showFrame (true);
    long long processedNumber (0LL);
    QWidget * leftFooterWidget (mUi->sStatus->currentWidget());

    mStatesStatistics[state] = number;

    if (mFinishedStates.contains(state))
    {
        label = mUi->lCompleted;
        for (auto state : mFinishedStates)
        {
            processedNumber += mStatesStatistics[state];
        }
        showFrame = processedNumber;
    }
    else
    {
        label = mUi->lAllTransfers;
        for (auto state : mActiveStates)
        {
            processedNumber += mStatesStatistics[state];
        }
        if (processedNumber)
        {
            leftFooterWidget = mUi->pSpeedAndClear;
        }
        else
        {
            leftFooterWidget = mUi->pUpToDate;
        }
    }

    label->setText(QString::number(processedNumber));
    label->parentWidget()->setVisible(showFrame);
    mUi->sStatus->setCurrentWidget(leftFooterWidget);
}

void TransferManager::onNbOfTransfersPerTypeChanged(int type, long long number)
{
    QLabel *label (nullptr);

    switch (type)
    {
        case MegaTransfer::TYPE_DOWNLOAD:
        case MegaTransfer::TYPE_LOCAL_HTTP_DOWNLOAD:
        {
            label = mUi->lDownloads;
            break;
        }
        case MegaTransfer::TYPE_UPLOAD:
        {
            label = mUi->lUploads;
            break;
        }
    }
    label->setText(QString::number(number));
    label->parentWidget()->setVisible(number);
}

void TransferManager::onNbOfTransfersPerFileTypeChanged(TransferData::FileTypes fileType, long long number)
{
    mMediaNumberLabelsGroup[fileType]->setText(QString::number(number));
    mMediaNumberLabelsGroup[fileType]->parentWidget()->setVisible(number);

    bool showMediaBox (number);

    if (!showMediaBox)
    {
        auto media (mMediaNumberLabelsGroup.keys());
        auto mediaIt (media.begin());
        do
        {
            showMediaBox = mMediaNumberLabelsGroup[*mediaIt]->text().toInt();
            mediaIt++;
        }
        while (!showMediaBox && mediaIt != media.end());
    }

    mUi->wMediaType->setVisible(showMediaBox);
}

void TransferManager::updateState()
{
    bool isPaused (false);
    QString bPauseTooltip;

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
        case COMPLETED_TAB:
        default:
            break;
    }

    // If search active, update result number
    if (mUi->sCurrentContent->currentWidget() == mUi->pSearchHeader)
    {
        mUi->lNbResults->setText(QString(tr("%1 results")).arg(mUi->wTransfers->rowCount()));
    }

    updatePauseState(isPaused, bPauseTooltip);
    setStyleSheet(styleSheet());
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
            mMegaApi->pauseTransfers(!mPreferences->getGlobalPaused());
            break;
        }
        case DOWNLOADS_TAB:
        {
            mMegaApi->pauseTransfers(!mPreferences->getDownloadsPaused(), MegaTransfer::TYPE_DOWNLOAD);
            break;
        }
        case UPLOADS_TAB:
        {
            mMegaApi->pauseTransfers(!mPreferences->getUploadsPaused(), MegaTransfer::TYPE_UPLOAD);
            break;
        }
    }
}

void TransferManager::on_bClearAll_clicked()
{
    QPointer<TransferManager> dialog = QPointer<TransferManager>(this);

    if (QMegaMessageBox::warning(nullptr,
                             QString::fromUtf8("MEGAsync"),
                             tr("Are you sure you want to cancel all transfers?"),
                             QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes
            || !dialog)
    {
        return;
    }

    mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}

void TransferManager::on_bSearch_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pSearch);
    mUi->leSearchField->setText(QString());
}

void TransferManager::on_tSearchIcon_clicked()
{
    mUi->fSearchString->setProperty("itsOn", true);
    mUi->bSearchString->setText(mUi->bSearchString->fontMetrics().elidedText(mUi->leSearchField->text(),
                                                                           Qt::ElideMiddle,
                                                                           mUi->bSearchString->width()-24));
    mUi->wTransfers->transferStateFilterChanged({});
    mUi->wTransfers->textFilterChanged(QRegExp(mUi->leSearchField->text(), Qt::CaseInsensitive));

    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics().elidedText(mUi->leSearchField->text(),
                                                                         Qt::ElideMiddle,
                                                                         mUi->lTextSearch->width()-24));
    mUi->lNbResults->setText(QString(tr("%1 results")).arg(mUi->wTransfers->rowCount()));
    mUi->sCurrentContent->setCurrentWidget(mUi->pSearchHeader);

    // Unselect type view
    mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);

    // Add number of found results
    setStyleSheet(styleSheet());
    mUi->wSearch->show();
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

void TransferManager::toggleTab(TM_TABS tab)
{
    if (mCurrentTab != tab)
    {
        mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);
        mCurrentTab = tab;
    }
    mUi->sCurrentContent->setCurrentWidget(mUi->pStatusHeader);
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
    mMediaNumberLabelsGroup[fileType]->parentWidget()->setProperty("itsOn", showFrame);
    setStyleSheet(styleSheet());
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        switch (mCurrentTab)
        {
            case ALL_TRANSFERS_TAB:
            {
                on_tAllTransfers_clicked();
                break;
            }
            case DOWNLOADS_TAB:
            {
                on_tDownloads_clicked();
                break;
            }
            case UPLOADS_TAB:
            {
                on_tUploads_clicked();
                break;
            }
            case COMPLETED_TAB:
            {
                on_tCompleted_clicked();
                break;
            }
            default:
                break;
        }
    }
    QDialog::changeEvent(event);
}

void TransferManager::mouseMoveEvent(QMouseEvent *event)
{
    if (event->buttons() & Qt::LeftButton)
    {
        if (mDragPosition.x() != -1)
        {
            move(event->globalPos() - mDragPosition);
            event->accept();
        }
    }
}

void TransferManager::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        mDragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void TransferManager::mouseReleaseEvent(QMouseEvent *event)
{
    mDragPosition = QPoint(-1, -1);
}
