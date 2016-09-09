#include "TransferManager.h"
#include <QMessageBox>
#include "ui_TransferManager.h"
#include "MegaApplication.h"

using namespace mega;

TransferManager::TransferManager(MegaApi *megaApi, QWidget *parent) :
    QDialog(parent),
    ui(new Ui::TransferManager)
{
    ui->setupUi(this);
    setAttribute(Qt::WA_QuitOnClose, false);
    setAttribute(Qt::WA_DeleteOnClose, true);
    Qt::WindowFlags flags =  Qt::Window | Qt::WindowSystemMenuHint
                                | Qt::WindowMinimizeButtonHint
                                | Qt::WindowCloseButtonHint;
    this->setWindowFlags(flags);
    preferences = Preferences::instance();

    addMenu = NULL;
    importLinksAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    settingsAction = NULL;
    this->megaApi = megaApi;
    delegateListener = new QTMegaTransferListener(megaApi, this);

    MegaTransferData *transferData = megaApi->getTransferData(delegateListener);
    ui->wUploads->setupTransfers(transferData, QTransfersModel::TYPE_UPLOAD);
    ui->wDownloads->setupTransfers(transferData, QTransfersModel::TYPE_DOWNLOAD);
    ui->wAllTransfers->setupTransfers(transferData, QTransfersModel::TYPE_ALL);
    ui->wCompleted->setupTransfers(((MegaApplication *)qApp)->getFinishedTransfers(), QTransfersModel::TYPE_FINISHED);
    delete transferData;

    on_tAllTransfers_clicked();
    updateState();
    createAddMenu();    
}

TransferManager::~TransferManager()
{
    delete delegateListener;
    delete ui;
}

void TransferManager::onTransferStart(MegaApi *api, MegaTransfer *transfer)
{
    ui->wUploads->getModel()->onTransferStart(api, transfer);
    ui->wDownloads->getModel()->onTransferStart(api, transfer);
    ui->wAllTransfers->getModel()->onTransferStart(api, transfer);
}

void TransferManager::onTransferFinish(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    ui->wUploads->getModel()->onTransferFinish(api, transfer, e);
    ui->wDownloads->getModel()->onTransferFinish(api, transfer, e);
    ui->wAllTransfers->getModel()->onTransferFinish(api, transfer, e);
    ui->wCompleted->getModel()->onTransferFinish(api, transfer, e);
}

void TransferManager::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    ui->wUploads->getModel()->onTransferUpdate(api, transfer);
    ui->wDownloads->getModel()->onTransferUpdate(api, transfer);
    ui->wAllTransfers->getModel()->onTransferUpdate(api, transfer);
}

void TransferManager::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError *e)
{
    ui->wUploads->getModel()->onTransferTemporaryError(api, transfer, e);
    ui->wDownloads->getModel()->onTransferTemporaryError(api, transfer, e);
    ui->wAllTransfers->getModel()->onTransferTemporaryError(api, transfer, e);
}

