#include "MegaApplication.h"
#include "gui/PasteMegaLinksDialog.h"
#include "gui/ImportMegaLinksDialog.h"

#include <QClipboard>
#include <QDesktopWidget>

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
	QApplication::setStyleSheet("QToolTip { color: #fff; background-color: #151412; border: none; }");
    setOrganizationName("Mega Limited");
    setOrganizationDomain("mega.co.nz");
	setApplicationName("MEGAsync");
	QDir::setCurrent(QCoreApplication::applicationDirPath());
	qRegisterMetaType<QQueue<QString> >("QQueueQString");
	qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    createActions();
    createTrayIcon();

	delegateListener = new QTMegaListener(this);
    infoDialog = new InfoDialog(this);
    setupWizard = NULL;
    settingsDialog = NULL;
	uploadFolderSelector = NULL;
    localServer = NULL;
    httpServer = NULL;
	queuedDownloads = 0;
    queuedUploads = 0;
	totalUploads = totalDownloads = 0;
	totalDownloadSize = totalUploadSize = 0;
	totalDownloadedSize = totalUploadedSize = 0;

	uploadSpeed = downloadSpeed = 0;
    preferences = new Preferences();
	QString basePath = QCoreApplication::applicationDirPath()+"/";
	string tmpPath = basePath.toStdString();
	megaApi = new MegaApi(delegateListener, &tmpPath);

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

	ShellDispatcher *shellDispatcher = new ShellDispatcher();
	connect(shellDispatcher, SIGNAL(newUploadQueue(QQueue<QString>)), this, SLOT(uploadFiles(QQueue<QString>)));
	shellDispatcher->start();
    trayIcon->show();
	trayIcon->showMessage(tr("MEGAsync"), tr("MEGAsync is running"));

#ifdef WIN32
	if(preferences->isTrayIconEnabled())
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
	sync_list *syncs = megaApi->getActiveSyncs();
	sync_list::iterator it = syncs->begin();
	while(it != syncs->end())
		delete *it++;
}

void MegaApplication::processUploadQueue(handle nodeHandle)
{
	cout << "processUploadQueue " <<  uploadQueue.size() << endl;
	Node *node = megaApi->getNodeByHandle(nodeHandle);
	if(!node || node->type==FILENODE)
	{
		uploadQueue.clear();
		trayIcon->showMessage(tr("MEGAsync"), tr("Error: Invalid destination folder. The upload has been cancelled"));
		return;
	}

	while(!uploadQueue.isEmpty())
	{
		QString filePath = uploadQueue.dequeue();
		if(!QFileInfo(filePath).isFile()) continue;
		filePath = QDir::toNativeSeparators(filePath);
		cout << "Starting upload for " << filePath.toStdString() << endl;
		megaApi->startUpload(filePath.toUtf8().constData(), node);
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

void MegaApplication::importLinks()
{
	PasteMegaLinksDialog dialog;
	dialog.exec();
	if(dialog.result()!=QDialog::Accepted) return;
	QStringList linkList = dialog.getLinks();
	LinkProcessor *linkProcessor = new LinkProcessor(megaApi, linkList);
	ImportMegaLinksDialog importDialog(megaApi, preferences, linkProcessor);
	importDialog.exec();
	if(importDialog.result()!=QDialog::Accepted) return;

	if(importDialog.shouldDownload())
	{
		preferences->setDownloadFolder(importDialog.getDownloadPath());
		linkProcessor->downloadLinks(importDialog.getDownloadPath());
	}

	if(importDialog.shouldImport())
	{
		connect(linkProcessor, SIGNAL(onLinkImportFinish()), this, SLOT(onLinkImportFinished()));
		linkProcessor->importLinks(importDialog.getImportPath());
	}
	else delete linkProcessor;
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

void MegaApplication::uploadFiles(QQueue<QString> newUploadQueue)
{
	uploadQueue.append(newUploadQueue);

	if(uploadFolderSelector) return;
	Node *node = megaApi->getNodeByHandle(preferences->uploadFolder());
	if(node)
	{
		processUploadQueue(node->nodehandle);
		return;
	}

	uploadFolderSelector = new UploadToMegaDialog(megaApi);
	QTimer::singleShot(1000, this, SLOT(showUploadDialog()));
	return;
}

void MegaApplication::showUploadDialog()
{
	uploadFolderSelector->exec();
	if(uploadFolderSelector->result()==QDialog::Accepted)
	{
		handle nodeHandle = uploadFolderSelector->getSelectedHandle();
		processUploadQueue(nodeHandle);
		if(uploadFolderSelector->isDefaultFolder())
			preferences->setUploadFolder(nodeHandle);
	}
	else uploadQueue.clear();

	delete uploadFolderSelector;
	uploadFolderSelector = NULL;
}

void MegaApplication::onLinkImportFinished()
{
	LinkProcessor *linkProcessor = ((LinkProcessor *)QObject::sender());
	preferences->setImportFolder(linkProcessor->getImportParentFolder());
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
		QRect screenGeometry = QApplication::desktop()->availableGeometry();
		infoDialog->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 500 - 2);
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
	importLinksAction = new QAction(tr("Import links"), this);
	connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));
}

