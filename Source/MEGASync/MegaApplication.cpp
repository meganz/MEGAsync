#include "MegaApplication.h"
#include "gui/PasteMegaLinksDialog.h"
#include "gui/ImportMegaLinksDialog.h"
#include "control/Utilities.h"
#include "control/CrashHandler.h"
#include "control/ExportProcessor.h"
#include "platform/Platform.h"

#include <QTranslator>
#include <QClipboard>
#include <QDesktopWidget>
#include <QSharedMemory>
#include <QFontDatabase>
#include <QNetworkProxy>

const int MegaApplication::VERSION_CODE = 1002;
const QString MegaApplication::VERSION_STRING = QString::fromAscii("1.0.2");
const QString MegaApplication::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString MegaApplication::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();

int main(int argc, char *argv[])
{
    MegaApplication app(argc, argv);
    QString crashPath = QDir::current().filePath(QString::fromAscii("crashDumps"));
    QDir crashDir(crashPath);
    if(!crashDir.exists()) crashDir.mkpath(QString::fromAscii("."));
    CrashHandler::instance()->Init(QDir::toNativeSeparators(crashPath));

    QSharedMemory singleInstanceChecker;
    singleInstanceChecker.setKey(QString::fromAscii("MEGAsyncSingleInstanceChecker"));
    if((argc == 2) && !strcmp("/reboot", argv[1]))
    {
        //If the app is being restarted, wait up to 10 seconds or until the previous instance ends.
        for(int i=0; i<10; i++)
        {
            if(!singleInstanceChecker.attach()) break;
            singleInstanceChecker.detach();

            #ifdef WIN32
                Sleep(1000);
            #else
                sleep(1);
            #endif
        }
    }

    if(singleInstanceChecker.attach() || !singleInstanceChecker.create(1))
        return 0;

#ifndef WIN32
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Regular.ttf"));
    QFontDatabase::addApplicationFont(QString::fromAscii("://fonts/OpenSans-Semibold.ttf"));

    QFont font(QString::fromAscii("Open Sans"), 8);
    app.setFont(font);
#endif

    //QDate betaLimit(2014, 1, 21);
    //long long now = QDateTime::currentDateTime().toMSecsSinceEpoch();
    //long long betaLimitTime = QDateTime(betaLimit).toMSecsSinceEpoch();
    //if(now > betaLimitTime)
    //{
    //    QMessageBox::information(NULL, QCoreApplication::translate("MegaApplication", "MEGAsync BETA"),
    //           QCoreApplication::translate("MegaApplication", "Thank you for testing MEGAsync.<br>"
    //       "This beta version is no longer current and has expired.<br>"
    //       "Please follow <a href=\"https://twitter.com/MEGAprivacy\">@MEGAprivacy</a> on Twitter for updates."));
    //    return 0;
    //}

    app.initialize();
    app.start();
    return app.exec();
}

MegaApplication::MegaApplication(int &argc, char **argv) :
    QApplication(argc, argv)
{
    //Set QApplication fields
    setOrganizationName(QString::fromAscii("Mega Limited"));
    setOrganizationDomain(QString::fromAscii("mega.co.nz"));
    setApplicationName(QString::fromAscii("MEGAsync"));
    setApplicationVersion(QString::number(VERSION_CODE));
    appPath = QDir::toNativeSeparators(QCoreApplication::applicationFilePath());
    appDirPath = QDir::toNativeSeparators(QCoreApplication::applicationDirPath());

    //Set the working directory
#if QT_VERSION < 0x050000
    QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString dataPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

    QDir currentDir(dataPath);
    if(!currentDir.exists()) currentDir.mkpath(QString::fromAscii("."));
    QDir::setCurrent(dataPath);

    lastStartedDownload = 0;
    lastStartedUpload = 0;
    trayIcon = NULL;
    trayMenu = NULL;
    megaApi = NULL;
    httpServer = NULL;
    totalDownloadSize = totalUploadSize = 0;
    totalDownloadedSize = totalUploadedSize = 0;
    uploadSpeed = downloadSpeed = 0;
    exportOps = 0;
    infoDialog = NULL;
    setupWizard = NULL;
    settingsDialog = NULL;
    uploadFolderSelector = NULL;
    reboot = false;
    translator = NULL;
    exitAction = NULL;
    aboutAction = NULL;
    settingsAction = NULL;
    pauseAction = NULL;
    resumeAction = NULL;
    importLinksAction = NULL;
    trayMenu = NULL;
    waiting = false;
    updated = false;
}

