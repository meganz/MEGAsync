#include "UpdateTask.h"
#include "utils/Utils.h"
#include <iostream>

using namespace std;

const unsigned int UpdateTask::INITIAL_DELAY_SECS = 300;
const unsigned int UpdateTask::RETRY_INTERVAL_SECS = 7200;
const QString UpdateTask::UPDATE_CHECK_URL = QString::fromUtf8("http://www.pycusoft.com/update.txt");
const QString UpdateTask::UPDATE_FOLDER_NAME = QString::fromAscii("update");
const QString UpdateTask::BACKUP_FOLDER_NAME = QString::fromAscii("backup");
const char UpdateTask::PUBLIC_KEY[] = "CADfKLVtM_wXRWS5cbw3Mo6CFWg0KaxK38wqTt6dCOrJS35PGhAAFk5_QJ4EHqa4Rh93m3akUBnbo9d1Jup0JepGSUsjGtWoE1p0yV_q84BDCSyRLW1kBD6BCetA54lZI98P_vFpE7v_zVpCWKLmvOC8fOdjqXHdMunPO1ytrbFICb7WXpovVGPNWBGbdPxRbcnupuBEEsF9Bf7fjFKSrg8q7ga3LXyovVL5KKTWQeqyw8DeAUyTzeMa-1t8C1wtqVaEzvCTZRIlD9eSPf_5w0Dhk6WI9GtU4FiTb1bwBdOplZw3cFInNA_xiG_faAZ1BNtuvIHuuvaxYNreC-wzIncfAAUR";
const char UpdateTask::PRIVATE_KEY[] = "BADx27mvzbBv2Tld-sNgeK7QEWCZq8GOtZKWJBPXqshnJdWVQsUuHesh5EZkQfwatuVZCVv4uGKqdae_BGhgXOS7UTlKW9QtbfsYSPMrpc35cWvujS0kfft6jCqa06LZ081lI45yIKadxHG6UeyZE64StJYAruE1F0QGDLnkff-a9QQA7DUU6WtpFPsNQbpxCFzDtLgIIUJ2GCeP_u9f_nd29nLO8z3_45hcCODA1cnt0IZIIVSli12AdChLWgUNajMeX9LamPGrI-eeweZuEjhtzmsC1fNe-rsj4eZMtRoO_oROX-REgDeIZtbgBhYKP_NzStESzlzVHcvRlx0zc5DJpUMH-waQQZJFSy3cZNhHHB-9uOW1VeNqoy9g72qn930W2bqgVonbHpaXPotFqktqUDKYppKToZtrxIZ1wpKBJQNqhuPzGMxTnN9L3QNvVaUHKWtiJvU2Cr8tTSHptBh_R83qd4B4cIMXKy0rr9TGXyTncAWLM_tm3bOurIYYVYyjOeuHwtiiWmGYUdomKP8-InSWcvmQ4vr-6jHhCqQe1nvHBTkCKVv0AyU_teNqZQXzh4N1XG0tK4s7nF4HxbgTdTlXtnPJRCez7xUpHgpv55dl3emQJLx7icSUvB8WWo8glnNWaDYbN4YWvJGYMlZWJvuz3g5idjPafUnVCVgfBFtsCSUEALTpaVh3_wyMjv2U6D_F0vKNHtjuKlRakW20rf87zPiAfR0yIZipDo5hYydsmt3pqkSrrEZSf_Pmssb_lB3zFdpbNOxKm4hdkN2iruoNzFml0VcVpllBbYcJezdpydB_loYhN2yabfgcuxziGluGJfmvXOlJv2cgmhTq1YZVQW-M";

UpdateTask::UpdateTask(QObject *parent) :
    QObject(parent)
{
    m_WebCtrl = NULL;
    signatureChecker = NULL;
    signatureGenerator = NULL;

    //KEY GENERATION
/*  CryptoPP::Integer pubk[AsymmCipher::PUBKEY];
    string pubks;
    string privks;
    asymkey.genkeypair(asymkey.key,pubk,2048);
    AsymmCipher::serializeintarray(pubk,AsymmCipher::PUBKEY,&pubks);
    AsymmCipher::serializeintarray(asymkey.key,AsymmCipher::PRIVKEY,&privks);

    int len = pubks.size();
    char* buf = new char[len*4/3+4];
    Base64::btoa((const byte *)pubks.data(),len,buf);
    cout << "PUBK: " << buf << endl;
    delete[] buf;

    len = privks.size();
    buf = new char[len*4/3+4];
    Base64::btoa((const byte *)privks.data(),len,buf);
    cout << "PRIVK: " << buf << endl;
    delete[] buf;
*/
}

