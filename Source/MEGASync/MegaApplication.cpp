#include "MegaApplication.h"

int main(int argc, char *argv[])
{
    MegaApplication app(argc, argv);
    return app.exec();
}


#ifdef WIN32
    #include "WindowsUtils.h"
#endif

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    setOrganizationName("Mega Limited");
    setOrganizationDomain("mega.co.nz");
    setApplicationName("MegaSync");

#ifdef WIN32
    WindowsUtils::initialize();
    WindowsUtils::startOnStartup(false);
#endif



    createActions();
    createTrayIcon();
    infoDialog = new InfoDialog(this);
    setupWizard = NULL;
    settingsDialog = NULL;
    localServer = NULL;
    httpServer = NULL;
    transfer = NULL;
    storageMax = NULL;
    error = NULL;
    queuedDownloads = 0;
    queuedUploads = 0;
    preferences = new Preferences();
    megaApi = new MegaApi(this,
         &(QCoreApplication::applicationDirPath()+"/").toStdString());

    //downloader = new FileDownloader(QUrl("http://www.google.es"));
    //connect(downloader, SIGNAL(downloaded()), this, SLOT(updateDowloaded()));
    init();
}

void MegaApplication::init()
{
    if(!preferences->isSetupWizardCompleted())
    {
        setupWizard = new SetupWizard(this);
        setupWizard->exec();
        if(!preferences->isSetupWizardCompleted())
            ::exit(0);
    }

    trayIcon->show();
    trayIcon->showMessage(tr("MegaSync"), tr("MEGA Sync is running"));

#ifdef WIN32
    if(!preferences->isTrayIconEnabled())
    {
        preferences->setTrayIconEnabled(true);
        if(!WindowsUtils::enableIcon(QFileInfo( QCoreApplication::applicationFilePath()).fileName()))
            cout << "Error enabling trayicon" << endl;
        else
            cout << "OK enabling trayicon" << endl;
    }
#endif

    QString s(tr("MEGA Sync is paused"));

    if(localServer)
    {
        localServer->close();
        delete localServer;
    }
    localServer = new QLocalServer();
    connect(localServer, SIGNAL(newConnection()), this, SLOT(onNewLocalConnection()));
    localServer->listen("MegaLocalServer");
    cout << "Server socket started" << endl;

    if(httpServer)
    {
        httpServer->close();
        delete httpServer;
    }
    httpServer = new HTTPServer(12346, NULL);
}

void MegaApplication::unlink()
{
    preferences->clearAll();
    localServer->close();
    delete localServer;
    localServer = NULL;

    httpServer->close();
    delete httpServer;
    httpServer = NULL;

    trayIcon->hide();
    init();
}

void MegaApplication::onNewLocalConnection()
{
    QLocalSocket *clientConnection = localServer->nextPendingConnection();
    connect(clientConnection, SIGNAL(disconnected()),
            clientConnection, SLOT(deleteLater()));

    connect(clientConnection, SIGNAL(readyRead()),
            this,             SLOT(onDataReady()));
    cout << "Local connection received!" << endl;
}

void MegaApplication::onDataReady()
{
    static int i=0;
    i++;

    QLocalSocket * clientConnection = (QLocalSocket *)QObject::sender();
    if (clientConnection->bytesAvailable() < (int)sizeof(quint16))
        return;

    QDataStream stream(clientConnection);
    stream.setVersion(QDataStream::Qt_4_8);
    QString path;
    stream >> path;

    string s = path.toAscii().constData();
    cout << "Local message received: " << s << endl;

    int numFolders = preferences->getNumSyncedFolders();
    for(int i=0; i<numFolders; i++)
    {
        QString localFolder = preferences->getLocalFolder(i);
        if(path.startsWith(localFolder))
        {
            uint hash = qHash(path);
            cout << "Hash: " << hash << endl;
            if(hash%3) stream << QString("0");
            else stream << QString("1");
            clientConnection->flush();
            return;
        }
    }
    stream << QString("2");
    clientConnection->flush();
}

void MegaApplication::pauseSync()
{
    trayIcon->setIcon(QIcon(":/images/tray_pause.ico"));
    trayMenu->removeAction(pauseAction);
    trayMenu->insertAction(settingsAction, resumeAction);
}

