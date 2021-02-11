#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "platform/Platform.h"

#include <QMouseEvent>

using namespace mega;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    mUi(new Ui::TransferManager),
    mPreferences(Preferences::instance()),
    mMegaApi(megaApi),
    mRefreshTransferTime(new QTimer(this)),
    mThreadPool(ThreadPoolSingleton::getInstance())
{
    mUi->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);

#ifndef __APPLE__
    Qt::WindowFlags flags =  Qt::Window | Qt::FramelessWindowHint;
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

        MegaTransfer* firstUpload   = nullptr;
        MegaTransfer* firstDownload = nullptr;

        if (transferData->getNumUploads() > 0)
        {
            firstUpload = mMegaApi->getTransferByTag(transferData->getUploadTag(0));
        }
        if (transferData->getNumDownloads() > 0)
        {
            firstDownload = mMegaApi->getTransferByTag(transferData->getDownloadTag(0));
        }

        Utilities::queueFunctionInAppThread([this, firstDownload, firstUpload, transferData, transferManager]()
        {//queued function

            if (transferManager) //Check if this is not deleted
            {
                mNotificationNumber = transferData->getNotificationNumber();
                mUi->wUploads->setupTransfers(transferData, QTransfersModel::TYPE_UPLOAD);
                mUi->wDownloads->setupTransfers(transferData, QTransfersModel::TYPE_DOWNLOAD);

                mUi->wActiveTransfers->init(mMegaApi, firstUpload, firstDownload);
            }
            delete firstUpload;
            delete firstDownload;

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

    connect(mUi->wCompleted->getModel(), SIGNAL(noTransfers()), this, SLOT(updateState()));
    connect(mUi->wCompleted->getModel(), SIGNAL(onTransferAdded()), this, SLOT(updateState()));


    QObject::connect(mUi->bImportLinks, SIGNAL(clicked()), qApp, SLOT(importLinks()));
    QObject::connect(mUi->tCogWheel, SIGNAL(clicked()), qApp, SLOT(openSettings()));
    QObject::connect(mUi->bDownload, SIGNAL(clicked()), qApp, SLOT(downloadActionClicked()));
    QObject::connect(mUi->bUpload, SIGNAL(clicked()), qApp, SLOT(uploadActionClicked()));


    Platform::enableDialogBlur(this);

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

void TransferManager::onTransferStart(MegaApi* api, MegaTransfer* transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    mUi->wActiveTransfers->onTransferStart(api, transfer);

    if (!transfer->getPriority())
    {
        return;
    }

    QTransfersModel* model = mUi->wUploads->getModel();
    if (model)
    {
        model->onTransferStart(api, transfer);
    }

    model = mUi->wDownloads->getModel();
    if (model)
    {
        model->onTransferStart(api, transfer);
    }
}

void TransferManager::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    mUi->wCompleted->getModel()->onTransferFinish(api, transfer, e);
    mUi->fCompleted->setVisible(true);

    if (mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    mUi->wActiveTransfers->onTransferFinish(api, transfer, e);

    if (!transfer->getPriority())
    {
        return;
    }

    QTransfersModel* model = mUi->wUploads->getModel();
    if (model)
    {
        model->onTransferFinish(api, transfer, e);
    }

    model = mUi->wDownloads->getModel();
    if (model)
    {
        model->onTransferFinish(api, transfer, e);
    }
}

void TransferManager::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer() || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    mUi->wActiveTransfers->onTransferUpdate(api, transfer);

    if (!transfer->getPriority())
    {
        return;
    }

    QTransfersModel* model = mUi->wUploads->getModel();
    if (model)
    {
        model->onTransferUpdate(api, transfer);
    }

    model = mUi->wDownloads->getModel();
    if (model)
    {
        model->onTransferUpdate(api, transfer);
    }
}

void TransferManager::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    mUi->wActiveTransfers->onTransferTemporaryError(api, transfer, e);

    if (!transfer->getPriority())
    {
        return;
    }

    QTransfersModel *model = mUi->wUploads->getModel();
    if (model)
    {
        model->onTransferTemporaryError(api, transfer, e);
    }

    model = mUi->wDownloads->getModel();
    if (model)
    {
        model->onTransferTemporaryError(api, transfer, e);
    }
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

    mUi->fDownloads->setProperty("itsOn", false);
    mUi->fUploads->setProperty("itsOn", false);
    mUi->fAllTransfers->setProperty("itsOn", false);
    mUi->fCompleted->setProperty("itsOn", true);

    mUi->bPause->setVisible(false);
    mUi->wTransfers->setCurrentWidget(mUi->wCompleted);
    updateState();
    updatePauseState();
    mUi->wCompleted->refreshTransferItems();
    mUi->fCompleted->setVisible(true);

    mUi->lCurrentContent->setText(tr("Finished"));

    setStyleSheet(styleSheet());
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

    mUi->fDownloads->setProperty("itsOn", true);
    mUi->fUploads->setProperty("itsOn", false);
    mUi->fAllTransfers->setProperty("itsOn", false);
    mUi->fCompleted->setProperty("itsOn", false);

    mUi->bPause->setVisible(true);

    mUi->lCurrentContent->setText(tr("Downloads"));

    mUi->wTransfers->setCurrentWidget(mUi->wDownloads);
    updateState();
    updatePauseState();

    setStyleSheet(styleSheet());
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

    mUi->fDownloads->setProperty("itsOn", false);
    mUi->fUploads->setProperty("itsOn", true);
    mUi->fAllTransfers->setProperty("itsOn", false);
    mUi->fCompleted->setProperty("itsOn", false);
    mUi->bPause->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wUploads);

    mUi->lCurrentContent->setText(tr("Uploads"));

    updateState();
    updatePauseState();

    setStyleSheet(styleSheet());
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

    mUi->fDownloads->setProperty("itsOn", false);
    mUi->fUploads->setProperty("itsOn", false);
    mUi->fAllTransfers->setProperty("itsOn", true);
    mUi->fCompleted->setProperty("itsOn", false);
    mUi->bPause->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wActiveTransfers);

    mUi->lCurrentContent->setText(tr("All Transfers"));

    updateState();
    updatePauseState();


    setStyleSheet(styleSheet());
}

