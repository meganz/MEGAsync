#include "TransferManager.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "ui_TransferManager.h"

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

    addMenu = NULL;
    importLinksAction = NULL;
    uploadAction = NULL;
    downloadAction = NULL;
    settingsAction = NULL;
    this->megaApi = megaApi;

    ui->wUploads->setupTransfers(megaApi->getTransfers(QTransfersModel::TYPE_UPLOAD), QTransfersModel::TYPE_UPLOAD);
    ui->wDownloads->setupTransfers(megaApi->getTransfers(QTransfersModel::TYPE_DOWNLOAD), QTransfersModel::TYPE_DOWNLOAD);
    ui->wAllTransfers->setupTransfers(megaApi->getTransfers(), QTransfersModel::TYPE_ALL);
    ui->wCompleted->setupTransfers(NULL, QTransfersModel::TYPE_FINISHED);

    on_tAllTransfers_clicked();
    createAddMenu();
}

TransferManager::~TransferManager()
{
    delete ui;
}

void TransferManager::createAddMenu()
{
    if (!addMenu)
    {
        addMenu = new QMenu(this);
        addMenu->setStyleSheet(QString::fromAscii(
                                   "QMenu {background: #ffffff; margin: 15px;}"
                                   "QMenu::item {font-family: Source Sans Pro; border-left: 10px solid transparent; color: #777777;padding: 5px 16px;padding-left: 30px;} "
                                   "QMenu::item:selected {background: #aaaaaa; border-left: 10px solid #aaaaaa; color:#ffffff;padding: 5px 16px;padding-left: 30px;}"));
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

    ui->wTransfers->setCurrentWidget(ui->wDownloads);
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

    ui->wTransfers->setCurrentWidget(ui->wUploads);
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

    ui->wTransfers->setCurrentWidget(ui->wAllTransfers);
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

void TransferManager::on_bPause_clicked()
{
    Preferences *preferences = Preferences::instance();
    preferences->wasPaused() ? ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/pause_ico.png")))
                             : ui->bPause->setIcon(QIcon(QString::fromUtf8(":/images/play_ico.png")));

    ((MegaApplication *)qApp)->pauseTransfers(!preferences->wasPaused());
}

void TransferManager::on_bClearAll_clicked()
{
    megaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    megaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}
