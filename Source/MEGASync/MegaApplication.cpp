#include "MegaApplication.h"
#include <QClipboard>

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

    createActions();
    createTrayIcon();
    infoDialog = new InfoDialog(this);
    setupWizard = NULL;
    settingsDialog = NULL;
    localServer = NULL;
    httpServer = NULL;
    transfer = NULL;
	storageMax = 0;
    error = NULL;
	invalidCredentials = false;
	updateInfo = false;
    queuedDownloads = 0;
    queuedUploads = 0;
    preferences = new Preferences();
    megaApi = new MegaApi(this,
         &(QCoreApplication::applicationDirPath()+"/").toStdString());

#ifdef WIN32
	WindowsUtils::initialize();
	WindowsUtils::startOnStartup(preferences->startOnStartup());
#endif

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
	else
	{
		megaApi->login(preferences->email().toUtf8().constData(),
					   preferences->password().toUtf8().constData());
	}

    trayIcon->show();
    trayIcon->showMessage(tr("MegaSync"), tr("MEGA Sync is running"));

#ifdef WIN32
	//if(!preferences->isTrayIconEnabled())
    {
        preferences->setTrayIconEnabled(true);
        if(!WindowsUtils::enableIcon(QFileInfo( QCoreApplication::applicationFilePath()).fileName()))
            cout << "Error enabling trayicon" << endl;
        else
            cout << "OK enabling trayicon" << endl;
    }
#endif

	//QString s(tr("MEGA Sync is paused"));

	if(localServer) delete localServer;
    localServer = new QLocalServer();
    connect(localServer, SIGNAL(newConnection()), this, SLOT(onNewLocalConnection()));
    localServer->listen("MegaLocalServer");
    cout << "Server socket started" << endl;

    if(httpServer) delete httpServer;
	httpServer = new HTTPServer(2973, NULL);
}

void MegaApplication::startSyncs()
{
	if(megaApi->getActiveSyncs()->size() != 0) stopSyncs();
	for(int i=0; i<preferences->getNumSyncedFolders(); i++)
	{
		cout << "Sync " << i << " added." << endl;
		megaApi->syncFolder(preferences->getLocalFolder(i).toUtf8().constData(),
					megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i)));
	}
}

void MegaApplication::stopSyncs()
{
	int i = 0;
	sync_list *syncs = megaApi->getActiveSyncs();
	for (sync_list::iterator it = syncs->begin(); it != syncs->end(); it++, i++)
	{
		delete *it;
		cout << "Sync " << i << " removed." << endl;
	}
}

void MegaApplication::reloadSyncs()
{
	stopSyncs();
	startSyncs();
}

void MegaApplication::unlink()
{
    preferences->clearAll();
    delete localServer;
    localServer = NULL;
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
	//static int i=0;
	//i++;

    QLocalSocket * clientConnection = (QLocalSocket *)QObject::sender();
    if (clientConnection->bytesAvailable() < (int)sizeof(quint16))
        return;

    QDataStream stream(clientConnection);
    stream.setVersion(QDataStream::Qt_4_8);
    QString path;
    stream >> path;

	//string s = path.toAscii().constData();
	//cout << "Local message received: " << s << endl;

	/*int numFolders = preferences->getNumSyncedFolders();
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
	}*/

	sync_list * syncs = megaApi->getActiveSyncs();
	int i=0;
	for (sync_list::iterator it = syncs->begin(); it != syncs->end(); it++, i++)
	{
		if(preferences->getNumSyncedFolders()<=i) break;
		QString basePath = preferences->getLocalFolder(i);
		if(path.startsWith(basePath))
		{
			if(!path.compare(basePath))
			{
				cout << "Base path found" << endl;
				stream << QString("0");
				clientConnection->flush();
				return;
			}
			QString relativePath = path.mid(basePath.length()+1);
			wchar_t windowsPath[512];
			int len = relativePath.toWCharArray(windowsPath);
			string str;
			str.append((char *)windowsPath, len*sizeof(wchar_t));
			wprintf(L"Checking: %s\n", str.data());
			pathstate_t state = (*it)->pathstate(&str);
			switch(state)
			{
				case PATHSTATE_SYNCED:
					stream << QString("0");
					break;
				case PATHSTATE_NOTFOUND:
					cout << "STATE NOT FOUND FOR FILE: " << relativePath.toStdString() << endl;
				default:
					stream << QString("1");
					break;
			}
			clientConnection->flush();
			return;
		}
	}

    stream << QString("2");
    clientConnection->flush();
}

void MegaApplication::pauseSync()
{
	stopSyncs();
	trayIcon->setIcon(QIcon("://images/tray_pause.ico"));
	trayMenu->removeAction(pauseAction);
    trayMenu->insertAction(settingsAction, resumeAction);
}

