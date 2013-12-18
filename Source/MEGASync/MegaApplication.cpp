#include "MegaApplication.h"
#include "gui/PasteMegaLinksDialog.h"
#include "gui/ImportMegaLinksDialog.h"
#include "utils/Utils.h"

#include <QClipboard>
#include <QDesktopWidget>

const int MegaApplication::VERSION_CODE = 101; //1.00

int main(int argc, char *argv[])
{
    MegaApplication app(argc, argv);
    return app.exec();
}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    //Hack to have tooltips with a black background
	QApplication::setStyleSheet("QToolTip { color: #fff; background-color: #151412; border: none; }");

    //Set QApplication fields
    setOrganizationName("Mega Limited");
    setOrganizationDomain("mega.co.nz");
	setApplicationName("MEGAsync");
    setApplicationVersion(QString::number(VERSION_CODE));

    //Set the working directory
    QDir::setCurrent(QCoreApplication::applicationDirPath());

    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
	qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    //Create GUI elements
    createActions();
    createTrayIcon();
    infoDialog = NULL;
    setupWizard = NULL;
    settingsDialog = NULL;
    uploadFolderSelector = NULL;
    preferences = new Preferences();

    //Initialize fields to manage communications and transfers
    delegateListener = new QTMegaListener(this);
    httpServer = NULL;
    queuedDownloads = 0;
    queuedUploads = 0;
    totalUploads = totalDownloads = 0;
    totalDownloadSize = totalUploadSize = 0;
    totalDownloadedSize = totalUploadedSize = 0;
    uploadSpeed = downloadSpeed = 0;
    QString basePath = QCoreApplication::applicationDirPath()+"/";
    string tmpPath = basePath.toStdString();
    megaApi = new MegaApi(delegateListener, &tmpPath);
    uploader = new MegaUploader(megaApi);
    reboot = false;

    //Apply the "Start on startup" configuration
    Utils::startOnStartup(preferences->startOnStartup());

    //Start the update task in the update thread
    if(preferences->updateAutomatically())
        startUpdateTask();

    //Start the app
    init();
}

void MegaApplication::init()
{
    //Start the initial setup wizard if needed
    if(!preferences->logged())
    {
        setupWizard = new SetupWizard(this);
		setupWizard->exec();
        if(!preferences->logged())
            ::exit(0);
        loggedIn();
    }
	else
	{
        //Otherwise, login in the account
        megaApi->fastLogin(preferences->email().toUtf8().constData(),
                       preferences->emailHash().toUtf8().constData(),
                       preferences->privatePw().toUtf8().constData());
	}
}

void MegaApplication::loggedIn()
{
    infoDialog = new InfoDialog(this);

    //Get account details
    megaApi->getAccountDetails();

    //Set the upload limit
    setUploadLimit(preferences->uploadLimitKB());

    //Start the Sync feature
    startSyncs();

    //Show the tray icon
    trayIcon->show();
    showNotificationMessage(tr("MEGAsync is running"));

    //Try to keep the tray icon always active
    if(!Utils::enableTrayIcon(QFileInfo( QCoreApplication::applicationFilePath()).fileName()))
        cout << "Error enabling trayicon" << endl;
    else
        cout << "OK enabling trayicon" << endl;

    Utils::startShellDispatcher(this);

    //Start the HTTP server
	httpServer = new HTTPServer(2973, NULL);
}

void MegaApplication::startSyncs()
{
    //Ensure that there is no active syncs
	if(megaApi->getActiveSyncs()->size() != 0) stopSyncs();

    //Start syncs
	for(int i=0; i<preferences->getNumSyncedFolders(); i++)
	{
		cout << "Sync " << i << " added." << endl;
		megaApi->syncFolder(preferences->getLocalFolder(i).toUtf8().constData(),
					megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i)));
	}
}

void MegaApplication::stopSyncs()
{
    //Stop syncs
	sync_list *syncs = megaApi->getActiveSyncs();
	sync_list::iterator it = syncs->begin();
	while(it != syncs->end())
		delete *it++;
}

