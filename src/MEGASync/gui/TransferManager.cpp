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
    mRefreshTransferTime(new QTimer(this)),
    mThreadPool(ThreadPoolSingleton::getInstance()),
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

    mRefreshTransferTime->setSingleShot(false);
    connect(mRefreshTransferTime, SIGNAL(timeout()), this, SLOT(refreshFinishedTime()));

    mUi->wTransfers->setupTransfers();

    mUi->fCompleted->setVisible(true);

    mUi->sStatus->setCurrentWidget(mUi->wSpeedAndClear);

//    mUi->wCompleted->setupFinishedTransfers(((MegaApplication *)qApp)->getFinishedTransfers());
    updateNumberOfCompletedTransfers(((MegaApplication *)qApp)->getNumUnviewedTransfers());

//    connect(mUi->wCompleted->getModel(), SIGNAL(noTransfers()), this, SLOT(updateState()));
//    connect(mUi->wCompleted->getModel(), SIGNAL(onTransferAdded()), this, SLOT(updateState()));


    QObject::connect(mUi->bImportLinks, SIGNAL(clicked()), qApp, SLOT(importLinks()));
    QObject::connect(mUi->tCogWheel, SIGNAL(clicked()), qApp, SLOT(openSettings()));
    QObject::connect(mUi->bDownload, SIGNAL(clicked()), qApp, SLOT(downloadActionClicked()));
    QObject::connect(mUi->bUpload, SIGNAL(clicked()), qApp, SLOT(uploadActionClicked()));

    QObject::connect(mUi->leSearchField, SIGNAL(returnPressed()), mUi->tSearchIcon, SIGNAL(clicked()));

    QObject::connect(qApp, SIGNAL(pauseStateChanged()), this, SLOT(updateState()));

    Platform::enableDialogBlur(this);

    mTabFramesToggleGroup[ALL_TRANSFERS_TAB] = mUi->fAllTransfers;
    mTabFramesToggleGroup[DOWNLOADS_TAB]     = mUi->fDownloads;
    mTabFramesToggleGroup[UPLOADS_TAB]       = mUi->fUploads;
    mTabFramesToggleGroup[COMPLETED_TAB]     = mUi->fCompleted;


    mUi->fArchives->setProperty("itsOn", true);
    mUi->fDocuments->setProperty("itsOn", true);
    mUi->fImages->setProperty("itsOn", true);
    mUi->fMusic->setProperty("itsOn", true);
    mUi->fVideos->setProperty("itsOn", true);
    mUi->fOther->setProperty("itsOn", true);

    mFileTypesFilter.insert(TransferData::TYPE_ARCHIVE);
    mFileTypesFilter.insert(TransferData::TYPE_AUDIO);
    mFileTypesFilter.insert(TransferData::TYPE_DOCUMENT);
    mFileTypesFilter.insert(TransferData::TYPE_IMAGE);
    mFileTypesFilter.insert(TransferData::TYPE_OTHER);
    mFileTypesFilter.insert(TransferData::TYPE_TEXT);
    mFileTypesFilter.insert(TransferData::TYPE_VIDEO);
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);

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

    if (!mRefreshTransferTime->isActive())
    {
        mRefreshTransferTime->start(Preferences::FINISHED_TRANSFER_REFRESH_INTERVAL_MS);
    }
    // Disable tracking of completed transfers and reset value
    emit viewedCompletedTransfers();
    emit completedTransfersTabActive(true);

    toggleTab(COMPLETED_TAB);

    mUi->bPause->setVisible(false);

    mUi->fCompleted->setVisible(true);

    mTransferStatesFilter.clear();
    mTransferStatesFilter.insert(MegaTransfer::STATE_COMPLETED);
    mUi->wTransfers->transferStateFilterChanged(mTransferStatesFilter);

    mTransferTypesFilter.insert(MegaTransfer::TYPE_DOWNLOAD);
    mTransferTypesFilter.insert(MegaTransfer::TYPE_UPLOAD);
    mUi->wTransfers->transferTypeFilterChanged(mTransferTypesFilter);

    mUi->lCurrentContent->setText(tr("Finished"));

    updateState();
}

