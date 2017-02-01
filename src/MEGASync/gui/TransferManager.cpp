#include "TransferManager.h"
#include "QMegaMessageBox.h"
#include "ui_TransferManager.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "platform/Platform.h"

using namespace mega;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferManager)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    Qt::WindowFlags flags =  Qt::Window | Qt::FramelessWindowHint;
    this->setWindowFlags(flags);
    preferences = Preferences::instance();

    addMenu = NULL;
    importLinksAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    settingsAction = NULL;
    this->megaApi = megaApi; 

    MegaTransferData *transferData = megaApi->getTransferData();
    notificationNumber = transferData->getNotificationNumber();
    ui->wUploads->setupTransfers(transferData, QTransfersModel::TYPE_UPLOAD);
    ui->wDownloads->setupTransfers(transferData, QTransfersModel::TYPE_DOWNLOAD);

    MegaTransfer *firstUpload = NULL;
    MegaTransfer *firstDownload = NULL;
    if (transferData->getNumUploads())
    {
        firstUpload = megaApi->getTransferByTag(transferData->getUploadTag(0));
    }
    if (transferData->getNumDownloads())
    {
        firstDownload = megaApi->getTransferByTag(transferData->getDownloadTag(0));
    }
    ui->wActiveTransfers->init(megaApi, firstUpload, firstDownload);
    delete firstUpload;
    delete firstDownload;

    ui->wCompleted->setupFinishedTransfers(((MegaApplication *)qApp)->getFinishedTransfers());
    updateNumberOfCompletedTransfers(((MegaApplication *)qApp)->getNumUnviewedTransfers());
    delete transferData;

    updatePauseState();
    on_tAllTransfers_clicked();
    createAddMenu();

    Platform::enableDialogBlur(this);
}

TransferManager::~TransferManager()
{
    delete ui;
}

void TransferManager::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    if (!transfer->getPriority() || transfer->isStreamingTransfer()
            || transfer->isFolderTransfer() || notificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    ui->wUploads->getModel()->onTransferStart(api, transfer);
    ui->wDownloads->getModel()->onTransferStart(api, transfer);
    ui->wActiveTransfers->onTransferStart(api, transfer);
}

void TransferManager::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    ui->wCompleted->getModel()->onTransferFinish(api, transfer, e);
    if (!transfer->getPriority() || notificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    ui->wUploads->getModel()->onTransferFinish(api, transfer, e);
    ui->wDownloads->getModel()->onTransferFinish(api, transfer, e);
    ui->wActiveTransfers->onTransferFinish(api, transfer, e);
}

void TransferManager::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    if (!transfer->getPriority() || transfer->isStreamingTransfer()
            || transfer->isFolderTransfer() || notificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    ui->wUploads->getModel()->onTransferUpdate(api, transfer);
    ui->wDownloads->getModel()->onTransferUpdate(api, transfer);
    ui->wActiveTransfers->onTransferUpdate(api, transfer);
}

void TransferManager::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    if (!transfer->getPriority() || transfer->isStreamingTransfer()
            || transfer->isFolderTransfer() || notificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    ui->wUploads->getModel()->onTransferTemporaryError(api, transfer, e);
    ui->wDownloads->getModel()->onTransferTemporaryError(api, transfer, e);
    ui->wActiveTransfers->onTransferTemporaryError(api, transfer, e);
}