void MegaApplication::resumeSync()
{
	startSyncs();
	trayIcon->setIcon(QIcon("://images/SyncApp_1.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

void MegaApplication::updateDowloaded()
{
    //cout << "Downloaded" << endl;
    //QString s(downloader->downloadedData());
	//cout << s.toStdString() << endl;
}

void MegaApplication::copyFileLink(handle fileHandle)
{
	megaApi->exportNode(megaApi->getNodeByHandle(fileHandle));
}

void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
		if(infoDialog->isVisible())
		{
			infoDialog->hide();
			return;
		}
        infoDialog->updateDialog();
        infoDialog->show();
		//infoDialog->startAnimation();
    }
}

void MegaApplication::openSettings()
{
	if(settingsDialog)
	{
		if(settingsDialog->isVisible())
		{
			settingsDialog->activateWindow();
			return;
		}
		settingsDialog->show();
		return;
	}
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

	trayIcon = new QSystemTrayIcon(QIcon("://images/SyncApp_1.ico"));
    trayIcon->setContextMenu(trayMenu);
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
}

void MegaApplication::onRequestStart(MegaApi* api, MegaRequest *request)
{

}

void MegaApplication::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{
    switch (request->getType()) {
	case MegaRequest::TYPE_EXPORT:
	{
		if(e->getErrorCode() == MegaError::API_OK)
		{
			linkForClipboard = QString(request->getLink());
			QApplication::postEvent(this, new QEvent(QEvent::User));
		}
		break;
	}
	case MegaRequest::TYPE_LOGIN:
	{
		if(preferences->isSetupWizardCompleted())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
				megaApi->fetchNodes();
				megaApi->getAccountDetails();
			}
			else
			{
				invalidCredentials = true;
				QApplication::postEvent(this, new QEvent(QEvent::User));
			}
		}
		break;
	}
	case MegaRequest::TYPE_FETCH_NODES:
	{
		if(preferences->isSetupWizardCompleted())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
				startSyncs();
				return;
			}
			cout << "Error fetching nodes" << endl;
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
		if(e->getErrorCode() != MegaError::API_OK)
			break;

        AccountDetails *details = request->getAccountDetails();
        preferences->setAccountType(details->pro_level);
        preferences->setTotalStorage(details->storage_max);
        preferences->setUsedStorage(details->storage_used);

        storageMax = details->storage_max;
        storageUsed = details->storage_used;
        QApplication::postEvent(this, new QEvent(QEvent::User));
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
		if((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) &&
				(transfer->getTransferredBytes() == transfer->getTotalBytes()))
		{
			infoDialog->addRecentFile(QString(transfer->getFileName()), transfer->getNodeHandle());
			cout << "Download finished" << endl;
		}
        infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName()),
                            transfer->getTransferredBytes(), transfer->getTotalBytes());
		updateInfo = true;
		delete transfer;
        transfer = NULL;
    }

    if(storageMax)
    {
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

	if(invalidCredentials)
	{
		unlink();
		invalidCredentials = false;
	}

	if(updateInfo)
	{
		updateInfo = false;
		infoDialog->updateDialog();
	}

	if(linkForClipboard.length())
	{
		QClipboard *clipboard = QApplication::clipboard();
		clipboard->setText(linkForClipboard);
		trayIcon->showMessage(tr("MegaSync"), tr("The link has been copied to the clipboard"));
		linkForClipboard.clear();
	}

    return true;
}

void MegaApplication::onRequestTemporaryError(MegaApi *api, MegaRequest *request, MegaError* e)
{

}

void MegaApplication::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* e)
{
    this->transfer = transfer->copy();
    this->error = e->copy();
	if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD) queuedDownloads--;
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
	if(!nodes) return;

	for(int i=0; i<nodes->size(); i++)
	{
		Node *node = nodes->get(i);
		if(node->type==FILENODE && !node->removed && node->tag && !node->syncdeleted)
		{
			cout << "Adding recent upload from nodes_update: " << node->displayname() << "   tag: " <<
					node->tag << endl;
			infoDialog->addRecentFile(QString(node->displayname()), node->nodehandle);
			queuedUploads--;
		}
	}
	updateInfo = true;
	QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onReloadNeeded(MegaApi* api)
{
	stopSyncs();
	megaApi->fetchNodes();
}

void MegaApplication::onSyncStateChanged(Sync *, syncstate state)
{
    syncState = state;
    cout << "New STATE: " << state << endl;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncRemoteCopy(Sync *, const char *name)
{
	cout << "Added upload - remote copy" << endl;
    queuedUploads++;
    transfer = new MegaTransfer(MegaTransfer::TYPE_UPLOAD);
    transfer->setTotalBytes(1000);
    transfer->setTransferredBytes(1000);
    transfer->setPath(name);
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncGet(Sync *, const char *)
{
	cout << "Added download - sync get" << endl;
    queuedDownloads++;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncPut(Sync *, const char *)
{
	cout << "Added upload - sync put" << endl;
    queuedUploads++;
    QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::showSyncedIcon()
{
	trayIcon->setIcon(QIcon("://images/SyncApp_1.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

void MegaApplication::showSyncingIcon()
{
	trayIcon->setIcon(QIcon("://images/tray_sync.ico"));
    trayMenu->removeAction(resumeAction);
    trayMenu->insertAction(settingsAction, pauseAction);
}

