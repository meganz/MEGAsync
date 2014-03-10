#include "MegaApplication.h"
#include "gui/ImportMegaLinksDialog.h"
#include "gui/CrashReportDialog.h"
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

#ifdef Q_OS_UNIX
 #include <signal.h>
#endif

const int MegaApplication::VERSION_CODE = 1011;
const QString MegaApplication::VERSION_STRING = QString::fromAscii("1.0.11g");
const QString MegaApplication::TRANSLATION_FOLDER = QString::fromAscii("://translations/");
const QString MegaApplication::TRANSLATION_PREFIX = QString::fromAscii("MEGASyncStrings_");

QString MegaApplication::appPath = QString();
QString MegaApplication::appDirPath = QString();

QSharedMemory singleInstanceChecker;

#ifdef Q_OS_UNIX
static void on_sigint(int sig);
#endif

int main(int argc, char *argv[])
{
    MegaApplication app(argc, argv);
    QString crashPath = QDir::current().filePath(QString::fromAscii("crashDumps"));
    QDir crashDir(crashPath);
    if(!crashDir.exists()) crashDir.mkpath(QString::fromAscii("."));
    CrashHandler::instance()->Init(QDir::toNativeSeparators(crashPath));

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
    else if((argc == 2) && !strcmp("/uninstall", argv[1]))
    {
        Preferences *preferences = Preferences::instance();
        if(preferences->logged())
            preferences->unlink();

        for(int i=0; i<preferences->getNumUsers(); i++)
        {
            preferences->enterUser(i);
            for(int j=0; j<preferences->getNumSyncedFolders(); j++)
                Platform::syncFolderRemoved(preferences->getLocalFolder(j), preferences->getSyncName(j));
            preferences->leaveUser();
        }

#if QT_VERSION < 0x050000
    QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString dataPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif
        QDir dataDir(dataPath);
        Utilities::removeRecursively(dataDir);

        return 0;
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

    QT_TRANSLATE_NOOP("Installer", "Choose Users");
    QT_TRANSLATE_NOOP("Installer", "Choose for which users you want to install $(^NameDA).");
    QT_TRANSLATE_NOOP("Installer", "Select whether you want to install $(^NameDA) for yourself only or for all users of this computer. $(^ClickNext)");
    QT_TRANSLATE_NOOP("Installer", "Install for anyone using this computer");
    QT_TRANSLATE_NOOP("Installer", "Install just for me");
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

    finished = false;
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
    updateAction = NULL;
    pasteMegaLinksDialog = NULL;
    updateBlocked = false;
    updateThread = NULL;
    updateTask = NULL;
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
    preferences->setLastStatsRequest(0);
    lastExit = preferences->getLastExit();

    QString basePath = QDir::toNativeSeparators(QDir::currentPath()+QString::fromAscii("/"));
    Utilities::removeRecursively(QDir(basePath + QString::fromAscii("cache")));

#if QT_VERSION < 0x050000
    QString dataPath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    QString dataPath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

#ifdef WIN32
    //Backwards compatibility code
    QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
    while (di.hasNext()) {
        di.next();
        const QFileInfo& fi = di.fileInfo();
        if(fi.fileName().startsWith(QString::fromAscii(".tmp.")))
            QFile::remove(di.filePath());
    }
#endif

    megaApi = new MegaApi(basePath.toUtf8().constData());
    delegateListener = new QTMegaListener(megaApi, this);
    megaApi->addListener(delegateListener);
    uploader = new MegaUploader(megaApi);
    uploadFolderSelector = new UploadToMegaDialog(megaApi);
    connect(uploader, SIGNAL(dupplicateUpload(QString, QString, long long)), this, SLOT(onDupplicateUpload(QString, QString, long long)));

    if(preferences->isCrashed())
    {
        preferences->setCrashed(false);
        QDirIterator di(dataPath, QDir::Files | QDir::NoDotAndDotDot);
        while (di.hasNext()) {
            di.next();
            const QFileInfo& fi = di.fileInfo();
            if(fi.fileName().endsWith(QString::fromAscii(".db")))
                QFile::remove(di.filePath());
        }

        QStringList reports = CrashHandler::instance()->getPendingCrashReports();
        if(reports.size())
        {
            CrashReportDialog crashDialog(reports.join(QString::fromAscii("------------------------------\n")));
            if(crashDialog.exec() == QDialog::Accepted)
            {
                applyProxySettings();
                CrashHandler::instance()->sendPendingCrashReports(crashDialog.getUserMessage());
                QMessageBox::information(NULL, QString::fromAscii("MEGAsync"), tr("Thank you for your collaboration!"));
            }
        }
    }

    //Create GUI elements
    trayIcon = new QSystemTrayIcon();
    connect(trayIcon, SIGNAL(messageClicked()), this, SLOT(onMessageClicked()));
    connect(trayIcon, SIGNAL(activated(QSystemTrayIcon::ActivationReason)),
            this, SLOT(trayIconActivated(QSystemTrayIcon::ActivationReason)));

    QString language = preferences->language();
    changeLanguage(language);

    initialMenu = new QMenu();
    changeProxyAction = new QAction(tr("Settings"), this);
    connect(changeProxyAction, SIGNAL(triggered()), this, SLOT(changeProxy()));
    initialExitAction = new QAction(tr("Exit"), this);
    connect(initialExitAction, SIGNAL(triggered()), this, SLOT(exitApplication()));
    initialMenu->addAction(changeProxyAction);
    initialMenu->addAction(initialExitAction);

    refreshTimer = new QTimer();
    refreshTimer->start(Preferences::STATE_REFRESH_INTERVAL_MS);

    connect(refreshTimer, SIGNAL(timeout()), this, SLOT(refreshTrayIcon()));
    connect(this, SIGNAL(aboutToQuit()), this, SLOT(cleanAll()));

#ifdef Q_OS_UNIX
    // instal signal handler
    signal(SIGINT, on_sigint);
#endif
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
    if(trayIcon->contextMenu() == initialMenu)
    {
        LOG("STATE: Logging in...");
        trayIcon->setIcon(QIcon(QString::fromAscii("://images/login_ico.ico")));
        trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Logging in"));
    }
    else if(paused)
    {
        LOG("STATE: Setting the \"pause\" tray icon. The pause flag is active");
        QString tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Paused");
        if(!updateAction)
        {
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_pause.ico")));
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else if(indexing || waiting || megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads())
    {
        LOG("STATE: Setting the \"syncing\" tray icon");
        LOG("Reason:");
        if(indexing) LOG("Indexing");
        if(waiting) LOG("Waiting");
        if(megaApi->getNumPendingUploads())
        {
            LOG("Pending uploads");
            LOG(QString::number(megaApi->getNumPendingUploads()));
        }

        if(megaApi->getNumPendingDownloads())
        {
            LOG("Pending downloads");
            LOG(QString::number(megaApi->getNumPendingDownloads()));
        }

        QString tooltip;
        if(indexing) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Scanning");
        else if(waiting) tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Waiting");
        else tooltip = QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Syncing");

        if(!updateAction)
        {
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
            trayIcon->setToolTip(tooltip);
        }
        else
        {
            //TODO: Change icon
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/tray_sync.ico")));
            tooltip += QString::fromAscii("\n") + tr("Update available!");
            trayIcon->setToolTip(tooltip);
        }
    }
    else
    {
        LOG("STATE: Setting the \"synced\" tray icon (default).");
        if(!updateAction)
        {
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Up to date"));
        }
        else
        {
            //TODO: Change icon
            trayIcon->setIcon(QIcon(QString::fromAscii("://images/app_ico.ico")));
            trayIcon->setToolTip(QCoreApplication::applicationName() + QString::fromAscii(" ") + MegaApplication::VERSION_STRING + QString::fromAscii("\n") + tr("Update available!"));
        }

        if(reboot)
            QTimer::singleShot(Preferences::REBOOT_DELAY_MS, this, SLOT(rebootApplication()));
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
    if(!preferences->lastExecutionTime())
        Platform::enableTrayIcon(QFileInfo(MegaApplication::applicationFilePath()).fileName());

    if(updated)
        showInfoMessage(tr("MEGAsync has been updated"));

    applyProxySettings();

    //Start the initial setup wizard if needed
    if(!preferences->logged())
    {
        updated = false;
        setupWizard = new SetupWizard(this);
		setupWizard->exec();
        if(!preferences->logged())
            ::exit(0);
        delete setupWizard;
        setupWizard = NULL;

        QStringList exclusions = preferences->getExcludedSyncNames();
        vector<string> vExclusions;
        for(int i=0; i<exclusions.size(); i++)
            vExclusions.push_back(exclusions[i].toUtf8().constData());
        megaApi->setExcludedNames(&vExclusions);

        loggedIn();
        startSyncs();
    }
	else
	{
        QStringList exclusions = preferences->getExcludedSyncNames();
        vector<string> vExclusions;
        for(int i=0; i<exclusions.size(); i++)
            vExclusions.push_back(exclusions[i].toUtf8().constData());
        megaApi->setExcludedNames(&vExclusions);

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
    else if(!updated) showNotificationMessage(tr("MEGAsync is now running. Click here to open the status window."));
    preferences->setLastExecutionTime(QDateTime::currentDateTime().toMSecsSinceEpoch());

    infoDialog = new InfoDialog(this);

    //Set the upload limit
    setUploadLimit(preferences->uploadLimitKB());
    Platform::startShellDispatcher(this);

    //Start the HTTP server
    //httpServer = new HTTPServer(2973, NULL);
    updateUserStats();
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
void MegaApplication::processUploadQueue(mega::handle nodeHandle)
{
    MegaNode *node = megaApi->getNodeByHandle(nodeHandle);
    QStringList notUploaded;

    //If the destination node doesn't exist in the current filesystem, clear the queue and show an error message
    if(!node || node->isFile())
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
            showInfoMessage(tr("The folder (%1) wasn't uploaded because it's extremely large. We do this check to prevent the uploading of entire boot volumes, which is inefficient and dangerous.")
                .arg(notUploaded[0]));
        }
        else
        {
            showInfoMessage(tr("%1 folders weren't uploaded because they are extremely large. We do this check to prevent the uploading of entire boot volumes, which is inefficient and dangerous.")
                .arg(notUploaded.size()));
        }
    }
}

void MegaApplication::rebootApplication()
{
    if(megaApi->getNumPendingDownloads() || megaApi->getNumPendingUploads() || megaApi->isWaiting())
    {
        if(!updateBlocked)
        {
            updateBlocked = true;
            showInfoMessage(tr("An update will be applied during the next application restart"));
        }
        return;
    }

    QString app = MegaApplication::applicationFilePath();
    QStringList args = QStringList();
    args.append(QString::fromAscii("/reboot"));
    QProcess::startDetached(app, args);
    trayIcon->hide();
    QApplication::exit();
}

void MegaApplication::exitApplication()
{
    if(!megaApi->isLoggedIn() || QMessageBox::question(NULL, tr("MEGAsync"),
            tr("Synchronization will stop.\nDeletions that occur while it is not running will not be propagated.\n\nExit anyway?"),
            QMessageBox::Yes|QMessageBox::No) == QMessageBox::Yes)
    {
        trayIcon->hide();
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
        LOG("STATE: Refreshing state");
        megaApi->update();
        megaApi->updateStatics();
        onSyncStateChanged(megaApi);
    }
    if(trayIcon) trayIcon->show();
}

#ifdef Q_OS_UNIX
// signal handler
static void on_sigint(int sig)
{
    // delete shared memory key
    singleInstanceChecker.detach();

    // reset the default handler
    signal(SIGINT, SIG_DFL);
    // call again
    raise(SIGINT);
}
#endif

void MegaApplication::cleanAll()
{
    LOG("Cleaning resources");
    finished = true;
    stopSyncs();
    stopUpdateTask();
    Platform::stopShellDispatcher();

    delete uploader;
    delete pasteMegaLinksDialog;
    delete uploadFolderSelector;
    delete delegateListener;
    delete megaApi;

    preferences->setLastExit(QDateTime::currentMSecsSinceEpoch());

    // remove shared memory key
    singleInstanceChecker.detach();
}

void MegaApplication::onDupplicateLink(QString link, QString name, long long handle)
{
    addRecentFile(name, handle);
}

void MegaApplication::onDupplicateUpload(QString localPath, QString name, long long handle)
{
    addRecentFile(name, handle, localPath);
}

void MegaApplication::onInstallUpdateClicked()
{
    showInfoMessage(tr("Installing update..."));
    emit installUpdate();
}

void MegaApplication::unlink()
{
    //Reset fields that will be initialized again upon login
    //delete httpServer;
    //httpServer = NULL;
    stopSyncs();
    stopUpdateTask();
    Platform::stopShellDispatcher();
    megaApi->logout();
}

void MegaApplication::showInfoMessage(QString message, QString title)
{
    if(trayIcon)
    {
        lastTrayMessage = message;
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information);
    }
    else QMessageBox::information(NULL, title, message);
}

void MegaApplication::showWarningMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(trayIcon)
    {
        lastTrayMessage = message;
        trayIcon->showMessage(title, message, QSystemTrayIcon::Warning);
    }
    else QMessageBox::warning(NULL, title, message);
}

void MegaApplication::showErrorMessage(QString message, QString title)
{
    QMessageBox::critical(NULL, title, message);
}

void MegaApplication::showNotificationMessage(QString message, QString title)
{
    if(!preferences->showNotifications()) return;
    if(trayIcon)
    {
        lastTrayMessage = message;
        trayIcon->showMessage(title, message, QSystemTrayIcon::Information, 8000);
    }
}

//KB/s
void MegaApplication::setUploadLimit(int limit)
{
    if(limit<0) megaApi->setUploadLimit(-1);
    else megaApi->setUploadLimit(limit*1024);
}

void MegaApplication::startUpdateTask()
{
    if(!updateThread && preferences->canUpdate())
    {
        updateThread = new QThread();
        updateTask = new UpdateTask();
        updateTask->moveToThread(updateThread);

        connect(this, SIGNAL(startUpdaterThread()), updateTask, SLOT(startUpdateThread()), Qt::UniqueConnection);
        connect(this, SIGNAL(tryUpdate()), updateTask, SLOT(checkForUpdates()), Qt::UniqueConnection);
        connect(this, SIGNAL(installUpdate()), updateTask, SLOT(installUpdate()), Qt::UniqueConnection);

        connect(updateTask, SIGNAL(updateCompleted()), this, SLOT(onUpdateCompleted()), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateAvailable(bool)), this, SLOT(onUpdateAvailable(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateNotFound(bool)), this, SLOT(onUpdateNotFound(bool)), Qt::UniqueConnection);
        connect(updateTask, SIGNAL(updateError()), this, SLOT(onUpdateError()), Qt::UniqueConnection);

        connect(updateThread, SIGNAL(finished()), updateTask, SLOT(deleteLater()), Qt::UniqueConnection);
        connect(updateThread, SIGNAL(finished()), updateThread, SLOT(deleteLater()), Qt::UniqueConnection);

        updateThread->start();
        emit startUpdaterThread();
    }
}

void MegaApplication::stopUpdateTask()
{
    if(updateThread)
    {
        updateThread->quit();
        updateThread = NULL;
        updateTask = NULL;
    }
}

void MegaApplication::applyProxySettings()
{
    QNetworkProxy proxy;

    MegaProxySettings proxySettings;
    proxySettings.setProxyType(preferences->proxyType());
    string proxyString = preferences->proxyHostAndPort().toStdString();
    proxySettings.setProxyURL(&proxyString);

    if(preferences->proxyType() == Preferences::PROXY_TYPE_CUSTOM)
    {
        LOG("Custom proxy QT");
        LOG(preferences->proxyServer());
        LOG(QString::number(preferences->proxyPort()));

        proxy.setType(QNetworkProxy::HttpProxy);
        proxy.setHostName(preferences->proxyServer());
        proxy.setPort(preferences->proxyPort());
        if(preferences->proxyRequiresAuth())
        {
            LOG("Auth proxy QT");
            string username = preferences->getProxyUsername().toStdString();
            string password = preferences->getProxyPassword().toStdString();
            proxySettings.setCredentials(&username, &password);

            proxy.setUser(preferences->getProxyUsername());
            proxy.setPassword(preferences->getProxyPassword());
        }
        megaApi->setProxySettings(&proxySettings);
    }
    else if(preferences->proxyType() == Preferences::PROXY_TYPE_AUTO)
    {
        LOG("Auto proxy QT");

        megaApi->setProxySettings(&proxySettings);
        if(preferences->proxyServer().size())
        {
            LOG(preferences->proxyServer());
            LOG(QString::number(preferences->proxyPort()));

            proxy.setType(QNetworkProxy::HttpProxy);
            proxy.setHostName(preferences->proxyServer());
            proxy.setPort(preferences->proxyPort());
        }
        else
        {
            LOG("No proxy found QT");
            proxy.setType(QNetworkProxy::NoProxy);
        }

    }
    else
    {
        LOG("No proxy QT");

        proxy.setType(QNetworkProxy::NoProxy);
        megaApi->setProxySettings(&proxySettings);
    }
    QNetworkProxy::setApplicationProxy(proxy);
}

void MegaApplication::showUpdatedMessage()
{
    updated = true;
}

void MegaApplication::updateUserStats()
{
    long long lastRequest = preferences->lastStatsRequest();
    if((QDateTime::currentMSecsSinceEpoch()-lastRequest) > Preferences::MIN_UPDATE_STATS_INTERVAL)
    {
        preferences->setLastStatsRequest(QDateTime::currentMSecsSinceEpoch());
        megaApi->getAccountDetails();
    }
}

void MegaApplication::addRecentFile(QString fileName, long long fileHandle, QString localPath)
{
    if(infoDialog)
        infoDialog->addRecentFile(fileName, fileHandle, localPath);
}

void MegaApplication::checkForUpdates()
{
    this->showInfoMessage(tr("Checking for updates..."));
    emit tryUpdate();
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
    if(pasteMegaLinksDialog)
    {
        pasteMegaLinksDialog->setVisible(true);
        pasteMegaLinksDialog->activateWindow();
        pasteMegaLinksDialog->raise();
        pasteMegaLinksDialog->setFocus();
        return;
    }

    //Show the dialog to paste public links
    pasteMegaLinksDialog = new PasteMegaLinksDialog();
    pasteMegaLinksDialog->exec();

    //If the dialog isn't accepted, return
    if(pasteMegaLinksDialog->result()!=QDialog::Accepted) return;

    //Get the list of links from the dialog
    QStringList linkList = pasteMegaLinksDialog->getLinks();
    delete pasteMegaLinksDialog;
    pasteMegaLinksDialog = NULL;

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
        connect(linkProcessor, SIGNAL(onDupplicateLink(QString, QString, long long)),
                this, SLOT(onDupplicateLink(QString, QString, long long)));
		linkProcessor->importLinks(importDialog.getImportPath());
	}
    //If importing links isn't needed, we can delete the link processor
    //It doesn't track transfers, only the importation of links
	else delete linkProcessor;
}

//Called when the user wants to generate the public link for a node
void MegaApplication::copyFileLink(mega::handle fileHandle)
{
    //Launch the creation of the import link, it will be handled in the "onRequestFinish" callback
	megaApi->exportNode(megaApi->getNodeByHandle(fileHandle));
}

//Called when the user wants to upload a list of files and/or folders from the shell
void MegaApplication::shellUpload(QQueue<QString> newUploadQueue)
{
    if(finished) return;

    //Append the list of files to the upload queue
	uploadQueue.append(newUploadQueue);

    //If the dialog to select the upload folder is active, return.
    //Files will be uploaded when the user selects the upload folder
    if(uploadFolderSelector->isVisible())
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

    showUploadDialog();
    return;
}

void MegaApplication::shellExport(QQueue<QString> newExportQueue)
{
    if(finished) return;

    ExportProcessor *processor = new ExportProcessor(megaApi, newExportQueue);
    connect(processor, SIGNAL(onRequestLinksFinished()), this, SLOT(onRequestLinksFinished()));
    processor->requestLinks();
    exportOps++;
}

void MegaApplication::showUploadDialog()
{
    uploadFolderSelector->initialize();
    uploadFolderSelector->showMinimized();
    uploadFolderSelector->setWindowState(Qt::WindowActive);
    uploadFolderSelector->showNormal();
    uploadFolderSelector->raise();
    uploadFolderSelector->activateWindow();
    uploadFolderSelector->exec();

    if(uploadFolderSelector->result()==QDialog::Accepted)
	{
        //If the dialog is accepted, get the destination node
        mega::handle nodeHandle = uploadFolderSelector->getSelectedHandle();
        if(uploadFolderSelector->isDefaultFolder())
            preferences->setUploadFolder(nodeHandle);
        processUploadQueue(nodeHandle);
	}
    //If the dialog is rejected, cancel uploads
	else uploadQueue.clear();
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
    if(trayMenu)
        trayMenu->removeAction(updateAction);
    delete updateAction;
    updateAction = NULL;
    reboot = true;
    rebootApplication();
}

void MegaApplication::onUpdateAvailable(bool requested)
{
    if(!updateAction && (trayIcon->contextMenu()==trayMenu))
    {
        updateAction = new QAction(tr("Install update"), this);
        connect(updateAction, SIGNAL(triggered()), this, SLOT(onInstallUpdateClicked()));
        if(trayMenu && !trayMenu->actions().contains(updateAction))
        {
            trayMenu->addAction(updateAction);
            trayIcon->setContextMenu(trayMenu);
        }
    }

    if(requested)
        showInfoMessage(tr("A new version of MEGAsync is available! Click on this message to install it"));
}

void MegaApplication::onUpdateNotFound(bool requested)
{
    if(requested)
    {
        if(!updateAction)
            showInfoMessage(tr("No updates available"));
        else
            showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync"));
    }
}

void MegaApplication::onUpdateError()
{
    showInfoMessage(tr("There was a problem installing the update. Please try again later or download the last version from:\nhttps://mega.co.nz/#sync"));
}

//Called when users click in the tray icon
void MegaApplication::trayIconActivated(QSystemTrayIcon::ActivationReason reason)
{
    LOG("Tray icon clicked");
    if(reason == QSystemTrayIcon::Trigger)
    {
        LOG("Event QSystemTrayIcon::Trigger");

        if(!infoDialog)
        {
            LOG("NULL information dialog");
            if(setupWizard)
            {
                LOG("Showing setup wizard");
                setupWizard->setVisible(true);
                setupWizard->activateWindow();
            }
            else showInfoMessage(tr("Logging in..."));

            return;
        }

        LOG("Information dialog available");
        if(!infoDialog->isVisible())
        {
            LOG("Information dialog not visible, showing it");

            //Put it in the right position (to prevent problems with changes in the taskbar or the resolution)
            QRect screenGeometry = QApplication::desktop()->availableGeometry();
            infoDialog->move(screenGeometry.right() - 400 - 2, screenGeometry.bottom() - 545 - 2);
            infoDialog->updateTransfers();
            infoDialog->updateDialog();

            //Show the dialog
            infoDialog->showMinimized();
            infoDialog->setWindowState(Qt::WindowActive);
            infoDialog->showNormal();
            infoDialog->raise();
            infoDialog->activateWindow();
            infoDialog->setFocus();
        }
        else if(!infoDialog->hasFocus())
        {
            LOG("Information dialog visible and without focus, hiding it");
            infoDialog->hide();
        }
        else
        {
            LOG("Information dialog visible with focus. Nothing to do here");
        }
    }
}

void MegaApplication::onMessageClicked()
{
    LOG("onMessageClicked");
    if(lastTrayMessage == tr("A new version of MEGAsync is available! Click on this message to install it"))
        emit installUpdate();
    else
        trayIconActivated(QSystemTrayIcon::Trigger);
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
    settingsDialog = new SettingsDialog(this, true);
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

    onSyncStateChanged(megaApi);
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
                if(megaApi->getRootNode())
                {
                    //If we have got the filesystem, start the app
                    loggedIn();
                }
                else
                {
                    QMessageBox::warning(NULL, tr("Error"), tr("Unable to get the filesystem.\n"
                                                               "Please contact bug@mega.co.nz"), QMessageBox::Ok);
                    unlink();
                }
			}
            else
            {
                LOG("Error fetching nodes");
                QMessageBox::warning(NULL, tr("Error"), e->QgetErrorString(), QMessageBox::Ok);
                unlink();
            }
		}

		break;
	}
    case MegaRequest::TYPE_ACCOUNT_DETAILS:
    {
        if(!preferences->logged()) break;

		if(e->getErrorCode() != MegaError::API_OK)
			break;

        //Account details retrieved, update the preferences and the information dialog
        mega::AccountDetails *details = request->getAccountDetails();
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
        preferences->setWasPaused(paused);
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
        onSyncStateChanged(megaApi);
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
                    else
                    {
                        showErrorMessage(e->QgetErrorString());
                    }

                    delete node;
                    delete parentNode;
                    delete rubbishNode;

                    LOG("Sync error! Removed");
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
                    preferences->removeSyncedFolder(i);
                    if(settingsDialog)
                        settingsDialog->loadSettings();
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
        if(e->getErrorCode() == MegaError::API_OK)
        {
            for(int i=preferences->getNumSyncedFolders()-1; i>=0; i--)
            {
                if((request->getNodeHandle() == preferences->getMegaFolderHandle(i)))
                {
                    if(infoDialog) infoDialog->updateSyncsButton();
                    LOG("Sync removed");
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
                    preferences->removeSyncedFolder(i);
                    onSyncStateChanged(megaApi);
                    break;
                }
            }
        }
        else
        {
            showErrorMessage(e->QgetErrorString());
            if(settingsDialog)
                settingsDialog->loadSettings();
        }
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
    if(infoDialog)
        infoDialog->setTotalTransferSize(totalDownloadSize, totalUploadSize);

    if((megaApi->getNumPendingDownloads() + megaApi->getNumPendingDownloads()) == 1)
        onSyncStateChanged(megaApi);
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
            addRecentFile(QString::fromUtf8(transfer->getFileName()), transfer->getNodeHandle(), localPath);
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
    if(infoDialog)
    {
        if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
            ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
            infoDialog->setTransfer(transfer);

        infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
        infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
        infoDialog->updateTransfers();
        infoDialog->updateDialog();
    }

    if(transfer->getType()==MegaTransfer::TYPE_DOWNLOAD)
    {
        if(lastStartedDownload == transfer->getStartTime())
            lastStartedDownload = 0;
    }
    else
    {
        if(lastStartedUpload == transfer->getStartTime())
            lastStartedUpload = 0;
    }

    if(infoDialog) infoDialog->increaseUsedStorage(transfer->getTotalBytes());
    preferences->setUsedStorage(preferences->usedStorage()+transfer->getTotalBytes());

    //If there are no pending transfers, reset the statics and update the state of the tray icon
    if(!megaApi->getNumPendingDownloads() && !megaApi->getNumPendingUploads())
    {
		totalUploadSize = totalDownloadSize = 0;
		totalUploadedSize = totalDownloadedSize = 0;
		uploadSpeed = downloadSpeed = 0;
        if(e->getErrorCode() == MegaError::API_OK)
            onSyncStateChanged(megaApi);
	}
}