void TransferManager::on_tDownloads_clicked()
{
    emit userActivity();

    if (mRefreshTransferTime->isActive())
    {
        mRefreshTransferTime->stop();
    }

    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    toggleTab(DOWNLOADS_TAB);

    mUi->bPause->setVisible(true);

    mTransferStatesFilter.clear();
    mTransferStatesFilter.insert(MegaTransfer::STATE_ACTIVE);
    mTransferStatesFilter.insert(MegaTransfer::STATE_PAUSED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_COMPLETING);
    mTransferStatesFilter.insert(MegaTransfer::STATE_FAILED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_QUEUED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_RETRYING);
    mUi->wTransfers->transferStateFilterChanged(mTransferStatesFilter);

    mTransferTypesFilter.clear();
    mTransferTypesFilter.insert(MegaTransfer::TYPE_DOWNLOAD);
    mUi->wTransfers->transferTypeFilterChanged(mTransferTypesFilter);

    mUi->lCurrentContent->setText(tr("Downloads"));

    updateState();
}

void TransferManager::on_tUploads_clicked()
{
    emit userActivity();

    if (mRefreshTransferTime->isActive())
    {
        mRefreshTransferTime->stop();
    }

    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    toggleTab(UPLOADS_TAB);

    mUi->bPause->setVisible(true);

    mTransferStatesFilter.clear();
    mTransferStatesFilter.insert(MegaTransfer::STATE_ACTIVE);
    mTransferStatesFilter.insert(MegaTransfer::STATE_PAUSED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_COMPLETING);
    mTransferStatesFilter.insert(MegaTransfer::STATE_FAILED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_QUEUED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_RETRYING);
    mUi->wTransfers->transferStateFilterChanged(mTransferStatesFilter);

    mTransferTypesFilter.clear();
    mTransferTypesFilter.insert(MegaTransfer::TYPE_UPLOAD);
    mUi->wTransfers->transferTypeFilterChanged(mTransferTypesFilter);

    mUi->lCurrentContent->setText(tr("Uploads"));

    updateState();
}

void TransferManager::on_tAllTransfers_clicked()
{
    emit userActivity();

    if (mRefreshTransferTime->isActive())
    {
        mRefreshTransferTime->stop();
    }

    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    toggleTab(ALL_TRANSFERS_TAB);

    mUi->bPause->setVisible(true);

    mTransferStatesFilter.clear();
    mTransferStatesFilter.insert(MegaTransfer::STATE_ACTIVE);
    mTransferStatesFilter.insert(MegaTransfer::STATE_PAUSED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_COMPLETING);
    mTransferStatesFilter.insert(MegaTransfer::STATE_FAILED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_QUEUED);
    mTransferStatesFilter.insert(MegaTransfer::STATE_RETRYING);
    mUi->wTransfers->transferStateFilterChanged(mTransferStatesFilter);

    mTransferTypesFilter.insert(MegaTransfer::TYPE_DOWNLOAD);
    mTransferTypesFilter.insert(MegaTransfer::TYPE_UPLOAD);
    mUi->wTransfers->transferTypeFilterChanged(mTransferTypesFilter);

    mUi->lCurrentContent->setText(tr("All Transfers"));

    updateState();
}

void TransferManager::updatePauseState(bool isPaused, QString toolTipText)
{
    static bool prevState (!isPaused);

    if (isPaused != prevState)
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

        prevState = isPaused;
    }
}