UpdateTask::~UpdateTask()
{
    delete m_WebCtrl;
    delete signatureChecker;
    delete signatureGenerator;
}

void UpdateTask::doWork()
{
    if(m_WebCtrl) return;

    int len;

    QString basePath = QCoreApplication::applicationDirPath() + QDir::separator();
    appFolder = QDir(basePath);
    updateFolder = QDir(basePath + UPDATE_FOLDER_NAME);
    m_WebCtrl = new QNetworkAccessManager();
    connect(m_WebCtrl, SIGNAL(finished(QNetworkReply*)), this, SLOT(downloadFinished(QNetworkReply*)));

    len = sizeof(PRIVATE_KEY)/4*3+3;
    string privks;
    privks.resize(len);
    privks.resize(Base64::atob(PRIVATE_KEY, (byte *)privks.data(), len));
    privkey.setkey(AsymmCipher::PRIVKEY,(byte*)privks.data(), privks.size());

    len = sizeof(PUBLIC_KEY)/4*3+3;
    string pubks;
    pubks.resize(len);
    pubks.resize(Base64::atob(PUBLIC_KEY, (byte *)pubks.data(), len));
    asymkey.setkey(AsymmCipher::PUBKEY,(byte*)pubks.data(), pubks.size());

    signatureChecker = new HashSignature(new Hash());
    signatureGenerator = new HashSignature(new Hash());

    signatureChecker->add((byte *)"TestingHashSignature", 20);
    signatureChecker->get(&privkey, (byte *)signature, sizeof(signature));

    signatureGenerator->add((byte *)"TestingHashSignature", 20);
    if(signatureGenerator->check(&asymkey, (byte *)signature, sizeof(signature))) cout << "HASHSIGNATURE TEST OK" << endl;
    else cout << "HASHSIGNATURE TEST FAILED" << endl;


    QTimer::singleShot(INITIAL_DELAY_SECS*1000, this, SLOT(tryUpdate()));
}

void UpdateTask::tryUpdate()
{
    cout << "tryUpdate" << endl;
    initialCleanup();
    downloadFile(UPDATE_CHECK_URL);
}

void UpdateTask::initialCleanup()
{
    //Delete previous backups if possible (they could be still in use)
    QStringList subdirs = appFolder.entryList(QDir::Dirs);
    for(int i=0; i<subdirs.size(); i++)
    {
        if(subdirs[i].startsWith(BACKUP_FOLDER_NAME))
            Utils::removeRecursively(QDir(appFolder.absoluteFilePath(subdirs[i])));
    }

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

    //Remove the update folder
    Utils::removeRecursively(QDir(updateFolder));
    emit updateCompleted();
}

void UpdateTask::postponeUpdate()
{
    cout << "postponeUpdate" << endl;
    QTimer::singleShot(RETRY_INTERVAL_SECS*1000, this, SLOT(tryUpdate()));
}

void UpdateTask::downloadFile(QString url)
{
    cout << "downloadFile " << url.toStdString() << endl;
    QNetworkRequest request(url);
    m_WebCtrl->get(request);
}

QString UpdateTask::readNextLine(QNetworkReply *reply)
{
    char line[512];
    int len = reply->readLine(line, sizeof(line));
    if((len <= 0) || (len >= (sizeof(line)-1)))
        return QString();

    QString qLine(QString::fromUtf8(line));
    return qLine.trimmed();
}

bool UpdateTask::processUpdateFile(QNetworkReply *reply)
{
    cout << "Parsing update file" << endl;
    QString version = readNextLine(reply);
    if(!version.size())
    {
        cout << "Invalid update file (start)" << endl;
        return false;
    }

    updateVersion = version.toInt();
    int currentVersion = QApplication::applicationVersion().toInt();
    if(updateVersion <= currentVersion)
    {
        cout << "Update not needed. Current version: " << currentVersion << "  Update version: " << updateVersion << endl;
        return false;
    }

    cout << "Update available! Current version: " << currentVersion << "  Update version: " << updateVersion << endl;

    QString updateSignature = readNextLine(reply);
    if(!updateSignature.size())
    {
        cout << "Invalid update file (update signature 2)" << endl;
        return false;
    }

    //cout << "Update signature: " << updateSignature.toStdString() << endl;
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
            cout << "File already installed: " << localPath.toStdString() << endl;
            continue;
        }
        downloadURLs.append(url);
        localPaths.append(localPath);
        fileSignatures.append(fileSignature);
    }

    if(!downloadURLs.size())
    {
        cout << "No new files" << endl;
        return false;
    }

    return checkSignature(updateSignature);
}

