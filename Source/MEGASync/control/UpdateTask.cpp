#include "MegaApplication.h"
#include "UpdateTask.h"
#include "control/Utilities.h"
#include <iostream>

using namespace std;

const unsigned int UpdateTask::INITIAL_DELAY_SECS = 30;
const unsigned int UpdateTask::RETRY_INTERVAL_SECS = 7200;
const QString UpdateTask::UPDATE_CHECK_URL = QString::fromUtf8("http://www.pycusoft.com/update.txt");
const QString UpdateTask::UPDATE_FOLDER_NAME = QString::fromAscii("update");
const QString UpdateTask::BACKUP_FOLDER_NAME = QString::fromAscii("backup");
const char UpdateTask::PUBLIC_KEY[] = "CACe_GbhzspMk6X5FcO29fDNzXEL-A5Lsd4o9KIeEG0uoCDvlDmr8djRpItL3GuhsjUMzPUyf2wLqlFs9Mu3SgJ-CdyTQUQ4bEDvYjqa8yfS_cWcVM2KvYhVT8X-JpHRtVGYfycmlzKEA2klqS3n3BwJgXL8vjFYUo34bsvUEEW6Q727phbd6M-2YxaGX9FXtU4Aqq0vzEhhmunVrFlEAtVaR0LhOHXJUucoUePufupGFFLEYEb-njzD7-9x3LuMMn25s7ZHFk4L_fvuZZtalu142QLblkp_rWm0iyM0ztfVs18qBl4J9WR6hVw6W2sHsKp6sBipELf1_owpCtLeFOMFAAUR";
const char UpdateTask::PRIVATE_KEY[] = "BADeDLmPEn4YClp42o70Ek2UPZs4rBHjfR4jIFJfrR-_HoGpsG0FnU6x_A18qDpuNsVrELyWbRx1fgwfaYcuAvRDh19NOxHB2tHmZspNXO04odwaG5tvFCH3r81UdWsQ9W_PXbuqMjJxifu3PCMO1n5-hH9qt6TBZBIG9gc-2MM_VQQAt0tKXxrbUi2XsllJqaQzCZ9oxDRiXvgJn9a3QVnDXJyVXW4yEwkAtw8O6-kit5ImDAzQNq3t3MymZ_FVmY3tnyegJRPYWxH4R3wM2AbDyAl2-TuB2Z7KnKzNQBlebvwsEG-dzkc5UXbCiFUjOAwhevPV6gZEN_NQyNhqEcXFNPEH-wcDmyCM40ckoazfE-24BtvnLmYs03RH2Nwo6QjbE-Astiiv83xKq3KpJENYyINec0gYGeBYcfF0sMTOkIRSUu7459lR8C-qbEbL3PA_bu70m4qpY2vEmNaWV8s2aFDH_9XJX9hKb2e02zoh0RWF8i2YiNZxzXiaOvgjAXaIP0-_J23Orvi7CD4Mgto6q7CGFtRleLg00xW5yT6HLcZ2GwNCIMEOS6NtVJ9oAa2FQSOjr1ZrNlGR0d-zpnl8O7R_yprTPzvj1P2co3FzdiU-2YGJrHMFVqrqY8PhM3hEABzi60QWr8ZgbMVfX2HP_dR1cTiTECxBwOFZsbiz8uw3IwED_RM8DUHFrMAnZ9YP_XJSS2e80A6p1ZJhk7fB6mCD9F7rGHEo8PAKpx395hZ-1E4XPLk2Ts4GDqFYXeK2iC0bfatpvxZVl6B3kZswSTQ8Yz0upFKlr1_oSnLvvHo9PEMb0dfDXn6FMf4nwFLltD42Y5deU-g18ivAKbx6duLugUTI";

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

    QString basePath = MegaApplication::applicationDirPath() + QDir::separator();
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
            Utilities::removeRecursively(QDir(appFolder.absoluteFilePath(subdirs[i])));
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
    Utilities::removeRecursively(QDir(updateFolder));
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