MegaApplication::~MegaApplication()
{
}

void MegaApplication::initialize()
{
    if(megaApi != NULL) return;

    setQuitOnLastWindowClosed(false);

    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Yes");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&No");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&OK");
    QT_TRANSLATE_NOOP("QDialogButtonBox", "&Cancel");

    //Register metatypes to use them in signals/slots
    qRegisterMetaType<QQueue<QString> >("QQueueQString");
    qRegisterMetaTypeStreamOperators<QQueue<QString> >("QQueueQString");

    //Hack to have tooltips with a black background
    QApplication::setStyleSheet(QString::fromAscii("QToolTip { color: #fff; background-color: #151412; border: none; }"));

    preferences = Preferences::instance();

    delegateListener = new QTMegaListener(this);
    QString basePath = QDir::toNativeSeparators(QDir::currentPath()+QString::fromAscii("/"));
    Utilities::removeRecursively(QDir(basePath + QString::fromAscii("cache")));

#ifdef WIN32
    //Backwards compatibility code
    QDirIterator di(QDir::currentPath(), QDir::Files | QDir::NoDotAndDotDot);
    while (di.hasNext()) {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        if(fi.fileName().startsWith(QString::fromAscii(".tmp.")))
            QFile::remove(di.filePath());
    }
#endif

    string tmpPath = basePath.toStdString();
    megaApi = new MegaApi(delegateListener, &tmpPath);
    uploader = new MegaUploader(megaApi);

    //Create GUI elements
    trayIcon = new QSystemTrayIcon(this);

    QString language = preferences->language();
    changeLanguage(language);

    initialMenu = new QMenu();
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(changeProxy()));
    initialExitAction = new QAction(tr("Exit"), this);
    connect(initialExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    initialMenu->addAction(changeProxyAction);
    initialMenu->addAction(initialExitAction);

    refreshTimer = new QTimer(this);
    refreshTimer->start(10000);

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTrayIcon()));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));

    preferences->setLastExecutionTime(QDateTime::currentMSecsSinceEpoch());
}

QString MegaApplication::applicationFilePath()
{
    return appPath;
}

QString MegaApplication::applicationDirPath()
{
    return appDirPath;
}

void MegaApplication::changeLanguage(QString languageCode)
{
    if(translator)
    {
        removeTranslator(translator);
        delete translator;
        translator = NULL;
    }

    QTranslator *newTranslator = new QTranslator();
    if(newTranslator->load(MegaApplication::TRANSLATION_FOLDER + MegaApplication::TRANSLATION_PREFIX + languageCode))
    {
        installTranslator(newTranslator);
        translator = newTranslator;
    }
    else delete newTranslator;

    createTrayIcon();
}

void MegaApplication::updateTrayIcon()
{
    if(!trayIcon) return;
    if(paused)
    {
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Paused"));
    }
    else if(indexing || waiting || megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads())
    {
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
        if(indexing) trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Scanning"));
        else if(waiting) trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Waiting"));
        else trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Syncing"));
    }
    else
    {
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Up to date"));
    }

    if(infoDialog)
    {
        infoDialog->setIndexing(indexing);
        infoDialog->setWaiting(waiting);
        infoDialog->setPaused(paused);
        infoDialog->updateState();
    }
}

void MegaApplication::start()
{
    paused = false;
    indexing = false;

    trayIcon->setIcon(QIcon(QString::fromAscii("://images/login_ico.ico")));
    trayIcon->setContextMenu(initialMenu);
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    trayIcon->show();
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));
    setupWizard = NULL;

    if(updated)
    {
        showInfoMessage(tr("MEGAsync has been updated"));
        updated = false;
    }
    applyProxySettings();

    //Start the initial setup wizard if needed
    if(!preferences->logged())
    {
        setupWizard = new SetupWizard(this);
		setupWizard->exec();
        if(!preferences->logged())
            ::exit(0);
        delete setupWizard;
        setupWizard = NULL;
        loggedIn();
        startSyncs();
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
    if(settingsDialog)
        settingsDialog->setProxyOnly(false);

    //Apply the "Start on startup" configuration
    Platform::startOnStartup(preferences->startOnStartup());

    startUpdateTask();

    QString language = preferences->language();
    changeLanguage(language);

    //Show the tray icon
    createTrayIcon();
    if(!preferences->lastExecutionTime()) showInfoMessage(tr("MEGAsync is now running. Click here to open the status window."));
    else showNotificationMessage(tr("MEGAsync is now running. Click here to open the status window."));
    preferences->setLastExecutionTime(QDateTime::currentDateTime().toMSecsSinceEpoch());

    infoDialog = new InfoDialog(this);

    //Set the upload limit
    setUploadLimit(preferences->uploadLimitKB());

    //Try to keep the tray icon always active
    if(!Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName()))
        LOG("Error enabling trayicon");
    else
        LOG("OK enabling trayicon");

    Platform::startShellDispatcher(this);

    //Start the HTTP server
    //httpServer = new HTTPServer(2973, NULL);
    megaApi->getAccountDetails();
}

