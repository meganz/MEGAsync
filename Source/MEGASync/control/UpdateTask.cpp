#include "MegaApplication.h"
#include "UpdateTask.h"
#include "control/Utilities.h"
#include <iostream>
#include <QAuthenticator>

using namespace mega;
using namespace std;

UpdateTask::UpdateTask(QObject *parent) :
    QObject(parent)
{
    m_WebCtrl = NULL;
    signatureChecker = NULL;
    forceInstall = false;
    running = false;
    forceCheck = false;
    updateTimer = NULL;
    timeoutTimer = NULL;
}

UpdateTask::~UpdateTask()
{
    delete m_WebCtrl;
    delete signatureChecker;
    delete updateTimer;
    delete timeoutTimer;
}

void UpdateTask::installUpdate()
{
    forceInstall = true;
    tryUpdate();
}

void UpdateTask::checkForUpdates()
{
    forceCheck = true;
    tryUpdate();
}

void UpdateTask::startUpdateThread()
{
    if(m_WebCtrl) return;

    updateTimer = new QTimer();
    updateTimer->setSingleShot(false);
    connect(updateTimer, SIGNAL(timeout()), this, SLOT(tryUpdate()));
    timeoutTimer = new QTimer();
    timeoutTimer->setSingleShot(true);
    connect(timeoutTimer, SIGNAL(timeout()), this, SLOT(onTimeout()));

    //Set the working directory
#if QT_VERSION < 0x050000
    basePath = QDesktopServices::storageLocation(QDesktopServices::DataLocation);
#else
    basePath = QStandardPaths::standardLocations(QStandardPaths::DataLocation)[0];
#endif

    appFolder = QDir(MegaApplication::applicationDirPath() + QDir::separator());

    #ifdef __APPLE__
        appFolder.cdUp();
        appFolder.cdUp();
    #endif

    updateFolder = QDir(basePath + QDir::separator() + Preferences::UPDATE_FOLDER_NAME);
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    connect(m_WebCtrl, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    signatureChecker = new MegaHashSignature((const char *)Preferences::UPDATE_PUBLIC_KEY);
    preferences = Preferences::instance();

    updateTimer->start(Preferences::UPDATE_RETRY_INTERVAL_SECS*1000);
    QTimer::singleShot(Preferences::UPDATE_INITIAL_DELAY_SECS*1000, this, SLOT(tryUpdate()));
}

void UpdateTask::tryUpdate()
{
    if(running) return;

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Starting update check");
    running = true;
    initialCleanup();

    QString randomSequence = QString::fromUtf8("?");
    for(int i=0;i<10;i++)
        randomSequence += QChar::fromAscii('A'+(rand() % 26));
    downloadFile(Preferences::UPDATE_CHECK_URL + randomSequence);
}

void UpdateTask::onTimeout()
{
    timeoutTimer->stop();
    delete m_WebCtrl;
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    connect(m_WebCtrl, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    postponeUpdate();
}

void UpdateTask::initialCleanup()
{
    //Delete previous backups if possible (they could be still in use)
    //Old location (app folder)
    QStringList subdirs = appFolder.entryList(QDir::Dirs);
    for(int i=0; i<subdirs.size(); i++)
    {
        if(subdirs[i].startsWith(Preferences::UPDATE_BACKUP_FOLDER_NAME))
            Utilities::removeRecursively(appFolder.absoluteFilePath(subdirs[i]));
    }

    //New location (data folder)
    QDir basePathDir(basePath);
    subdirs = basePathDir.entryList(QDir::Dirs);
    for(int i=0; i<subdirs.size(); i++)
    {
        if(subdirs[i].startsWith(Preferences::UPDATE_BACKUP_FOLDER_NAME))
            Utilities::removeRecursively(basePathDir.absoluteFilePath(subdirs[i]));
    }

    //Remove update folder (old location)
    Utilities::removeRecursively(MegaApplication::applicationDirPath() +
                                 QDir::separator() +
                                 Preferences::UPDATE_FOLDER_NAME);

    //Initialize update info
    downloadURLs.clear();
    localPaths.clear();
    fileSignatures.clear();
    currentFile = -1;
}

//Called after a successful update
void UpdateTask::finalCleanup()
{
    //Change the version info to skip checking the same update again.
    QApplication::setApplicationVersion(QString::number(updateVersion));

    //Remove the update folder (new location)
    Utilities::removeRecursively(updateFolder.absolutePath());

    #ifdef __APPLE__
        QFile exeFile(MegaApplication::applicationFilePath());
        exeFile.setPermissions(QFile::ExeOwner | QFile::ReadOwner | QFile::WriteOwner |
                                  QFile::ExeGroup | QFile::ReadGroup |
                                  QFile::ExeOther | QFile::ReadOther);
    #endif

    emit updateCompleted();
}

void UpdateTask::postponeUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update task finished. No updates available");

    if(forceInstall)
        emit updateError();
    else
        emit updateNotFound(forceCheck);

    forceInstall = false;
    running = false;
    forceCheck = false;
}

void UpdateTask::downloadFile(QString url)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Downloading updated file from %1").arg(url).toUtf8().constData());

    QNetworkRequest request(url);
    request.setAttribute(QNetworkRequest::CacheLoadControlAttribute,
                         QVariant( int(QNetworkRequest::AlwaysNetwork)));
    m_WebCtrl->get(request);
    timeoutTimer->start(Preferences::UPDATE_TIMEOUT_SECS*1000);
}