void TransferManager::createAddMenu()
{
    if (!addMenu)
    {
        addMenu = new QMenu(this);
        addMenu->setStyleSheet(QString::fromAscii(
                                   "QMenu {background: #ffffff;}"
                                   "QMenu::item {height: 24px; font-family: Source Sans Pro; margin-left: 8px; margin-right: 14px; border-left: 10px solid transparent; color: #777777; padding: 6px 16px 8px 36px;}"
                                   "QMenu::separator {height: 1px; margin: 6px 0px 6px 0px; background-color: rgba(0, 0, 0, 0.1);}"
                                   "QMenu::item:selected {height: 24px; background: #aaaaaa; border-left: 10px solid #aaaaaa; color:#ffffff; }"));
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

    importLinksAction = new QAction(tr("Import links"), this);
    importLinksAction->setIcon(QIcon(QString::fromAscii("://images/get_link_ico.png")));
    connect(importLinksAction, SIGNAL(triggered()), qApp, SLOT(importLinks()));

    if (uploadAction)
    {
        uploadAction->deleteLater();
        uploadAction = NULL;
    }

    uploadAction = new QAction(tr("Upload to MEGA"), this);
    uploadAction->setIcon(QIcon(QString::fromAscii("://images/upload_to_mega_ico.png")));
    connect(uploadAction, SIGNAL(triggered()), qApp, SLOT(uploadActionClicked()));

    if (downloadAction)
    {
        downloadAction->deleteLater();
        downloadAction = NULL;
    }

    downloadAction = new QAction(tr("Download from MEGA"), this);
    downloadAction->setIcon(QIcon(QString::fromAscii("://images/download_from_mega_ico.png")));
    connect(downloadAction, SIGNAL(triggered()), qApp, SLOT(downloadActionClicked()));

    if (settingsAction)
    {
        settingsAction->deleteLater();
        settingsAction = NULL;
    }

#ifndef __APPLE__
    settingsAction = new QAction(tr("Settings"), this);
#else
    settingsAction = new QAction(tr("Preferences"), this);
#endif
    settingsAction->setIcon(QIcon(QString::fromAscii("://images/settings_ico.png")));
    connect(settingsAction, SIGNAL(triggered()), qApp, SLOT(openSettings()));

    addMenu->addAction(importLinksAction);
    addMenu->addAction(uploadAction);
    addMenu->addAction(downloadAction);
    addMenu->addSeparator();
    addMenu->addAction(settingsAction);
}

void TransferManager::on_tCompleted_clicked()
{
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->bClearAll->setText(tr("Clear all"));
    ui->bPause->setVisible(false);
    ui->wTransfers->setCurrentWidget(ui->wCompleted);
}

void TransferManager::on_tDownloads_clicked()
{
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->bClearAll->setText(tr("Cancel all"));
    ui->bPause->setVisible(true);

    ui->wTransfers->setCurrentWidget(ui->wDownloads);
    updateState();
}

void TransferManager::on_tUploads_clicked()
{
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
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
}

void TransferManager::on_tAllTransfers_clicked()
{
    ui->lAll->setStyleSheet(QString::fromUtf8("background-color : #ff333a;"));
    ui->lCompleted->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lUploads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));
    ui->lDownloads->setStyleSheet(QString::fromUtf8("background-color : transparent;"));

    ui->tAllTransfers->setStyleSheet(QString::fromUtf8("color: #333333;"));
    ui->tCompleted->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tUploads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->tDownloads->setStyleSheet(QString::fromUtf8("color: #999999;"));
    ui->bClearAll->setText(tr("Cancel all"));
    ui->bPause->setVisible(true);

    ui->wTransfers->setCurrentWidget(ui->wAllTransfers);
    updateState();
}

void TransferManager::on_bAdd_clicked()
{
    QPoint point = ui->bAdd->mapToGlobal(QPoint(ui->bAdd->width() , ui->bAdd->height() + 4));
    QPoint p = &point ? (point) - QPoint(addMenu->sizeHint().width(), 0) : QCursor::pos();

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

void TransferManager::updateState()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (w == ui->wAllTransfers)
    {
        ui->wAllTransfers->pausedTransfers(preferences->wasPaused());
        preferences->wasPaused() ? ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")))
                                 : ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
    }
    else if(w == ui->wDownloads)
    {
        ui->wDownloads->pausedTransfers(preferences->wasDownloadsPaused());
        preferences->wasDownloadsPaused() ? ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")))
                                          : ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
    }
    else if(w == ui->wUploads)
    {
        ui->wUploads->pausedTransfers(preferences->wasUploadsPaused());
        preferences->wasUploadsPaused() ? ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")))
                                        : ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")));
    }

}

void TransferManager::on_bPause_clicked()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (w == ui->wAllTransfers)
    {
        megaApi->pauseTransfers(!preferences->wasPaused());
    }
    else if(w == ui->wDownloads)
    {
        megaApi->pauseTransfers(!preferences->wasDownloadsPaused(), MegaTransfer::TYPE_DOWNLOAD);
    }
    else if(w == ui->wUploads)
    {
        megaApi->pauseTransfers(!preferences->wasUploadsPaused(), MegaTransfer::TYPE_UPLOAD);
    }
}

void TransferManager::on_bClearAll_clicked()
{
    QWidget *w = ui->wTransfers->currentWidget();
    if (((TransfersWidget *)w)->getModel()->rowCount(QModelIndex()) == 0)
    {
        return;
    }

    if (w != ui->wCompleted)
    {
        if(QMessageBox::question(this,
                                 QString::fromUtf8("MEGAsync"),
                                 tr("Are you sure you want to cancel all transfers?"),
                                 QMessageBox::Yes | QMessageBox::No, QMessageBox::No) == QMessageBox::No)
        {
            return;
        }

    }

    if (w == ui->wAllTransfers)
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
}

void TransferManager::changeEvent(QEvent *event)
{
    if (event->type() == QEvent::LanguageChange)
    {
        ui->retranslateUi(this);
    }
    QDialog::changeEvent(event);
}