void MegaApplication::startSyncs()
{
    //Ensure that there is no active syncs
    if(megaApi->getNumActiveSyncs() != 0) stopSyncs();

    //Start syncs
    MegaNode *rubbishNode =  megaApi->getRubbishNode();
	for(int i=0; i<preferences->getNumSyncedFolders(); i++)
	{
        MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
        if(!node)
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                "because the remote folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->removeSyncedFolder(i);
            i--;
            continue;
        }

        MegaNode *parentNode = megaApi->getParentNode(node);
        if(parentNode->getHandle() == rubbishNode->getHandle())
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                "because the remote folder is in the rubbish bin")
                             .arg(preferences->getSyncName(i)));
            preferences->removeSyncedFolder(i);
            i--;
            delete parentNode;
            continue;
        }
        delete parentNode;

        QString localFolder = preferences->getLocalFolder(i);
        if(!QFileInfo(localFolder).isDir())
        {
            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                "because the local folder doesn't exist")
                             .arg(preferences->getSyncName(i)));
            preferences->removeSyncedFolder(i);
            i--;
            continue;
        }

        LOG(QString::fromAscii("Sync  %1 added.").arg(i));
        megaApi->syncFolder(localFolder.toUtf8().constData(), node);
        delete node;
	}
    delete rubbishNode;
}

void MegaApplication::stopSyncs()
{
    //Stop syncs
    megaApi->stopSyncs();
}

//This function is called to upload all files in the uploadQueue field
//to the Mega node that is passed as parameter
void MegaApplication::processUploadQueue(handle nodeHandle)
{
    MegaNode *node = megaApi->getNodeByHandle(nodeHandle);
    QStringList notUploaded;

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if(!node || node->getType()==FILENODE)
	{
		uploadQueue.clear();
        showErrorMessage(tr("Error: Invalid destination folder. The upload has been cancelled"));
        delete node;
        return;
	}

    //Process the upload queue using the MegaUploader object
	while(!uploadQueue.isEmpty())
	{
		QString filePath = uploadQueue.dequeue();
        if(!Utilities::verifySyncedFolderLimits(filePath))
        {
            //If a folder can't be uploaded, save its name
            notUploaded.append(QFileInfo(filePath).fileName());
            continue;
        }
        uploader->upload(filePath, node);
    }
    delete node;

    //If any file or folder couldn't be uploaded, inform users
    if(notUploaded.size())
    {
        if(notUploaded.size()==1)
        {            
            showInfoMessage(tr("The folder (%1) wasn't uploaded "
                               "because it's too large (this beta is limited to %2 folders or %3 files.")
                .arg(notUploaded[0]).arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
                .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER));
        }
        else
        {
            showInfoMessage(tr("%1 folders weren't uploaded "
                 "because they are too large (this beta is limited to %2 folders or %3 files.")
                .arg(notUploaded.size()).arg(Preferences::MAX_FOLDERS_IN_NEW_SYNC_FOLDER)
                .arg(Preferences::MAX_FILES_IN_NEW_SYNC_FOLDER));
        }
    }
}

void MegaApplication::rebootApplication()
{
    if(megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads())
        return;

    QString app = MegaApplication::applicationFilePath();
    QStringList args = QStringList();
    args.append(QString::fromAscii("/reboot"));
    QProcess::startDetached(app, args);
    QApplication::exit();
}