//Called when a transfer has been updated
void MegaApplication::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    //Update statics
	if(transfer->getType() == MegaTransfer::TYPE_DOWNLOAD)
	{
        downloadSpeed = transfer->getSpeed();
        if(!lastStartedDownload || !transfer->getTransferredBytes())
            lastStartedDownload = transfer->getStartTime();
		totalDownloadedSize += transfer->getDeltaSize();
	}
	else
	{
        uploadSpeed = transfer->getSpeed();
        if(!lastStartedUpload || !transfer->getTransferredBytes())
            lastStartedUpload = transfer->getStartTime();
		totalUploadedSize += transfer->getDeltaSize();
	}

    //Send updated statics to the information dialog
    if(infoDialog)
    {
        if(((transfer->getType() == MegaTransfer::TYPE_DOWNLOAD) && (transfer->getStartTime()>=lastStartedDownload)) ||
            ((transfer->getType() == MegaTransfer::TYPE_UPLOAD) && (transfer->getStartTime()>=lastStartedUpload)))
            infoDialog->setTransfer(transfer);

        infoDialog->setTransferSpeeds(downloadSpeed, uploadSpeed);
        infoDialog->setTransferredSize(totalDownloadedSize, totalUploadedSize);
        infoDialog->updateTransfers();
        infoDialog->updateDialog();
    }
}

