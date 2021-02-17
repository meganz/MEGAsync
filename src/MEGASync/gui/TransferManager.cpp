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
    mThreadPool(ThreadPoolSingleton::getInstance())
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

    QPointer<TransferManager> transferManager = this;

    mThreadPool->push([this, transferManager]()
    {//thread pool function

        if (!transferManager)
        {
            return;
        }

        std::shared_ptr<MegaTransferData> transferData(mMegaApi->getTransferData());

        Utilities::queueFunctionInAppThread([this, transferData, transferManager]()
        {//queued function

            if (transferManager) //Check if this is not deleted
            {
                mUi->wActiveTransfers->setupTransfers();
//                mUi->wUploads->setupTransfers(transferData, QTransfersModel::TYPE_UPLOAD);
//                mUi->wDownloads->setupTransfers(transferData, QTransfersModel::TYPE_DOWNLOAD);
            }


        });//end of queued function

    });// end of thread pool function

    if (((MegaApplication *)qApp)->getFinishedTransfers().size() > 0)
    {
        mUi->fCompleted->setVisible(true);
    }
    else
    {
        mUi->fCompleted->setVisible(false);
    }

    mUi->wCompleted->setupFinishedTransfers(((MegaApplication *)qApp)->getFinishedTransfers());
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
    mUi->tPauseResumeAll->setVisible(false);
    mUi->tCancelAll->setVisible(false);

    mUi->wTransfers->setCurrentWidget(mUi->wCompleted);
    mUi->wCompleted->refreshTransferItems();
    mUi->fCompleted->setVisible(true);

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
    mUi->tPauseResumeAll->setVisible(true);
    mUi->tCancelAll->setVisible(true);

    mUi->lCurrentContent->setText(tr("Downloads"));

    mUi->wTransfers->setCurrentWidget(mUi->wDownloads);
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
    mUi->tPauseResumeAll->setVisible(true);
    mUi->tCancelAll->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wUploads);

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
    mUi->tPauseResumeAll->setVisible(true);
    mUi->tCancelAll->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wActiveTransfers);

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
    QWidget *w = mUi->wTransfers->currentWidget();
    bool isPaused (false);
    QString bPauseTooltip;

    if (w == mUi->wActiveTransfers)
    {
        onTransfersActive(mUi->wActiveTransfers->areTransfersActive());
        isPaused = mPreferences->getGlobalPaused();
        mUi->wActiveTransfers->pausedTransfers(isPaused);
        mUi->wActiveTransfers->refreshTransferItems();
        bPauseTooltip = QLatin1String("All");
    }
    else if (w == mUi->wDownloads)
    {
        onTransfersActive(mUi->wDownloads->areTransfersActive());
        isPaused = mPreferences->getDownloadsPaused();
        mUi->wDownloads->pausedTransfers(isPaused);
        mUi->wDownloads->refreshTransferItems();
        bPauseTooltip = QLatin1String("Downloads");
    }
    else if (w == mUi->wUploads)
    {
        onTransfersActive(mUi->wUploads->areTransfersActive());
        isPaused = mPreferences->getUploadsPaused();
        mUi->wUploads->pausedTransfers(isPaused);
        mUi->wUploads->refreshTransferItems();
        bPauseTooltip = QLatin1String("Uploads");
    }
    else if (w == mUi->wCompleted)
    {
        onTransfersActive(mUi->wCompleted->areTransfersActive());
    }

    if (mUi->wActiveTransfers->areTransfersActive())
    {
        mUi->wStatus->setCurrentWidget(mUi->wSpeedAndClear);
    }
    else
    {
        mUi->wStatus->setCurrentWidget(mUi->pUpToDate);
    }

    updatePauseState(isPaused, bPauseTooltip);
    setStyleSheet(styleSheet());
}

void TransferManager::disableGetLink(bool disable)
{
    mUi->wCompleted->disableGetLink(disable);
}

void TransferManager::on_bPause_clicked()
{
    emit userActivity();

    QWidget *w = mUi->wTransfers->currentWidget();
    if (w == mUi->wActiveTransfers)
    {
        mMegaApi->pauseTransfers(!mPreferences->getGlobalPaused());
    }
    else if(w == mUi->wDownloads)
    {
        mMegaApi->pauseTransfers(!mPreferences->getDownloadsPaused(), MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == mUi->wUploads)
    {
        mMegaApi->pauseTransfers(!mPreferences->getUploadsPaused(), MegaTransfer::TYPE_UPLOAD);
    }
}

void TransferManager::on_bClearAll_clicked()
{
    QWidget *w = mUi->wTransfers->currentWidget();
    QPointer<TransferManager> dialog = QPointer<TransferManager>(this);

    if (w != mUi->wCompleted)
    {
        if (QMegaMessageBox::warning(nullptr,
                                 QString::fromUtf8("MEGAsync"),
                                 tr("Are you sure you want to cancel all transfers?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) != QMessageBox::Yes
                || !dialog)
        {
            return;
        }
    }

    if (w == mUi->wActiveTransfers)
    {
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == mUi->wDownloads)
    {
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == mUi->wUploads)
    {
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    }
    else if(w == mUi->wCompleted)
    {
        mUi->wCompleted->clearTransfers();
    }
}

void TransferManager::on_bSearch_clicked()
{
    mUi->wTitleAndSearch->setCurrentWidget(mUi->pSearch);
    mUi->leSearchField->setText(QString());
}

void TransferManager::on_tSearchIcon_clicked()
{
    mUi->fSearchString->setProperty("itsOn", true);
    mUi->bSearchString->setText(mUi->leSearchField->text());
    // Todo: use a stacked panel instead with search + number of results
    mUi->lCurrentContent->setText(tr("Search: ") + mUi->leSearchField->text());
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

void TransferManager::refreshFinishedTime()
{
    QWidget *w = mUi->wTransfers->currentWidget();
    if (w == mUi->wCompleted)
    {
        mUi->wCompleted->getModel()->refreshTransfers();
    }
}

void TransferManager::toggleTab(TM_TABS tab)
{
    static TM_TABS prevTab = COMPLETED_TAB;

    if (prevTab != tab)
    {
        mTabFramesToggleGroup[prevTab]->setProperty("itsOn", false);
        mTabFramesToggleGroup[tab]->setProperty("itsOn", true);
        prevTab = tab;
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
        mUi->retranslateUi(this);
        QWidget *w = mUi->wTransfers->currentWidget();
        if (w == mUi->wActiveTransfers)
        {
            on_tAllTransfers_clicked();
        }
        else if (w == mUi->wUploads)
        {
            on_tUploads_clicked();
        }
        else if (w == mUi->wDownloads)
        {
            on_tDownloads_clicked();
        }
        else if (w == mUi->wCompleted)
        {
            on_tCompleted_clicked();
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