void MegaApplication::exitApplication()
{
    if(!megaApi->isLoggedIn() || QMessageBox::question(NULL, tr("MEGAsync"),
            tr("Synchronization will stop.\nDeletions that occur while it is not running will not be propagated.\n\nExit anyway?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        QApplication::exit();
    }
}

void MegaApplication::pauseTransfers(bool pause)
{
    megaApi->pauseTransfers(pause);
}

void MegaApplication::aboutDialog()
{
    QMessageBox::about(NULL, tr("About MEGAsync"), tr("MEGAsync version code %1").arg(this->applicationVersion()));
}

void MegaApplication::refreshTrayIcon()
{
    if(megaApi)
    {
        megaApi->updateStatics();
        onSyncStateChanged(megaApi);
    }
    if(trayIcon) trayIcon->show();
}

void MegaApplication::cleanAll()
{
    LOG("Cleaning resources");
    stopSyncs();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    trayIcon->hide();
    processEvents();
    megaApi->removeListener(delegateListener);
    delete megaApi;
    delete delegateListener;
    delete preferences;
}

void MegaApplication::unlink()
{
    //Reset fields that will be initialized again upon login
    //delete httpServer;
    //httpServer = NULL;
    stopSyncs();
    Platform::stopShellDispatcher();
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
    if(!preferences->updateAutomatically()) return;

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

void MegaApplication::applyProxySettings()
{
    QNetworkProxy proxy;

    MegaProxySettings proxySettings;
    proxySettings.setProxyType(preferences->proxyType());
    string proxyString = preferences->proxyString().toStdString();
    proxySettings.setProxyURL(&proxyString);
    string username = preferences->getProxyUsername().toStdString();
    string password = preferences->getProxyPassword().toStdString();
    proxySettings.setCredentials(&username, &password);

    if(preferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM)
    {
        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(preferences->proxyServer());
        proxy.setPort(preferences->proxyPort());
        if(preferences->proxyRequiresAuth())
        {
            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
        megaApi->setProxySettings(&proxySettings);
    }
    else if(preferences->proxyType() == Preferences::PROXY_TYPE_AUTO)
    {
        megaApi->setProxySettings(&proxySettings);
        if(preferences->proxyServer().size())
        {
            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(preferences->proxyServer());
            proxy.setPort(preferences->proxyPort());
        }
    }
    else
    {
        megaApi->setProxySettings(&proxySettings);
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

void MegaApplication::showUpdatedMessage()
{
    updated = true;
}

void MegaApplication::pauseSync()
{
    pauseTransfers(true);
}

void MegaApplication::resumeSync()
{
    pauseTransfers(false);
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
    if(uploadFolderSelector)
    {
        uploadFolderSelector->showMinimized();
        uploadFolderSelector->setWindowState(Qt::WindowActive);
        uploadFolderSelector->showNormal();
        uploadFolderSelector->raise();
        uploadFolderSelector->activateWindow();
        return;
    }

    //If there is a default upload folder in the preferences
    MegaNode *node = megaApi->getNodeByHandle(preferences->uploadFolder());
	if(node)
	{
        //use it to upload the list of files
        processUploadQueue(node->getHandle());
        delete node;
		return;
	}

    //If there isn't a default upload folder, show the dialog
	uploadFolderSelector = new UploadToMegaDialog(megaApi);
    showUploadDialog();
    return;
}

void MegaApplication::shellExport(QQueue<QString> newExportQueue)
{
    ExportProcessor *processor = new ExportProcessor(megaApi, newExportQueue);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

void MegaApplication::showUploadDialog()
{
    uploadFolderSelector->showMinimized();
    uploadFolderSelector->setWindowState(Qt::WindowActive);
    uploadFolderSelector->showNormal();
    uploadFolderSelector->raise();
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

void MegaApplication::onRequestLinksFinished()
{
    ExportProcessor *exportProcessor = ((ExportProcessor *)QObject::sender());
    QStringList links = exportProcessor->getValidLinks();
    if(!links.size()) return;
    QString linkForClipboard(links.join(QChar::fromAscii('\n')));
    QApplication::clipboard()->setText(linkForClipboard);
    if(links.size()==1) showInfoMessage(tr("The link has been copied to the clipboard"));
    else showInfoMessage(tr("The links have been copied to the clipboard"));
    exportProcessor->deleteLater();
    exportOps--;
}

void MegaApplication::onUpdateCompleted()
{
    LOG("Update completed. Initializing a silent reboot...");
    reboot = true;
    QTimer::singleShot(10000, this, SLOT(rebootApplication()));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    if(reason == QSystemTrayIcon::Trigger)
    {
        if(!infoDialog)
        {
            if(setupWizard)
            {
                setupWizard->setVisible(true);
                setupWizard->activateWindow();
            }
            else showInfoMessage(tr("Logging in..."));

            return;
        }

        if(!infoDialog->isVisible())
        {
            //Put it in the right position (to prevent problems with changes in the taskbar or the resolution)
            QRect screenGeometry = QApplication::desktop()->availableGeometry();
            infoDialog->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 545 - 2);

            //Show the dialog
            infoDialog->showMinimized();
            infoDialog->setWindowState(Qt::WindowActive);
            infoDialog->showNormal();
            infoDialog->raise();
            infoDialog->activateWindow();
            infoDialog->setFocus();
        }
        else infoDialog->hide();
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

void MegaApplication::changeProxy()
{
    if(settingsDialog)
    {
        settingsDialog->setProxyOnly(true);

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
    settingsDialog->setProxyOnly(true);
    settingsDialog->show();
}

//This function creates the tray icon
void MegaApplication::createTrayIcon()
{
    if(!trayMenu) trayMenu = new QMenu();
    else
    {
        QList<QAction *> actions = trayMenu->actions();
        for(int i=0; i<actions.size(); i++)
        {
            trayMenu->removeAction(actions[i]);
        }
    }

    if(exitAction) delete exitAction;
    exitAction = new QAction(tr("Exit"), this);
    connect(exitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));

    if(aboutAction) delete aboutAction;
    aboutAction = new QAction(tr("About"), this);
    connect(aboutAction, SIGNAL(triggered()), this, SLOT(aboutDialog()));

    if(settingsAction) delete settingsAction;
    settingsAction = new QAction(tr("Settings"), this);
    connect(settingsAction, SIGNAL(triggered()), this, SLOT(openSettings()));

    if(pauseAction) delete pauseAction;
    pauseAction = new QAction(tr("Pause"), this);
    connect(pauseAction, SIGNAL(triggered()), this, SLOT(pauseSync()));

    if(resumeAction) delete resumeAction;
    resumeAction = new QAction(tr("Resume"), this);
    connect(resumeAction, SIGNAL(triggered()), this, SLOT(resumeSync()));

    if(importLinksAction) delete importLinksAction;
    importLinksAction = new QAction(tr("Import links"), this);
    connect(importLinksAction, SIGNAL(triggered()), this, SLOT(importLinks()));

    //trayMenu->addAction(aboutAction);
    if(!paused) trayMenu->addAction(pauseAction);
    else trayMenu->addAction(resumeAction);
	trayMenu->addAction(importLinksAction);
    trayMenu->addAction(settingsAction);
    trayMenu->addAction(exitAction);

    trayIcon->setContextMenu(trayMenu);

    updateTrayIcon();
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
        if(!exportOps && e->getErrorCode() == MegaError::API_OK)
		{
            //A public link has been created, put it in the clipboard and inform users
            QString linkForClipboard(QString::fromUtf8(request->getLink()));
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
                break;
			}

            //Wrong login -> logout
            unlink();
		}
		break;
	}
    case MegaRequest::TYPE_LOGOUT:
    {
        if(preferences && preferences->logged())
        {
            preferences->unlink();
            if(infoDialog) delete infoDialog;
            infoDialog = NULL;
            start();
        }
        break;
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
            LOG("Error fetching nodes");
            unlink();
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        if(!preferences->logged()) break;

		if(e->getErrorCode() != MegaError::API_OK)
			break;

        //Account details retrieved, update the preferences and the information dialog
        AccountDetails *details = request->getAccountDetails();
        preferences->setAccountType(details->pro_level);
        preferences->setTotalStorage(details->storage_max);
        preferences->setUsedStorage(details->storage_used);
		preferences->setTotalBandwidth(details->transfer_max);
		preferences->setUsedBandwidth(details->transfer_own_used);
        if(infoDialog) infoDialog->setUsage(details->storage_max, details->storage_used);
        break;
    }
    case MegaRequest::TYPE_PAUSE_TRANSFERS:
    {
        paused = request->getFlag();
        if(paused)
        {
            trayMenu->removeAction(pauseAction);
            trayMenu->insertAction(importLinksAction, resumeAction);
        }
        else
        {
            trayMenu->removeAction(resumeAction);
            trayMenu->insertAction(importLinksAction, pauseAction);
        }
        updateTrayIcon();
        break;
    }
    case MegaRequest::TYPE_ADD_SYNC:
    {
        LOG("Sync added!");
        for(int i=preferences->getNumSyncedFolders()-1; i>=0; i--)
        {
            if((request->getNodeHandle() == preferences->getMegaFolderHandle(i)))
            {
                if(e->getErrorCode() != MegaError::API_OK)
                {
                    if(e->getErrorCode() == MegaError::API_ENOENT)
                    {
                        QString localFolder = preferences->getLocalFolder(i);
                        MegaNode *node = megaApi->getNodeByHandle(preferences->getMegaFolderHandle(i));
                        MegaNode *parentNode = megaApi->getParentNode(node);
                        MegaNode *rubbishNode = megaApi->getRubbishNode();
                        if(!node)
                        {
                            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                                "because the remote folder doesn't exist")
                                             .arg(preferences->getSyncName(i)));
                        }
                        else if(parentNode && (parentNode->getHandle() == rubbishNode->getHandle()))
                        {
                            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                                "because the remote folder is in the rubbish bin")
                                             .arg(preferences->getSyncName(i)));
                        }
                        else if(!QFileInfo(localFolder).isDir())
                        {
                            showErrorMessage(tr("Your sync \"%1\" has been disabled\n"
                                                "because the local folder doesn't exist")
                                             .arg(preferences->getSyncName(i)));
                        }
                        delete node;
                        delete parentNode;
                        delete rubbishNode;
                    }
                    LOG("Sync error! Removed");
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i));
                    preferences->removeSyncedFolder(i);
                }
                else
                {
                    LOG("Sync notified!");
                    Platform::syncFolderAdded(preferences->getLocalFolder(i), preferences->getSyncName(i));
                }
                break;
            }
        }
        if(infoDialog)
        {
            LOG("Sync commited! Updating GUI");
            infoDialog->updateSyncsButton();
        }
        break;
    }
    case MegaRequest::TYPE_REMOVE_SYNC:
    {
        if(infoDialog) infoDialog->updateSyncsButton();
        LOG("Sync removed");
        Platform::syncFolderRemoved(QString::fromUtf8(request->getFile()));
        onSyncStateChanged(megaApi);
        break;
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
		totalDownloadSize += transfer->getTotalBytes();
	}
	else
	{
		uploadSpeed = 0;
		totalUploadSize += transfer->getTotalBytes();
	}

    //Send statics to the information dialog
    if(infoDialog) infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);
    updateTrayIcon();
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
		totalDownloadedSize += transfer->getDeltaSize();
		downloadSpeed = transfer->getSpeed();

        //Show the transfer in the "recently updated" list
        if(e->getErrorCode() == MegaError::API_OK)
        {
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif
            if(infoDialog) infoDialog->addRecentFile(QString::fromUtf8(transfer->getFileName()), transfer->getNodeHandle(), localPath);
        }
	}
	else
	{
		totalUploadedSize += transfer->getDeltaSize();
		uploadSpeed = transfer->getSpeed();

        //Here the file isn't added to the "recently updated" list,
        //because the file isn't in the destination folder yet.
        //The SDK still has to put the new node.
        //onNodes update will be called with node->tag == transfer->getTag()
        //so we save the path of the file to show it later
        if((e->getErrorCode() == MegaError::API_OK) && (!transfer->isSyncTransfer()))
        {
            LOG(QString::fromAscii("Putting: %1 TAG: %2").arg(QString::fromUtf8(transfer->getPath())).arg(transfer->getTag()));
            QString localPath = QString::fromUtf8(transfer->getPath());
            #ifdef WIN32
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #endif
            uploadLocalPaths[transfer->getTag()]=localPath;
        }
        else LOG("Sync Transfer");
	}

    //Send updated statics to the information dialog
    if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
        ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
    {
        if(infoDialog)
        {
            infoDialog->setTransfer(transfer);
            infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
            infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
            infoDialog->updateTransfers();
            infoDialog->updateDialog();
        }
    }

    if(infoDialog) infoDialog->increaseUsedStorage(transfer->getTotalBytes());
    preferences->setUsedStorage(preferences->usedStorage()+transfer->getTotalBytes());

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if(!megaApi->getNumPendingDownloads() && !megaApi->getNumPendingUploads())
    {
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
        if(!transfer->getTransferredBytes()) lastStartedDownload = transfer->getStartTime();
		downloadSpeed = transfer->getSpeed();
		totalDownloadedSize += transfer->getDeltaSize();
	}
	else
	{
        if(!transfer->getTransferredBytes()) lastStartedUpload = transfer->getStartTime();
		uploadSpeed = transfer->getSpeed();
		totalUploadedSize += transfer->getDeltaSize();
	}

    //Send updated statics to the information dialog
    if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
        ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
    {
        if(infoDialog)
        {
            infoDialog->setTransfer(transfer);
            infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
            infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
            infoDialog->updateDialog();
        }
    }
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{    
    //Show information to users
    showWarningMessage(tr("Temporary transmission error: ") + e->QgetErrorString(), QString::fromUtf8(transfer->getFileName()));
    onSyncStateChanged(megaApi);
}

