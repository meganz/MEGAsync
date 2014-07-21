#include "MegaApplication.h"
#include "UpdateTask.h"
#include "control/Utilities.h"
#include <iostream>
#include <QAuthenticator>

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
    cout << "UPDATEFOLDER " << updateFolder.absolutePath().toStdString() << endl;
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));
    connect(m_WebCtrl, SIGNAL(proxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)), this, SLOT(onProxyAuthenticationRequired(const QNetworkProxy&, QAuthenticator*)));

    int len = strlen(Preferences::UPDATE_PUBLIC_KEY)/4*3+3;
    string pubks;
    pubks.resize(len);
    pubks.resize(mega::Base64::atob(Preferences::UPDATE_PUBLIC_KEY, (byte *)pubks.data(), len));
    asymkey.setkey(mega::AsymmCipher::PUBKEY,(byte*)pubks.data(), pubks.size());

    signatureChecker = new mega::HashSignature(new mega::Hash());
    preferences = Preferences::instance();

    updateTimer->start(Preferences::UPDATE_RETRY_INTERVAL_SECS*1000);
    QTimer::singleShot(Preferences::UPDATE_INITIAL_DELAY_SECS*1000, this, SLOT(tryUpdate()));
}

void UpdateTask::tryUpdate()
{
    LOG("tryUpdate");
    if(running) return;

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
            Utilities::removeRecursively(QDir(appFolder.absoluteFilePath(subdirs[i])));
    }

    //New location (data folder)
    QDir basePathDir(basePath);
    subdirs = basePathDir.entryList(QDir::Dirs);
    for(int i=0; i<subdirs.size(); i++)
    {
        if(subdirs[i].startsWith(Preferences::UPDATE_BACKUP_FOLDER_NAME))
            Utilities::removeRecursively(QDir(basePathDir.absoluteFilePath(subdirs[i])));
    }

    //Remove update folder (old location)
    QDir oldUpdateFolder(MegaApplication::applicationDirPath() +
                           QDir::separator() +
                           Preferences::UPDATE_FOLDER_NAME);
    Utilities::removeRecursively(oldUpdateFolder);

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
    Utilities::removeRecursively(QDir(updateFolder));

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
    LOG("postponeUpdate");
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
    LOG(QString::fromAscii("downloadFile ") + url);
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
    LOG("Parsing update file");
    QString version = readNextLine(reply);
    if(!version.size())
    {
        LOG("Invalid update file (start)");
        return false;
    }

    updateVersion = version.toInt();
    int currentVersion = QApplication::applicationVersion().toInt();
    if(updateVersion <= currentVersion)
    {
        LOG(QString::fromAscii("Update not needed. Current version: ") + QString::number(currentVersion) + QString::fromAscii("  Update version: ") + QString::number(updateVersion));
        return false;
    }

    LOG(QString::fromAscii("Update available! Current version: ") + QString::number(currentVersion) + QString::fromAscii("  Update version: ") + QString::number(updateVersion));

    QString updateSignature = readNextLine(reply);
    if(!updateSignature.size())
    {
        LOG("Invalid update file (empty signature)");
        return false;
    }

    initSignature();
    addToSignature(version);

    while(true)
    {
        QString url = readNextLine(reply);
        if(!url.size()) break;

        QString localPath = readNextLine(reply);
        if(!localPath.size()) return false;

        QString fileSignature = readNextLine(reply);
        if(!fileSignature.size()) return false;

        addToSignature(url);
        addToSignature(localPath);
        addToSignature(fileSignature);

        if(alreadyInstalled(localPath, fileSignature))
        {
            LOG(QString::fromAscii("File already installed: ") + localPath);
            continue;
        }
        downloadURLs.append(url);
        localPaths.append(localPath);
        fileSignatures.append(fileSignature);
    }

    if(!downloadURLs.size())
    {
        LOG("No new files");
        return false;
    }

    return checkSignature(updateSignature);
}