QString UpdateTask::readNextLine(QNetworkReply *reply)
{
    char line[4096];
    int len = reply->readLine(line, sizeof(line));
    if((len <= 0) || ((unsigned int)(len-1) >= sizeof(line)))
        return QString();

    QString qLine(QString::fromUtf8(line));
    return qLine.trimmed();
}

bool UpdateTask::processUpdateFile(QNetworkReply *reply)
{
    MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, "Reading update info");

    QString version = readNextLine(reply);
    if(!version.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "Invalid update info");
        return false;
    }

    updateVersion = version.toInt();
    int currentVersion = QApplication::applicationVersion().toInt();
    if(updateVersion <= currentVersion)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,
                     QString::fromUtf8("Update not needed. Current version: %1  Update version: %2")
                     .arg(currentVersion).arg(updateVersion).toUtf8().constData());
        return false;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("Update available! Current version: %1  Update version: %2")
                 .arg(currentVersion).arg(updateVersion).toUtf8().constData());

    QString updateSignature = readNextLine(reply);
    if(!updateSignature.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty info signature)");
        return false;
    }

    initSignature();
    addToSignature(version);

    while(true)
    {
        QString url = readNextLine(reply);
        if(!url.size()) break;

        QString localPath = readNextLine(reply);
        if(!localPath.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty path)");
            return false;
        }

        QString fileSignature = readNextLine(reply);
        if(!fileSignature.size())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (empty file signature)");
            return false;
        }

        addToSignature(url);
        addToSignature(localPath);
        addToSignature(fileSignature);

        if(alreadyInstalled(localPath, fileSignature))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("File already installed: %1").arg(localPath).toUtf8().constData());
            continue;
        }
        downloadURLs.append(url);
        localPaths.append(localPath);
        fileSignatures.append(fileSignature);
    }

    if(!downloadURLs.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, "All files are up to date");
        return false;
    }

    if(!checkSignature(updateSignature))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR,"Invalid update info (invalid signature)");
        return false;
    }

    return true;
}

bool UpdateTask::processFile(QNetworkReply *reply)
{
    QByteArray data = reply->readAll();

    //Check signature
    initSignature();
    addToSignature(data);
    if(!checkSignature(fileSignatures[currentFile]))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Invalid or corrupt file: %1")
                     .arg(updateFolder.absoluteFilePath(localPaths[currentFile])).toUtf8().constData());
        return false;
    }

    //Create the folder for the new file
    QFile localFile(updateFolder.absoluteFilePath(localPaths[currentFile]));
    QFileInfo info(localFile);
    info.absoluteDir().mkpath(QString::fromAscii("."));

    //Delete the file if it exists.
    localFile.remove();

    //Open the new file
    if (!localFile.open(QIODevice::WriteOnly))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error opening local file from writting: %1").arg(info.absoluteFilePath()).toUtf8().constData());
        return false;
    }

    //Write the new file
    int remainingSize = data.size();
    int position = 0;
    while(remainingSize)
    {
        int written = localFile.write(data.constData()+position, remainingSize);
        if(written == -1)
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error writting file: %1").arg(info.absoluteFilePath()).toUtf8().constData());
            localFile.close();
            return false;
        }
        remainingSize -= written;
        position += written;
    }


    //Save the new file
    if(!localFile.flush())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error flushing file: %1").arg(info.absoluteFilePath()).toUtf8().constData());
        localFile.close();
        return false;
    }
    localFile.close();

    return true;
}