//Called when contacts have been updated in MEGA
void MegaApplication::onUsersUpdate(MegaApi* api, UserList *users)
{

}

//Called when nodes have been updated in MEGA
void MegaApplication::onNodesUpdate(MegaApi* api, NodeList *nodes)
{
    if(!infoDialog) return;

    bool externalNodes = 0;

    //If this is a full reload, return
	if(!nodes) return;

    //Check all modified nodes
    QString localPath;
    for(int i=0; i<nodes->size(); i++)
	{
        LOG(QString::fromAscii("onNodesUpdate ") + QString::number(nodes->size()));
        localPath.clear();
        MegaNode *node = nodes->get(i);

        if(node->isRemoved() && (node->getType()==FOLDERNODE))
        {
            for(int i=0; i<preferences->getNumSyncedFolders(); i++)
            {
                if(preferences->getMegaFolderHandle(i) == node->getHandle())
                {
                    LOG("Remote sync deletion detected!");
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i));
                    preferences->removeSyncedFolder(i);
                    if(infoDialog) infoDialog->updateSyncsButton();
                    onSyncStateChanged(megaApi);
                }
            }
        }

        if(!node->getTag() && !node->isRemoved() && !node->isSyncDeleted())
            externalNodes++;

        if(!node->isRemoved() && node->getTag() && !node->isSyncDeleted() && (node->getType()==FILENODE))
        {
            //Get the associated local node
            LOG(QString::fromAscii("Node: %1 TAG: %2").arg(QString::fromUtf8(node->getName())).arg(node->getTag()));
            string path = node->getLocalPath();
            if(path.size())
            {
                //If the node has been uploaded by a synced folder
                //The SDK provides its local path
                LOG("Sync upload");
            #ifdef WIN32
                localPath = QString::fromWCharArray((const wchar_t *)path.data());
                if(localPath.startsWith(QString::fromAscii("\\\\?\\"))) localPath = localPath.mid(4);
            #else
                localPath = QString::fromUtf8(path.data());
            #endif
                LOG(QString::fromAscii("Sync path: %1").arg(localPath));
            }
            else if(uploadLocalPaths.contains(node->getTag()))
            {
                //If the node has been uploaded by a regular upload,
                //we recover the path using the tag of the transfer
                localPath = uploadLocalPaths.value(node->getTag());
                uploadLocalPaths.remove(node->getTag());
                LOG(QString::fromAscii("Local upload: %1").arg(localPath));
            }

            //If we have the local path, notify the state change in the local file
            if(localPath.size())
            {
                if(infoDialog) infoDialog->addRecentFile(QString::fromUtf8(node->getName()), node->getHandle(), localPath);
            }
        }
	}

    //Update the information dialog
    if(infoDialog)
    {
        infoDialog->updateDialog();
    }

    if(externalNodes) showNotificationMessage(tr("You have new or updated files in your account"));
}

void MegaApplication::onReloadNeeded(MegaApi* api)
{
    megaApi->fetchNodes();
}

void MegaApplication::onSyncStateChanged(MegaApi *api)
{
    if(!infoDialog) return;

    infoDialog->updateTransfers();
    indexing = megaApi->isIndexing();
    waiting = megaApi->isWaiting();
    updateTrayIcon();
    for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        Platform::notifyItemChange(preferences->getLocalFolder(i));
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
    trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
    trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Up to date"));
	trayMenu->removeAction(resumeAction);
	trayMenu->insertAction(importLinksAction, pauseAction);
}