//This function is called to upload all files in the uploadQueue field
//to the Mega node that is passed as parameter
void MegaApplication::processUploadQueue(handle nodeHandle)
{
	Node *node = megaApi->getNodeByHandle(nodeHandle);
    QStringList notUploaded;

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
	if(!node || node->type==FILENODE)
	{
		uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
		return;
	}

    //Process the upload queue using the MegaUploader object
	while(!uploadQueue.isEmpty())
	{
		QString filePath = uploadQueue.dequeue();
        if(!Utils::verifySyncedFolderLimits(filePath))
        {
            //If a folder can't be uploaded, save its name
            notUploaded.append(QFileInfo(filePath).fileName());
            continue;
        }
        uploader->upload(filePath, node);
    }

    //If any file or folder couldn't be uploaded, inform users
    if(notUploaded.size())
    {
        if(notUploaded.size()==1)
        {
            showInfoMessage(tr("The folder (%1) wasn't uploaded "
                "because it contains too much files or folders (+%2 folders or +%3 files)")
                .arg(notUploaded[0]).arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
                .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER));
        }
        else
        {
            showInfoMessage(tr("%1 folders weren't uploaded "
                "because they contain too much files or folders (+%2 folders or +%3 files)")
                .arg(notUploaded.size()).arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
                .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER));
        }
    }
}

void MegaApplication::rebootApplication()
{
    if(queuedDownloads || queuedUploads)
        return;

    stopUpdateTask();
    Utils::stopShellDispatcher();

    QString app = QApplication::applicationFilePath();
    QStringList arguments = QApplication::arguments();
    QString wd = QDir::currentPath();
    QProcess::startDetached(app, arguments, wd);
    QApplication::exit();
}

void MegaApplication::exitApplication()
{
    stopUpdateTask();
    Utils::stopShellDispatcher();
    QApplication::exit();
}

void MegaApplication::pauseTransfers(bool pause)
{
    megaApi->pauseTransfers(pause);
}

void MegaApplication::reloadSyncs()
{
	stopSyncs();
	startSyncs();
}

void MegaApplication::unlink()
{
    //Reset fields that will be initialized again upon login
    delete httpServer;
    httpServer = NULL;
    stopSyncs();
    megaApi->logout();
}  

void MegaApplication::showInfoMessage(QString message, QString title)
{
    if(trayIcon) trayIcon->showMessage(title, message, QSystemTrayIcon::Information);
    else QMessageBox::information(NULL, title, message);
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(trayIcon) trayIcon->showMessage(title, message, QSystemTrayIcon::Warning);
    else QMessageBox::warning(NULL, title, message);
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    QMessageBox::critical(NULL, title, message);
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(trayIcon) trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 8000);
}

//KB/s
void MegaApplication::setUploadLimit(int limit)
{
    if(limit<0) megaApi->setUploadLimit(-1);
    else megaApi->setUploadLimit(limit*1024);
}

void MegaApplication::startUpdateTask()
{
    if(!updateThread.isRunning())
    {
        updateTask.moveToThread(&updateThread);
        updateThread.start();
        connect(this, SIGNAL(startUpdaterThread()), &updateTask, SLOT(doWork()), Qt::UniqueConnection);
        connect(&updateTask, SIGNAL(updateCompleted()), this, SLOT(onUpdateCompleted()), Qt::UniqueConnection);
        emit startUpdaterThread();
    }
}