bool UpdateTask::performUpdate()
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Applying update...");

    //Create backup folder
    QDir basePathDir(basePath);
    backupFolder = QDir(basePathDir.absoluteFilePath(Preferences::UPDATE_BACKUP_FOLDER_NAME + QDateTime::currentDateTime().toString(QString::fromAscii("_dd_MM_yy__hh_mm_ss"))));
    backupFolder.mkdir(QString::fromAscii("."));

    for(int i=0; i<localPaths.size(); i++)
    {
        QString file = localPaths[i];

        QFileInfo bakInfo(backupFolder.absoluteFilePath(file));
        QDir bakDir = bakInfo.dir();
        bakDir.mkpath(QString::fromUtf8("."));

        QFileInfo dstInfo(appFolder.absoluteFilePath(file));
        QDir dstDir = dstInfo.dir();
        dstDir.mkpath(QString::fromUtf8("."));

        appFolder.rename(file, backupFolder.absoluteFilePath(file));
        if(!updateFolder.rename(file, appFolder.absoluteFilePath(file)))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Error installing file: %1 in %2")
                        .arg(file).arg(appFolder.absoluteFilePath(file)).toUtf8().constData());
            rollbackUpdate(i);
            return false;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromUtf8("File installed: %1").arg(file).toUtf8().constData());
    }

    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update successfully installed");
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Uninstalling update...");
    for(int i=fileNum; i>=0; i--)
    {
        QString file = localPaths[i];
        appFolder.rename(file, updateFolder.absoluteFilePath(file));
        backupFolder.rename(file, appFolder.absoluteFilePath(file));
        MegaApi::log(MegaApi::LOG_LEVEL_INFO,QString::fromUtf8("File restored: %1").arg(file).toUtf8().constData());
    }
    MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update uninstalled");
}

void UpdateTask::addToSignature(QString value)
{
    QByteArray bytes = value.toAscii();
    addToSignature(bytes);
}

void UpdateTask::addToSignature(QByteArray bytes)
{
    signatureChecker->add(bytes.constData(), bytes.length());
}

void UpdateTask::initSignature()
{
    signatureChecker->init();
}

bool UpdateTask::checkSignature(QString value)
{
    int result = signatureChecker->check(value.toAscii().constData());
    if(!result)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Invalid signature");
    }

    return result;
}

bool UpdateTask::alreadyInstalled(QString relativePath, QString fileSignature)
{
    return alreadyExists(appFolder.absoluteFilePath(relativePath), fileSignature);
}

bool UpdateTask::alreadyDownloaded(QString relativePath, QString fileSignature)
{
    return alreadyExists(updateFolder.absoluteFilePath(relativePath), fileSignature);
}

bool UpdateTask::alreadyExists(QString absolutePath, QString fileSignature)
{
    MegaHashSignature tmpHash((const char *)Preferences::UPDATE_PUBLIC_KEY);
    QFile file(absolutePath);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray bytes = file.readAll();
    tmpHash.add(bytes.constData(), bytes.size());
    file.close();

    return tmpHash.check(fileSignature.toAscii().constData());
}

void UpdateTask::downloadFinished(QNetworkReply *reply)
{
    timeoutTimer->stop();
    reply->deleteLater();

    //Check if the request has been successful
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Unable to download file");
        postponeUpdate();
        return;
    }

    //Process the received data
    if(currentFile <  0)
    {
        //Process the update file
        if(!processUpdateFile(reply))
        {
            postponeUpdate();
            return;
        }
        emit installingUpdate(forceCheck);
    }
    else
    {
        //Process the file
        if(!processFile(reply))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Update failed processing file: %1")
                         .arg(downloadURLs[currentFile]).toUtf8().constData());
            postponeUpdate();
            return;
        }
    }

    //File processed. Download the next file
    currentFile++;
    while(currentFile < downloadURLs.size())
    {
        if(!alreadyDownloaded(localPaths[currentFile], fileSignatures[currentFile]))
        {
            MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("Downloading file: %1").arg(downloadURLs[currentFile]).toUtf8().constData());
            downloadFile(downloadURLs[currentFile]);
            return;
        }

        MegaApi::log(MegaApi::LOG_LEVEL_INFO, QString::fromAscii("File already downloaded: %1").arg(localPaths[currentFile]).toUtf8().constData());
        currentFile++;
    }

    //All files have been processed. Apply update
    if(preferences->updateAutomatically() || forceInstall)
    {
        if(!performUpdate())
        {
            postponeUpdate();
            return;
        }

        finalCleanup();
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Update installed");
        emit updateAvailable(forceCheck);
        preferences->setLastUpdateTime(QDateTime::currentMSecsSinceEpoch());
        preferences->setLastUpdateVersion(updateVersion);
    }
    forceInstall = false;
    forceCheck = false;
    running = false;
}

void UpdateTask::onProxyAuthenticationRequired(const QNetworkProxy &, QAuthenticator *auth)
{
    auth->setUser(preferences->getProxyUsername());
    auth->setPassword(preferences->getProxyPassword());
}