void TransferManager::createAddMenu()
{
    if (!addMenu)
    {
        addMenu = new QMenu(this);
        addMenu->setStyleSheet(QString::fromAscii("QMenu {background: #ffffff; padding-top: 8px; padding-bottom: 8px;}"));

    }
    else
    {
        QList<QAction *> actions = addMenu->actions();
        for (int i = 0; i < actions.size(); i++)
        {
            addMenu->removeAction(actions[i]);
        }
    }

    if (importLinksAction)
    {
        importLinksAction->deleteLater();
        importLinksAction = NULL;
    }

    importLinksAction = new TransferMenuItemAction(tr("Import links"), QIcon(QString::fromAscii("://images/get_link_ico.png")), QIcon(QString::fromAscii("://images/get_link_ico_white.png")));
    connect(importLinksAction, SIGNAL(triggered()), qApp, SLOT(importLinks()));

    if (uploadAction)
    {
        uploadAction->deleteLater();
        uploadAction = NULL;
    }

    uploadAction = new TransferMenuItemAction(tr("Upload to MEGA"), QIcon(QString::fromAscii("://images/upload_to_mega_ico.png")), QIcon(QString::fromAscii("://images/upload_to_mega_ico_white.png")));
    connect(uploadAction, SIGNAL(triggered()), qApp, SLOT(uploadActionClicked()));

    if (downloadAction)
    {
        downloadAction->deleteLater();
        downloadAction = NULL;
    }

    downloadAction = new TransferMenuItemAction(tr("Download from MEGA"), QIcon(QString::fromAscii("://images/download_from_mega_ico.png")), QIcon(QString::fromAscii("://images/download_from_mega_ico_white.png")));
    connect(downloadAction, SIGNAL(triggered()), qApp, SLOT(downloadActionClicked()));

    if (settingsAction)
    {
        settingsAction->deleteLater();
        settingsAction = NULL;
    }

#ifndef __APPLE__
    settingsAction = new TransferMenuItemAction(tr("Settings"), QIcon(QString::fromAscii("://images/settings_ico.png")), QIcon(QString::fromAscii("://images/settings_ico_white.png")));
#else
    settingsAction = new TransferMenuItemAction(tr("Preferences"), QIcon(QString::fromAscii("://images/settings_ico.png")), QIcon(QString::fromAscii("://images/settings_ico_white.png")));
#endif
    connect(settingsAction, SIGNAL(triggered()), qApp, SLOT(openSettings()));

    addMenu->addAction(importLinksAction);
    addMenu->addAction(uploadAction);
    addMenu->addAction(downloadAction);
    addMenu->addSeparator();
    addMenu->addAction(settingsAction);
}

void TransferManager::on_tCompleted_clicked()
{
    // Disable tracking of completed transfers and reset value
    emit viewedCompletedTransfers();
    emit completedTransfersTabActive(true);

    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #333333; padding-right: 19px;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->bClearAll->setText(tr("Clear all"));
    ui->bPause->setVisible(false);
    ui->wTransfers->setCurrentWidget(ui->wCompleted);
    updateState();
    updatePauseState();
    ui->wCompleted->refreshTransferItems();
}

void TransferManager::on_tDownloads_clicked()
{
    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999; padding-right: 5px;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->bClearAll->setText(tr("Cancel all"));
    ui->bPause->setVisible(true);

    ui->wTransfers->setCurrentWidget(ui->wDownloads);
    updateState();
    updatePauseState();
}

void TransferManager::on_tUploads_clicked()
{
    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent; padding-right: 5px;"));
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->bClearAll->setText(tr("Cancel all"));
    ui->bPause->setVisible(true);

    ui->wTransfers->setCurrentWidget(ui->wUploads);
    updateState();
    updatePauseState();
}

void TransferManager::on_tAllTransfers_clicked()
{
    // Enable tracking of completed transfers
    emit completedTransfersTabActive(false);

    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent; padding-right: 5px;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->bClearAll->setText(tr("Cancel all"));
    ui->bPause->setVisible(true);

    ui->wTransfers->setCurrentWidget(ui->wActiveTransfers);
    updateState();
    updatePauseState();
}

void TransferManager::on_bAdd_clicked()
{
    QPoint point = ui->bAdd->mapToGlobal(QPoint(ui->bAdd->width() , ui->bAdd->height() + 4));
    QPoint p = !point.isNull() ? point - QPoint(addMenu->sizeHint().width(), 0) : QCursor::pos();

#ifdef __APPLE__
    addMenu->exec(p);
#else
    addMenu->popup(p);
#endif
}

void TransferManager::on_bClose_clicked()
{
    close();
}