bool UpdateTask::processFile(QNetworkReply *reply)
{
    LOG("processFile");
    QByteArray data = reply->readAll();

    //Check signature
    initSignature();
    addToSignature(data);
    if(!checkSignature(fileSignatures[currentFile]))
        return false;

    //Create the folder for the new file
    QFile localFile(updateFolder.absoluteFilePath(localPaths[currentFile]));
    QFileInfo info(localFile);
    info.absoluteDir().mkpath(QString::fromAscii("."));

    //Delete the file if it exists.
    localFile.remove();

    //Open the new file
    if (!localFile.open(QIODevice::WriteOnly))
    {
        LOG("Error opening file");
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
            LOG("Error writting file");
            localFile.close();
            return false;
        }
        remainingSize -= written;
        position += written;
    }


    //Save the new file
    if(!localFile.flush())
    {
        LOG("Error flushing file");
        localFile.close();
        return false;
    }
    localFile.close();

    return true;
}

bool UpdateTask::performUpdate()
{
    LOG("performUpdate");

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
            LOG(QString::fromAscii("Error installing the file ") + file + QString::fromAscii(" in ") + appFolder.absoluteFilePath(file));
            rollbackUpdate(i);
            return false;
        }

        LOG(QString::fromAscii("File installed: ") + file);
    }

    LOG("Update installed!!");
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    LOG("rollbackUpdate");
    for(int i=fileNum; i>=0; i--)
    {
        QString file = localPaths[i];
        appFolder.rename(file, updateFolder.absoluteFilePath(file));
        backupFolder.rename(file, appFolder.absoluteFilePath(file));
        LOG(QString::fromAscii("File restored: ") + file);
    }
}

void UpdateTask::addToSignature(QString value)
{
    QByteArray bytes = value.toAscii();
    addToSignature(bytes);
}

void UpdateTask::addToSignature(QByteArray bytes)
{
    signatureChecker->add((const byte *)bytes.constData(), bytes.length());
}

void UpdateTask::initSignature()
{
    signatureChecker->get(&asymkey, NULL, 0);
}

bool UpdateTask::checkSignature(QString value)
{
    int l = mega::Base64::atob(value.toAscii().constData(), (byte *)signature, sizeof(signature));
    if(l != sizeof(signature))
    {
        LOG(QString::fromAscii("Invalid signature size: ") + QString::number(l));
        return false;
    }

    int result = signatureChecker->check(&asymkey, (const byte *)signature, sizeof(signature));
    if(result) LOG("Valid signature");
    else  LOG("Invalid signature");

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
    mega::HashSignature tmpHash(new mega::Hash());
    char tmpSignature[512];
    if(mega::Base64::atob(fileSignature.toAscii().constData(), (byte *)tmpSignature, sizeof(tmpSignature)) != sizeof(tmpSignature))
        return false;

    QFile file(absolutePath);
    if(!file.open(QIODevice::ReadOnly))
        return false;

    QByteArray bytes = file.readAll();
    tmpHash.add((byte *)bytes.constData(), bytes.size());
    file.close();

    return tmpHash.check(&asymkey, (const byte *)tmpSignature, sizeof(tmpSignature));
}

void UpdateTask::downloadFinished(QNetworkReply *reply)
{
    timeoutTimer->stop();
    reply->deleteLater();

    //Check if the request has been successful
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        LOG("Invalid status code");
        postponeUpdate();
        return;
    }

    //Process the received data
    if(currentFile <  0)
    {
        //Process the update file
        if(!processUpdateFile(reply))
        {
            LOG("Update not needed or invalid update file");
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
            LOG(QString::fromAscii("Update failed processing file: ") + downloadURLs[currentFile]);
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
            downloadFile(downloadURLs[currentFile]);
            return;
        }

        LOG(QString::fromAscii("File already downloaded: ") + localPaths[currentFile]);
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
        LOG("Update ready");
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