void MegaApplication::stopUpdateTask()
{
    if(updateThread.isRunning())
        updateThread.quit();
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

//Called when the "Import links" menu item is clicked
void MegaApplication::importLinks()
{
    //Show the dialog to paste public links
	PasteMegaLinksDialog dialog;
	dialog.exec();

    //If the dialog isn't accepted, return
	if(dialog.result()!=QDialog::Accepted) return;

    //Get the list of links from the dialog
    QStringList linkList = dialog.getLinks();

    //Send links to the link processor
	LinkProcessor *linkProcessor = new LinkProcessor(megaApi, linkList);

    //Open the import dialog
	ImportMegaLinksDialog importDialog(megaApi, preferences, linkProcessor);
	importDialog.exec();
	if(importDialog.result()!=QDialog::Accepted) return;

    //If the user wants to download some links, do it
	if(importDialog.shouldDownload())
	{
		preferences->setDownloadFolder(importDialog.getDownloadPath());
		linkProcessor->downloadLinks(importDialog.getDownloadPath());
	}

    //If the user wants to import some links, do it
	if(importDialog.shouldImport())
	{
		connect(linkProcessor, SIGNAL(onLinkImportFinish()), this, SLOT(onLinkImportFinished()));
		linkProcessor->importLinks(importDialog.getImportPath());
	}
    //If importing links isn't needed, we can delete the link processor
    //It doesn't track transfers, only the importation of links
	else delete linkProcessor;
}

//Called when the user wants to generate the public link for a node
void MegaApplication::copyFileLink(handle fileHandle)
{
    //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
	megaApi->exportNode(megaApi->getNodeByHandle(fileHandle));
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{
    //Append the list of files to the upload queue
	uploadQueue.append(newUploadQueue);

    //If the dialog to select the upload folder is active, return.
    //Files will be uploaded when the user selects the upload folder
	if(uploadFolderSelector) return;

    //If there is a default upload folder in the preferences
	Node *node = megaApi->getNodeByHandle(preferences->uploadFolder());
	if(node)
	{
        //use it to upload the list of files
		processUploadQueue(node->nodehandle);
		return;
	}

    //If there isn't a default upload folder, show the dialog
    //with a delay to make sure that the click in the context menu
    //of the shell has finished
	uploadFolderSelector = new UploadToMegaDialog(megaApi);
	QTimer::singleShot(1000, this, SLOT(showUploadDialog()));
	return;
}

void MegaApplication::showUploadDialog()
{
    //Show the dialog to select the upload folder
    uploadFolderSelector->activateWindow();
	uploadFolderSelector->exec();

	if(uploadFolderSelector->result()==QDialog::Accepted)
	{
        //If the dialog is accepted, get the destination node
		handle nodeHandle = uploadFolderSelector->getSelectedHandle();
		if(uploadFolderSelector->isDefaultFolder())
			preferences->setUploadFolder(nodeHandle);
        processUploadQueue(nodeHandle);
	}
    //If the dialog is rejected, cancel uploads
	else uploadQueue.clear();

    //Free the dialog
	delete uploadFolderSelector;
	uploadFolderSelector = NULL;
}

//Called when the link import finishes
void MegaApplication::onLinkImportFinished()
{
	LinkProcessor *linkProcessor = ((LinkProcessor *)QObject::sender());
	preferences->setImportFolder(linkProcessor->getImportParentFolder());
    linkProcessor->deleteLater();
}

void MegaApplication::onUpdateCompleted()
{
    cout << "Update completed. Initializing a silent reboot..." << endl;
    reboot = true;
    QTimer::singleShot(10000, this, SLOT(rebootApplication()));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        //If the information dialog is visible, hide it
		if(infoDialog->isVisible())
		{
			infoDialog->hide();
			return;
		}

        //If the information dialog isn't visible:
        //Update it
        infoDialog->updateDialog();

        //Put it in the right position (to prevent problems with changes in the taskbar or the resolution)
        QRect screenGeometry = QApplication::desktop()->availableGeometry();
		infoDialog->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 500 - 2);

        //Show the dialog
		infoDialog->show();
    }
}

//Called when the user wants to open the settings dialog
void MegaApplication::openSettings()
{
	if(settingsDialog)
    {
        //If the dialog is active
		if(settingsDialog->isVisible())
		{
            //and visible -> show it
			settingsDialog->activateWindow();
			return;
		}

        //Otherwise, delete it
        delete settingsDialog;
	}

    //Show a new settings dialog
    settingsDialog = new SettingsDialog(this);
	settingsDialog->show();
}