bool UpdateTask::processFile(QNetworkReply *reply)
{
    cout << "processFile" << endl;
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
        cout << "Error opening file" << endl;
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
            cout << "Error writting file" << endl;
            localFile.close();
            return false;
        }
        remainingSize -= written;
        position += written;
    }


    //Save the new file
    if(!localFile.flush())
    {
        cout << "Error flushing file" << endl;
        localFile.close();
        return false;
    }
    localFile.close();

    return true;
}

bool UpdateTask::performUpdate()
{
    cout << "performUpdate" << endl;

    //Create backup folder
    backupFolder = QDir(appFolder.absoluteFilePath(BACKUP_FOLDER_NAME + QDateTime::currentDateTime().toString(QString::fromAscii("_dd_MM_yy__hh_mm_ss"))));
    backupFolder.mkdir(QString::fromAscii("."));

    for(int i=0; i<localPaths.size(); i++)
    {
        QString file = localPaths[i];
        appFolder.rename(file, backupFolder.absoluteFilePath(file));
        if(!updateFolder.rename(file, appFolder.absoluteFilePath(file)))
        {
            cout << "Error installing the file " << file.toStdString() << " in " << appFolder.absoluteFilePath(file).toStdString() << endl;
            rollbackUpdate(i);
            return false;
        }

        cout << "File installed: " << file.toStdString() << endl;
    }

    cout << "Update installed!!" << endl;
    return true;
}

void UpdateTask::rollbackUpdate(int fileNum)
{
    cout << "rollbackUpdate" << endl;
    for(int i=fileNum; i>=0; i--)
    {
        QString file = localPaths[i];
        appFolder.rename(file, updateFolder.absoluteFilePath(file));
        backupFolder.rename(file, appFolder.absoluteFilePath(file));
        cout << "File restored: " << file.toStdString() << endl;
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
    signatureGenerator->add((const byte *)bytes.constData(), bytes.length());
}

void UpdateTask::initSignature()
{
    signatureChecker->get(&asymkey, NULL, 0);
    signatureGenerator->get(&asymkey, NULL, 0);
}

bool UpdateTask::checkSignature(QString value)
{
    /*signatureGenerator->get(&privkey, (byte *)signature, sizeof(signature));
    int len = (sizeof(signature)*4)/3+4;
    char *buf = new char[len];
    Base64::btoa((const byte *)signature,sizeof(signature),buf);
    cout << "SIGNATURE SHOULD BE: " << buf << endl;
    delete buf;
*/

  //  cout << "Checking signature: " << value.toStdString() << endl;
    int l = Base64::atob(value.toAscii().constData(), (byte *)signature, sizeof(signature));
    if(l != sizeof(signature))
    {
        cout << "Invalid signature size: " << l << endl;
        return false;
    }

    int result = signatureChecker->check(&asymkey, (const byte *)signature, sizeof(signature));
    if(result) cout << "Valid signature" << endl;
    else  cout << "Invalid signature" << endl;

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
    HashSignature tmpHash(new Hash());
    char tmpSignature[256];
    if(Base64::atob(fileSignature.toAscii().constData(), (byte *)tmpSignature, sizeof(tmpSignature)) != sizeof(tmpSignature))
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
    reply->deleteLater();

    //Check if the request has been successful
    QVariant statusCode = reply->attribute( QNetworkRequest::HttpStatusCodeAttribute );
    if (!statusCode.isValid() || (statusCode.toInt() != 200) || (reply->error() != QNetworkReply::NoError))
    {
        cout << "Invalid status code" << endl;
        postponeUpdate();
        return;
    }

    //Process the received data
    if(currentFile <  0)
    {
        //Process the update file
        if(!processUpdateFile(reply))
        {
            cout << "Update not needed or invalid update file" << endl;
            postponeUpdate();
            return;
        }
    }
    else
    {
        //Process the file
        if(!processFile(reply))
        {
            cout << "Update failed processing file: " << downloadURLs[currentFile].toStdString() << endl;
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

        cout << "File already downloaded: " << localPaths[currentFile].toStdString() << endl;
        currentFile++;
    }

    //All files have been processed. Apply update
    if(!performUpdate())
    {
        postponeUpdate();
        return;
    }

    finalCleanup();
}