void MegaApplication::resumeSync()
{
    trayIcon->setIcon(QIcon(":/images/SyncApp_1.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

void MegaApplication::updateDowloaded()
{
    //cout << "Downloaded" << endl;
    //QString s(downloader->downloadedData());
    //cout << s.toStdString() << endl;
}

void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        infoDialog->updateDialog();
        infoDialog->show();
        //infoDialog->startAnimation();
    }
}

void MegaApplication::openSettings()
{
    if(settingsDialog) delete settingsDialog;
    settingsDialog = new SettingsDialog(this);
    settingsDialog->show();
}

void MegaApplication::createActions()
{
    exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(quit()));
    settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    pauseAction = new QAction(tr("Pause synchronization"), this);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseSync()));
    resumeAction = new QAction(tr("Resume synchronization"), this);
    connect(resumeAction, SIGNAL(triggered()), this, SLOT(resumeSync()));
}

void MegaApplication::createTrayIcon()
{
    trayMenu = new QMenu();
    trayMenu->addAction(pauseAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addAction(exitAction);

    trayIcon = new QSystemTrayIcon(QIcon(":/images/SyncApp_1.ico"));
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void MegaApplication::onRequestStart(MegaApi* api, MegaRequest *request)
{

}

void MegaApplication::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{
    if(e->getErrorCode() != MegaError::API_OK)
        return;

    switch (request->getType()) {
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        cout << "Details start" << endl;
        AccountDetails *details = request->getAccountDetails();
        preferences->setAccountType(details->pro_level);
        preferences->setTotalStorage(details->storage_max);
        preferences->setUsedStorage(details->storage_used);

        storageMax = details->storage_max;
        storageUsed = details->storage_used;
        QApplication::postEvent(this, new QEvent(QEvent::User));
        cout << "Details end" << endl;
        break;
    }
    default:
        break;
    }
}

bool MegaApplication::event(QEvent *event)
{
    if((event->type() != QEvent::User))
        return QApplication::event(event);

    if(transfer)
    {
        if(transfer->getTransferredBytes() == transfer->getTotalBytes())
        {
            if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) queuedDownloads--;
            else queuedUploads --;
        }

        infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName()),
                            transfer->getTransferredBytes(), transfer->getTotalBytes());
        delete transfer;
        transfer = NULL;
    }

    if(storageMax)
    {
        cout << "Setting usage" << endl;
        infoDialog->setUsage(storageMax/(1024*1024*1024),
                             100 * (double)storageUsed/storageMax);
    }

    if(error)
    {
        delete error;
        error = NULL;
    }

    infoDialog->setQueuedTransfers(queuedDownloads, queuedUploads);

    if(queuedDownloads || queuedUploads)
        this->showSyncingIcon();
    else
        this->showSyncedIcon();

    return true;
}

void MegaApplication::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e)
{

}

void MegaApplication::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e)
{
    this->transfer = transfer->copy();
    this->error = e->copy();
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{
    this->transfer = transfer->copy();
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{
    this->transfer = transfer->copy();
    this->error = e->copy();
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onUsersUpdate(MegaApi* api, UserList *users)
{

}

void MegaApplication::onNodesUpdate(MegaApi* api, NodeList *nodes)
{

}

void MegaApplication::onReloadNeeded(MegaApi* api)
{

}

void MegaApplication::onSyncStateChanged(Sync *, syncstate state)
{
    syncState = state;
    cout << "New STATE: " << state << endl;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncRemoteCopy(Sync *, const char *name)
{
    queuedUploads++;
    transfer = new MegaTransfer(MegaTransfer::TYPE_UPLOAD);
    transfer->setTotalBytes(1000);
    transfer->setTransferredBytes(1000);
    transfer->setPath(name);
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncGet(Sync *, const char *)
{
    queuedDownloads++;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncPut(Sync *, const char *)
{
    queuedUploads++;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::showSyncedIcon()
{
    trayIcon->setIcon(QIcon(":/images/SyncApp_1.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

void MegaApplication::showSyncingIcon()
{
    trayIcon->setIcon(QIcon(":/images/tray_sync.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