//This function creates the menu actions
void MegaApplication::createActions()
{
    exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));
    pauseAction = new QAction(tr("Pause synchronization"), this);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseSync()));
    resumeAction = new QAction(tr("Resume synchronization"), this);
    connect(resumeAction, SIGNAL(triggered()), this, SLOT(resumeSync()));
	importLinksAction = new QAction(tr("Import links"), this);
	connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));
}

//This function creates the tray icon
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

//Called when a request is about to start
void MegaApplication::onRequestStart(MegaApi* api, MegaRequest *request)
{

}

//Called when a request has finished
void MegaApplication::onRequestFinish(MegaApi* api, MegaRequest *request, MegaError* e)
{
    switch (request->getType()) {
	case MegaRequest::TYPE_EXPORT:
	{
		if(e->getErrorCode() == MegaError::API_OK)
		{
            //A public link has been created, put it in the clipboard and inform users
			QString linkForClipboard(request->getLink());
            QApplication::clipboard()->setText(linkForClipboard);
            showInfoMessage(tr("The link has been copied to the clipboard"));
		}
		break;
	}
	case MegaRequest::TYPE_LOGIN:
    case MegaRequest::TYPE_FAST_LOGIN:
	{
        //This prevents to handle logins in the initial setup wizard
        if(preferences->logged())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
                //Successful login, fetch nodes and account details
				megaApi->fetchNodes();
				megaApi->getAccountDetails();
                break;
			}

            //Wrong login -> logout
            unlink();
		}
		break;
	}
    case MegaRequest::TYPE_LOGOUT:
    {
        if(preferences->logged())
        {
            preferences->unlink();
            trayIcon->hide();
            delete infoDialog;
            init();
        }
    }
	case MegaRequest::TYPE_FETCH_NODES:
	{
        //This prevents to handle node requests in the initial setup wizard
        if(preferences->logged())
		{
			if(e->getErrorCode() == MegaError::API_OK)
			{
                //If we have got the filesystem, start the app
                loggedIn();
                break;
			}

            //Problem fetching nodes.
            //This shouldn't happen -> logout
            cout << "Error fetching nodes" << endl;
            unlink();
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
		if(e->getErrorCode() != MegaError::API_OK)
			break;

        //Account details retrieved, update the preferences and the information dialog
        AccountDetails *details = request->getAccountDetails();
        preferences->setAccountType(details->pro_level);
        preferences->setTotalStorage(details->storage_max);
        preferences->setUsedStorage(details->storage_used);
		preferences->setTotalBandwidth(details->transfer_max);
		preferences->setUsedBandwidth(details->transfer_own_used);
		infoDialog->setUsage(details->storage_max, details->storage_used);
        break;
    }
    case MegaRequest::TYPE_PAUSE_TRANSFERS:
    {
        infoDialog->setPaused(request->getFlag());
        if(request->getFlag()) trayIcon->setIcon(QIcon("://images/tray_pause.ico"));
        else if(queuedUploads || queuedDownloads) trayIcon->setIcon(QIcon("://images/tray_sync.ico"));
        else trayIcon->setIcon(QIcon("://images/SyncApp_1.ico"));
    }
    default:
        break;
    }
}

//Called when a transfer is about to start
void MegaApplication::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    //Update statics
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

    //Send statics to the information dialog
	infoDialog->setTransferCount(totalDownloads, totalUploads, queuedDownloads, queuedUploads);
	infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);

    //Update the state of the tray icon
    showSyncingIcon();
}

//Called when there is a temporal problem in a request
void MegaApplication::onRequestTemporaryError(MegaApi *, MegaRequest *request, MegaError* e)
{

}