void TransferManager::updatePauseState()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (w == ui->wActiveTransfers)
    {
        if (preferences->getGlobalPaused())
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")));
            ui->bPause->setText(tr("Resume"));
        }
        else
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
            ui->bPause->setText(tr("Pause"));
        }
    }
    else if (w == ui->wDownloads)
    {
        ui->wDownloads->pausedTransfers(preferences->getDownloadsPaused());
        if (preferences->getDownloadsPaused())
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")));
            ui->bPause->setText(tr("Resume"));

        }
        else
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
            ui->bPause->setText(tr("Pause"));
        }
        ui->wDownloads->refreshTransferItems();
    }
    else if (w == ui->wUploads)
    {
        ui->wUploads->pausedTransfers(preferences->getUploadsPaused());
        if (preferences->getUploadsPaused())
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")));
            ui->bPause->setText(tr("Resume"));

        }
        else
        {
            ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
            ui->bPause->setText(tr("Pause"));
        }
        ui->wUploads->refreshTransferItems();
    }
}

void TransferManager::updateState()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (w == ui->wActiveTransfers)
    {
        onTransfersActive(ui->wActiveTransfers->areTransfersActive());
    }
    else if (w == ui->wDownloads)
    {
        onTransfersActive(ui->wDownloads->areTransfersActive());
        ui->wDownloads->pausedTransfers(preferences->getDownloadsPaused());
    }
    else if (w == ui->wUploads)
    {
        onTransfersActive(ui->wUploads->areTransfersActive());
        ui->wUploads->pausedTransfers(preferences->getUploadsPaused());
    }
    else if (w == ui->wCompleted)
    {
        onTransfersActive(ui->wCompleted->areTransfersActive());
    }
}

void TransferManager::disableGetLink(bool disable)
{
    ui->wCompleted->disableGetLink(disable);
}

void TransferManager::on_bPause_clicked()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (w == ui->wActiveTransfers)
    {
        megaApi->pauseTransfers(!preferences->getGlobalPaused());
    }
    else if(w == ui->wDownloads)
    {
        megaApi->pauseTransfers(!preferences->getDownloadsPaused(), MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == ui->wUploads)
    {
        megaApi->pauseTransfers(!preferences->getUploadsPaused(), MegaTransfer::TYPE_UPLOAD);
    }
}

void TransferManager::on_bClearAll_clicked()
{
    QWidget *w = ui->wTransfers->currentWidget();

    if (w != ui->wCompleted)
    {
        if(QMegaMessageBox::warning(this,
                                 QString::fromUtf8("MEGAsync"),
                                 tr("Are you sure you want to cancel all transfers?"),
                                 Utilities::getDevicePixelRatio(), QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }

    }

    if (w == ui->wActiveTransfers)
    {
        megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        megaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == ui->wDownloads)
    {
        megaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == ui->wUploads)
    {
        megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    }
    else if(w == ui->wCompleted)
    {
        ui->wCompleted->clearTransfers();
    }

    updateState();
}

void TransferManager::onTransfersActive(bool exists)
{
    ui->bClearAll->setEnabled(exists);
}

void TransferManager::updateNumberOfCompletedTransfers(int num)
{
    if (!num)
    {
        ui->bNumberCompleted->setVisible(false);
        return;
    }

    if (num > TransferManager::COMPLETED_ITEMS_LIMIT)
    {
        ui->bNumberCompleted->setVisible(true);
        ui->bNumberCompleted->setText(QString::fromUtf8("+") + QString::number(TransferManager::COMPLETED_ITEMS_LIMIT));
        return;
    }

    ui->bNumberCompleted->setVisible(true);
    ui->bNumberCompleted->setText(QString::number(num));
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
        createAddMenu();
        QWidget *w = ui->wTransfers->currentWidget();
        if (w == ui->wActiveTransfers)
        {
            on_tAllTransfers_clicked();
        }
        else if (w == ui->wUploads)
        {
            on_tUploads_clicked();
        }
        else if (w == ui->wDownloads)
        {
            on_tDownloads_clicked();
        }
        else if (w == ui->wCompleted)
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
        if (dragPosition.x() != -1)
        {
            move(event->globalPos() - dragPosition);
            event->accept();
        }
    }
}

void TransferManager::mousePressEvent(QMouseEvent *event)
{
    if (event->button() == Qt::LeftButton)
    {
        dragPosition = event->globalPos() - frameGeometry().topLeft();
        event->accept();
    }
}

void TransferManager::mouseReleaseEvent(QMouseEvent *event)
{
    dragPosition = QPoint(-1, -1);
}