void TransferManager::on_bClose_clicked()
{
    emit userActivity();
    close();
}

void TransferManager::updatePauseState()
{
    QWidget *w = mUi->wTransfers->currentWidget();
    bool isPaused = false;

    if (w == mUi->wActiveTransfers)
    {
        isPaused = mPreferences->getGlobalPaused();
    }
    else if (w == mUi->wDownloads)
    {
        isPaused = mPreferences->getDownloadsPaused();
        mUi->wDownloads->pausedTransfers(isPaused);
        mUi->wDownloads->refreshTransferItems();
    }
    else if (w == mUi->wUploads)
    {
        isPaused = mPreferences->getUploadsPaused();
        mUi->wUploads->pausedTransfers(isPaused);
        mUi->wUploads->refreshTransferItems();
    }
    else
    {
        return;
    }

    setProperty("isPaused", isPaused);
}

void TransferManager::updateState()
{
    QWidget *w = mUi->wTransfers->currentWidget();
    if (w == mUi->wActiveTransfers)
    {
        onTransfersActive(mUi->wActiveTransfers->areTransfersActive());
    }
    else if (w == mUi->wDownloads)
    {
        onTransfersActive(mUi->wDownloads->areTransfersActive());
        mUi->wDownloads->pausedTransfers(mPreferences->getDownloadsPaused());
    }
    else if (w == mUi->wUploads)
    {
        onTransfersActive(mUi->wUploads->areTransfersActive());
        mUi->wUploads->pausedTransfers(mPreferences->getUploadsPaused());
    }
    else if (w == mUi->wCompleted)
    {
        onTransfersActive(mUi->wCompleted->areTransfersActive());
    }
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

    setStyleSheet(styleSheet());
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

void TransferManager::refreshFinishedTime()
{
    QWidget *w = mUi->wTransfers->currentWidget();
    if (w == mUi->wCompleted)
    {
        mUi->wCompleted->getModel()->refreshTransfers();
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

