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
    mRefreshTransferTime(new QTimer(this)),
    mAddMenu(nullptr),
    mImportLinksAction(nullptr),
    mUploadAction(nullptr),
    mDownloadAction(nullptr),
    mSettingsAction(nullptr),
    mMegaApi(megaApi),
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
        mUi->wCompletedTab->setVisible(true);
    }
    else
    {
        mUi->wCompletedTab->setVisible(false);
    }

    mUi->wCompleted->setupFinishedTransfers(((MegaApplication *)qApp)->getFinishedTransfers());
    updateNumberOfCompletedTransfers(((MegaApplication *)qApp)->getNumUnviewedTransfers());

    connect(mUi->wCompleted->getModel(), SIGNAL(noTransfers()), this, SLOT(updateState()));
    connect(mUi->wCompleted->getModel(), SIGNAL(onTransferAdded()), this, SLOT(updateState()));

    updatePauseState();
    on_tAllTransfers_clicked();
    createAddMenu();

    Platform::enableDialogBlur(this);
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
    mUi->wCompletedTab->setVisible(true);

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

void TransferManager::createAddMenu()
{
    if (mAddMenu)
    {
        QList<QAction *> actions = mAddMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            mAddMenu->removeAction(actions[i]);
        }
#ifdef _WIN32
        mAddMenu->deleteLater();
#endif
    }
#ifndef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    else
#endif
    {
        mAddMenu = new QMenu(this);
#ifdef __APPLE__
        addMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));
#else
        mAddMenu->setStyleSheet(QString::fromAscii("QMenu { border: 1px solid #B8B8B8; border-radius: 5px; background: #ffffff; padding-top: 5px; padding-bottom: 5px;}"));
#endif
    }

    if (mImportLinksAction)
    {
        mImportLinksAction->deleteLater();
        mImportLinksAction = nullptr;
    }

    mImportLinksAction = new MenuItemAction(tr("Open links"),
                                            QIcon(QString::fromAscii("://images/ico_Import_links.png")));
    connect(mImportLinksAction, SIGNAL(triggered()), qApp, SLOT(importLinks()), Qt::QueuedConnection);

    if (mUploadAction)
    {
        mUploadAction->deleteLater();
        mUploadAction = nullptr;
    }

    mUploadAction = new MenuItemAction(tr("Upload"), QIcon(QString::fromAscii("://images/ico_upload.png")));
    connect(mUploadAction, SIGNAL(triggered()), qApp, SLOT(uploadActionClicked()), Qt::QueuedConnection);

    if (mDownloadAction)
    {
        mDownloadAction->deleteLater();
        mDownloadAction = nullptr;
    }

    mDownloadAction = new MenuItemAction(tr("Download"), QIcon(QString::fromAscii("://images/ico_download.png")));
    connect(mDownloadAction, SIGNAL(triggered()), qApp, SLOT(downloadActionClicked()), Qt::QueuedConnection);

    if (mSettingsAction)
    {
        mSettingsAction->deleteLater();
        mSettingsAction = nullptr;
    }

    mSettingsAction = new MenuItemAction(QCoreApplication::translate("Platform", Platform::settingsString),
                                         QIcon(QString::fromUtf8("://images/ico_preferences.png")));
    connect(mSettingsAction, SIGNAL(triggered()), qApp, SLOT(openSettings()), Qt::QueuedConnection);

    mAddMenu->addAction(mImportLinksAction);
    mAddMenu->addAction(mUploadAction);
    mAddMenu->addAction(mDownloadAction);
    mAddMenu->addSeparator();
    mAddMenu->addAction(mSettingsAction);
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

    mUi->lCompleted->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    mUi->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    mUi->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tCompleted->setStyleSheet(QString::fromUtf8("color: #333333; padding-right: 19px;"));
    mUi->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->bClearAll->setText(tr("Clear all"));
    mUi->bPause->setVisible(false);
    mUi->wTransfers->setCurrentWidget(mUi->wCompleted);
    updateState();
    updatePauseState();
    mUi->wCompleted->refreshTransferItems();
    mUi->wCompletedTab->setVisible(true);
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

    mUi->lDownloads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    mUi->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    mUi->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999; padding-right: 5px;"));
    mUi->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tDownloads->setStyleSheet(QString::fromUtf8("color: #333333;"));
    mUi->bClearAll->setText(tr("Cancel all"));
    mUi->bPause->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wDownloads);
    updateState();
    updatePauseState();
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

    mUi->lUploads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    mUi->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent; padding-right: 5px;"));
    mUi->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    mUi->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tUploads->setStyleSheet(QString::fromUtf8("color: #333333;"));
    mUi->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->bClearAll->setText(tr("Cancel all"));
    mUi->bPause->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wUploads);
    updateState();
    updatePauseState();
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

    mUi->lAll->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    mUi->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent; padding-right: 5px;"));
    mUi->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    mUi->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    mUi->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #333333;"));
    mUi->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    mUi->bClearAll->setText(tr("Cancel all"));
    mUi->bPause->setVisible(true);

    mUi->wTransfers->setCurrentWidget(mUi->wActiveTransfers);
    updateState();
    updatePauseState();
}

void TransferManager::on_bAdd_clicked()
{
    emit userActivity();

#ifdef _WIN32 // win32 needs to recreate menu to fix scaling qt issue
    createAddMenu();
#endif

    auto menuWidthInitialPopup = mAddMenu->sizeHint().width();
    auto displayedMenu = mAddMenu;
    QPoint point = mUi->bAdd->mapToGlobal(QPoint(mUi->bAdd->width() , mUi->bAdd->height() + 4));
    QPoint p = !point.isNull() ? point - QPoint(mAddMenu->sizeHint().width(), 0) : QCursor::pos();


#ifdef __APPLE__
    addMenu->exec(p);
#else
    mAddMenu->popup(p);

    // Menu width might be incorrect the first time it's shown.
    // This works around that and repositions the menu at the expected position afterwards.
    if (!point.isNull())
    {
        QPoint pointValue = point;
        QTimer::singleShot(1, displayedMenu, [displayedMenu, pointValue, menuWidthInitialPopup] () {
            displayedMenu->update();
            displayedMenu->ensurePolished();
            if (menuWidthInitialPopup != displayedMenu->sizeHint().width())
            {
                QPoint p = pointValue - QPoint(displayedMenu->sizeHint().width(), 0);
                displayedMenu->update();
                displayedMenu->popup(p);
            }
        });
    }
#endif
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

    if (isPaused)
    {
        mUi->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")));
        mUi->bPause->setText(tr("Resume"));
    }
    else
    {
        mUi->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
        mUi->bPause->setText(tr("Pause"));
    }
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
        mUi->bNumberCompleted->setVisible(false);
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

    mUi->bNumberCompleted->setText(numString);
    mUi->bNumberCompleted->setVisible(true);
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        mUi->retranslateUi(this);
        createAddMenu();
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