void MegaApplication::createTrayIcon()
{
    trayMenu = new QMenu();
    trayMenu->addAction(pauseAction);
	trayMenu->addAction(importLinksAction);
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
			QString linkForClipboard(request->getLink());
			QClipboard *clipboard = QApplication::clipboard();
			clipboard->setText(linkForClipboard);
			trayIcon->showMessage(tr("MEGAsync"), tr("The link has been copied to the clipboard"));
			linkForClipboard.clear();
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
				unlink();
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
		preferences->setTotalBandwidth(details->transfer_max);
		preferences->setUsedBandwidth(details->transfer_own_used);
		infoDialog->setUsage(details->storage_max, details->storage_used);
        break;
    }
    default:
        break;
    }
}

void MegaApplication::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
		downloadSpeed = 0;
		queuedDownloads++;
		totalDownloads++;
		totalDownloadSize += transfer->getTotalBytes();
	}
	else
	{
		uploadSpeed = 0;
		queuedUploads++;
		totalUploads++;
		totalUploadSize += transfer->getTotalBytes();
	}

	infoDialog->setTransferCount(totalDownloads, totalUploads, queuedDownloads, queuedUploads);
	infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);
	this->showSyncingIcon();
}

void MegaApplication::onRequestTemporaryError(MegaApi *, MegaRequest *request, MegaError* e)
{

}

void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{	
	if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
	{
		queuedDownloads--;
		totalDownloadedSize += transfer->getDeltaSize();
		downloadSpeed = transfer->getSpeed();
		infoDialog->addRecentFile(QString(transfer->getFileName()), transfer->getNodeHandle(), transfer->getPath());
	}
	else
	{
		queuedUploads--;
		totalUploadedSize += transfer->getDeltaSize();
		uploadSpeed = transfer->getSpeed();
		uploadLocalPaths[transfer->getTag()]=transfer->getPath();
	}

	infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
	infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
	infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName())
							,transfer->getTransferredBytes(), transfer->getTotalBytes());
	infoDialog->setTransferCount(totalDownloads, totalUploads, queuedDownloads, queuedUploads);
	infoDialog->updateDialog();

	if(queuedDownloads || queuedUploads)
	{
		this->showSyncingIcon();
	}
	else
	{
		totalUploads = totalDownloads = 0;
		totalUploadSize = totalDownloadSize = 0;
		totalUploadedSize = totalDownloadedSize = 0;
		uploadSpeed = downloadSpeed = 0;
		this->showSyncedIcon();
	}
}

void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
	infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName()),
						transfer->getTransferredBytes(), transfer->getTotalBytes());

	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
		downloadSpeed = transfer->getSpeed();
		totalDownloadedSize += transfer->getDeltaSize();
	}
	else
	{
		uploadSpeed = transfer->getSpeed();
		totalUploadedSize += transfer->getDeltaSize();
	}

	infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
	infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);

	infoDialog->updateDialog();
}

void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{
	trayIcon->showMessage(transfer->getFileName(), tr("Temporarily error in transfer: ") + e->getErrorString());
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
		if(!node->removed && node->tag && !node->syncdeleted)
		{
			cout << "Adding recent upload from nodes_update: " << node->displayname() << "   tag: " <<
					node->tag << endl;
			QString localPath;


			if(node->localnode)
			{
				cout << "Sync upload" << endl;
				string localseparator;
				localseparator.assign((char*)L"\\",sizeof(wchar_t));
				string path;
				LocalNode* l = node->localnode;
				while (l)
				{
					path.insert(0,l->localname);
					if ((l = l->parent)) path.insert(0, localseparator);
				}
				path.append("", 1);
				localPath = QString::fromWCharArray((const wchar_t *)path.data());
				cout << "Sync path: " << localPath.toStdString() << endl;
			}
			else if(uploadLocalPaths.contains(node->tag))
			{
				localPath = uploadLocalPaths.value(node->tag);
				uploadLocalPaths.remove(node->tag);
				cout << "Local upload: " << localPath.toStdString() << endl;
			}
			else
			{
				cout << "LOCAL PATH NOT FOUND" << endl;
			}

			if(!localPath.isNull()) WindowsUtils::notifyItemChange(localPath);
            if(node->type == FILENODE) infoDialog->addRecentFile(QString::fromUtf8(node->displayname()), node->nodehandle, localPath);
		}
	}
	infoDialog->updateDialog();
}

void MegaApplication::onReloadNeeded(MegaApi* api)
{
	stopSyncs();
	megaApi->fetchNodes();
}

/*
void MegaApplication::onSyncStateChanged(Sync *, syncstate state)
{
	//syncState = state;
	//cout << "New STATE: " << state << endl;
	//QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncRemoteCopy(Sync *, const char *name)
{
	//cout << "Added upload - remote copy" << endl;
	//transfer = new MegaTransfer(MegaTransfer::TYPE_UPLOAD);
	//transfer->setTotalBytes(1000);
	//transfer->setTransferredBytes(1000);
	//transfer->setPath(name);
	//QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncGet(Sync *, const char *)
{
	//cout << "Added download - sync get" << endl;
	//QApplication::postEvent(this, new QEvent(QEvent::User));
}

void MegaApplication::onSyncPut(Sync *, const char *)
{
	//cout << "Added upload - sync put" << endl;
	//QApplication::postEvent(this, new QEvent(QEvent::User));
}
*/

void MegaApplication::showSyncedIcon()
{
	trayIcon->setIcon(QIcon("://images/SyncApp_1.ico"));
	trayMenu->removeAction(resumeAction);
	trayMenu->insertAction(importLinksAction, pauseAction);
}

void MegaApplication::showSyncingIcon()
{
	trayIcon->setIcon(QIcon("://images/tray_sync.ico"));
    trayMenu->removeAction(resumeAction);
	trayMenu->insertAction(importLinksAction, pauseAction);
}

