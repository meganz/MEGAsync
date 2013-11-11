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
    WindowsUtils::startOnStartup(false);
#endif

    megaApi = new MegaApi(this);

    createActions();
    createTrayIcon();
    infoDialog = new InfoDialog(this);
    setupWizard = NULL;
    settingsDialog = NULL;
    localServer = NULL;
    preferences = new Preferences();

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

    localServer = new QLocalServer();
    connect(localServer, SIGNAL(newConnection()), this, SLOT(onNewLocalConnection()));
    localServer->listen("MegaLocalServer");
    cout << "Server socket started" << endl;

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
        infoDialog->show();
        infoDialog->startAnimation();
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
        cout << "Details end" << endl;
        break;
    }
    default:
        break;
    }
}

void MegaApplication::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e)
{

}

void MegaApplication::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e)
{

}

void MegaApplication::onTransferUpdate(MegaApi *api, MegaTransfer *transfer)
{

}

void MegaApplication::onTransferTemporaryError(MegaApi *api, MegaTransfer *transfer, MegaError* e)
{

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