//Called when there is a temporal problem in a transfer
void MegaApplication::onTransferTemporaryError(MegaApi *, MegaTransfer *transfer, MegaError* e)
{
    //Show information to users
    if(megaApi->getNumPendingUploads() || megaApi->getNumPendingDownloads())
        showWarningMessage(tr("Temporary transmission error: ") + e->QgetErrorString(), QString::fromUtf8(transfer->getFileName()));
    else
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

        if(node->isRemoved() && (node->getType()==MegaNode::TYPE_FOLDER))
        {
            for(int i=0; i<preferences->getNumSyncedFolders(); i++)
            {
                if(preferences->getMegaFolderHandle(i) == node->getHandle())
                {
                    LOG("Remote sync deletion detected!");
                    Platform::syncFolderRemoved(preferences->getLocalFolder(i), preferences->getSyncName(i));
                    preferences->removeSyncedFolder(i);
                    if(infoDialog) infoDialog->updateSyncsButton();
                    onSyncStateChanged(megaApi);
                }
            }
        }

        if(!node->getTag() && !node->isRemoved() && !node->isSyncDeleted() && ((lastExit/1000)<node->getCreationTime()))
            externalNodes++;

        if(!node->isRemoved() && node->getTag() && !node->isSyncDeleted() && (node->getType()==MegaNode::TYPE_FILE))
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

            addRecentFile(QString::fromUtf8(node->getName()), node->getHandle(), localPath);
        }
	}

    //Update the information dialog
    if(infoDialog)
    {
        infoDialog->updateDialog();
    }

    if(externalNodes)
    {
        updateUserStats();
        showNotificationMessage(tr("You have new or updated files in your account"));
    }
}

void MegaApplication::onReloadNeeded(MegaApi* api)
{
    megaApi->fetchNodes();
}

void MegaApplication::onSyncStateChanged(MegaApi *api)
{
    if(megaApi)
    {
        indexing = megaApi->isIndexing();
        waiting = megaApi->isWaiting();
    }

    if(infoDialog)
    {
        infoDialog->updateTransfers();
        infoDialog->updateDialog();
        infoDialog->setIndexing(indexing);
        infoDialog->setWaiting(waiting);
        infoDialog->setPaused(paused);
        infoDialog->updateState();
    }

    LOG("Current state: ");
    if(paused) LOG("Paused = true");
    else LOG("Paused = false");
    if(indexing) LOG("Indexing = true");
    else LOG("Indexing = false");
    if(waiting) LOG("Waiting = true");
    else LOG("Waiting = false");

    updateTrayIcon();

    if(preferences->logged())
    {
        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
            Platform::notifyItemChange(preferences->getLocalFolder(i));
    }
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