void TransferManager::updateState()
{
    bool isPaused (false);
    QString bPauseTooltip;
    bool areTransfersActive(mUi->wTransfers->areTransfersActive());

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
            onTransfersActive(areTransfersActive);
            isPaused = mPreferences->getDownloadsPaused();
            bPauseTooltip = QLatin1String("Downloads");
            break;
        }
        case UPLOADS_TAB:
        {
            onTransfersActive(areTransfersActive);
            isPaused = mPreferences->getUploadsPaused();
            bPauseTooltip = QLatin1String("Uploads");
            break;
        }
        case COMPLETED_TAB:
        {
            onTransfersActive(areTransfersActive);
            break;
        }
        default:
            break;
    }

//    if (areTransfersActive)
//    {
//        mUi->sStatus->setCurrentWidget(mUi->wSpeedAndClear);
//    }
//    else
//    {
//        mUi->sStatus->setCurrentWidget(mUi->pUpToDate);
//    }

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
    mUi->wTransfers->textFilterChanged(QRegExp(mUi->leSearchField->text(), Qt::CaseInsensitive));

    mUi->lTextSearch->setText(mUi->lTextSearch->fontMetrics().elidedText(mUi->leSearchField->text(),
                                                                         Qt::ElideMiddle,
                                                                         mUi->lTextSearch->width()-24));
    mUi->sCurrentContent->setCurrentWidget(mUi->pSearchHeader);
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

void TransferManager::on_tPauseResumeAll_clicked()
{
    // TODO: only selected ?
    on_bPause_clicked();
}

void TransferManager::on_tCancelAll_clicked()
{
    // TODO: only selected ?
    on_bClearAll_clicked();
}

void TransferManager::on_bArchives_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_ARCHIVE))
    {
        mUi->fArchives->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_ARCHIVE);
    }
    else
    {
        mUi->fArchives->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_ARCHIVE);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::on_bDocuments_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_DOCUMENT))
    {
        mUi->fDocuments->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_DOCUMENT);
    }
    else
    {
        mUi->fDocuments->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_DOCUMENT);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::on_bImages_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_IMAGE))
    {
        mUi->fImages->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_IMAGE);
    }
    else
    {
        mUi->fImages->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_IMAGE);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::on_bMusic_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_AUDIO))
    {
        mUi->fMusic->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_AUDIO);
    }
    else
    {
        mUi->fMusic->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_AUDIO);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::on_bVideos_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_VIDEO))
    {
        mUi->fVideos->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_VIDEO);
    }
    else
    {
        mUi->fVideos->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_VIDEO);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::on_bOther_clicked()
{
    if (mFileTypesFilter.contains(TransferData::TYPE_OTHER))
    {
        mUi->fOther->setProperty("itsOn", false);
        mFileTypesFilter.remove(TransferData::TYPE_OTHER);
    }
    else
    {
        mUi->fOther->setProperty("itsOn", true);
        mFileTypesFilter.insert(TransferData::TYPE_OTHER);
    }
    mUi->wTransfers->fileTypeFilterChanged(mFileTypesFilter);
    setStyleSheet(styleSheet());
}

void TransferManager::refreshFinishedTime()
{
//    QWidget *w = mUi->wTransfers->currentWidget();
//    if (w == mUi->wCompleted)
//    {
//        mUi->wCompleted->getModel()->refreshTransfers();
//    }
}

void TransferManager::toggleTab(TM_TABS tab)
{
    if (mCurrentTab != tab)
    {
        mTabFramesToggleGroup[mCurrentTab]->setProperty("itsOn", false);
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);
        mCurrentTab = tab;
    }
}

void TransferManager::onTransfersActive(bool exists)
{
    mUi->bClearAll->setEnabled(exists);
}

void TransferManager::updateNumberOfCompletedTransfers(int num)
{
    if (num == 0)
    {
        mUi->lCompletedNb->setVisible(false);
        return;
    }

    QString numString;
    if (num > TransferManager::COMPLETED_ITEMS_LIMIT)
    {
        numString = QString::fromUtf8("+") + QString::number(TransferManager::COMPLETED_ITEMS_LIMIT);
    }
    else
    {
        numString = QString::number(num);
    }

    mUi->lCompletedNb->setText(numString);
    mUi->lCompletedNb->setVisible(true);
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