//Called when a transfer has finished
void MegaApplication::onTransferFinish(MegaApi* , MegaTransfer *transfer, MegaError* e)
{	
    //Update statics
	if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
	{
		queuedDownloads--;
		totalDownloadedSize += transfer->getDeltaSize();
		downloadSpeed = transfer->getSpeed();

        //Show the transfer in the "recently updated" list
        if(e->getErrorCode() == MegaError::API_OK)
            infoDialog->addRecentFile(QString(transfer->getFileName()), transfer->getNodeHandle(), transfer->getPath());
	}
	else
	{
		queuedUploads--;
		totalUploadedSize += transfer->getDeltaSize();
		uploadSpeed = transfer->getSpeed();

        //Here the file isn't added to the "recently updated" list,
        //because the file isn't in the destination folder yet.
        //The SDK still has to put the new node.
        //onNodes update will be called with node->tag == transfer->getTag()
        //so we save the path of the file to show it later
        if(e->getErrorCode() == MegaError::API_OK)
            uploadLocalPaths[transfer->getTag()]=transfer->getPath();
	}

    //Send updated statics to the information dialog
	infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
	infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
	infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName())
							,transfer->getTransferredBytes(), transfer->getTotalBytes());
	infoDialog->setTransferCount(totalDownloads, totalUploads, queuedDownloads, queuedUploads);
	infoDialog->updateDialog();

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if(!queuedDownloads && !queuedUploads)
    {
		totalUploads = totalDownloads = 0;
		totalUploadSize = totalDownloadSize = 0;
		totalUploadedSize = totalDownloadedSize = 0;
		uploadSpeed = downloadSpeed = 0;
		this->showSyncedIcon();
        if(reboot)
        {
            QTimer::singleShot(10000, this, SLOT(rebootApplication()));
        }
	}
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    //Update statics
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

    //Send updated statics to the information dialog
    infoDialog->setTransfer(transfer->getType(), QString(transfer->getFileName()),
                        transfer->getTransferredBytes(), transfer->getTotalBytes());
	infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
	infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
	infoDialog->updateDialog();
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{
    //Show information to users
    showWarningMessage(tr("Temporarily error in transfer: ") + e->getErrorString(), transfer->getFileName());
}

//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi* api, UserList *users)
{

}

//Called when nodes have been updated in MEGA
void MegaApplication::onNodesUpdate(MegaApi* api, NodeList *nodes)
{
    bool externalNodes = 0;

    //If this is a full reload, return
	if(!nodes) return;

    //Check all modified nodes
	for(int i=0; i<nodes->size(); i++)
	{
		Node *node = nodes->get(i);
        if(!node->tag) externalNodes++;

		if(!node->removed && node->tag && !node->syncdeleted)
		{
            //If the node has been modified by a local operation...

            cout << "Adding recent upload from nodes_update: " << node->displayname() << "   tag: " << node->tag << endl;

            //Get the associated local node
            QString localPath;
			if(node->localnode)
			{
                //If the node has been uploaded by a synced folder
                //The SDK provides its local path

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
                //If the node has been uploaded by a regular upload,
                //we recover the path using the tag of the transfer
				localPath = uploadLocalPaths.value(node->tag);
				uploadLocalPaths.remove(node->tag);
				cout << "Local upload: " << localPath.toStdString() << endl;
			}

            //If we have the local path, notify the state change in the local file
			if(!localPath.isNull()) WindowsUtils::notifyItemChange(localPath);

            //If the new node is a file, add it to the "recently updated" list
            if(node->type == FILENODE) infoDialog->addRecentFile(QString::fromUtf8(node->displayname()), node->nodehandle, localPath);
		}
	}

    //Update the information dialog
	infoDialog->updateDialog();

    if(externalNodes) showNotificationMessage(tr("You have new or updated files in your account"));
}

void MegaApplication::onReloadNeeded(MegaApi* api)
{
	stopSyncs();
	megaApi->fetchNodes();
}

//TODO: Manage sync callbacks here
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

