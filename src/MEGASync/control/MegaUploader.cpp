#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
#include "MegaApplication.h"
#include <QMessageBox>
#include <QtCore>
#include <QApplication>
#include <QPointer>
#include <QFile>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

#ifndef WIN32
#include <utime.h>
#endif

using namespace mega;
using namespace std;

const char* FILE_PATH = "FILE_PATH";

MegaUploader::MegaUploader(MegaApi *megaApi)
{
    this->megaApi = megaApi;
}

MegaUploader::~MegaUploader()
{
}

void MegaUploader::upload(QString path, const QString& nodeName, MegaNode *parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch>& transferBatch)
{
    upload(QFileInfo(path), nodeName, parent, appDataID, transferBatch);
}

bool MegaUploader::filesdiffer(QFileInfo &source, QFileInfo &destination)
{
    if ( source.size() != destination.size()
            || source.lastModified().toTime_t() != destination.lastModified().toTime_t())
    {
        return true;
    }

    QCryptographicHash hashsrc( QCryptographicHash::Sha1 );
    QFile filesrc( source.absoluteFilePath() );
    if ( filesrc.open( QIODevice::ReadOnly ) ) {
        hashsrc.addData( filesrc.readAll() );
    } else {
        return true;
    }

    QCryptographicHash hashdst( QCryptographicHash::Sha1 );
    QFile filedst( destination.absoluteFilePath() );
    if ( filedst.open( QIODevice::ReadOnly ) ) {
        hashdst.addData( filedst.readAll() );
    } else {
        return true;
    }

    if (hashdst.result() != hashsrc.result())
    {
        return true;
    }
    return false;
}

/**
 * @brief MegaUploader::uploadRecursivelyIntoASyncedLocation
 * @param srcFileInfo local path to be copied uploaded
 * @param destPath corresponding local synced path where srcFileInfo would end
 * @param parent node parent that will hold the uploaded file/folder
 * @param appDataID
 * @return false if something failed
 */
void MegaUploader::uploadRecursivelyIntoASyncedLocation(QFileInfo srcFileInfo, QString destPath, MegaNode *parent, unsigned long long appDataID)
{
    if (!srcFileInfo.exists())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Recursive upload failed: source file non existing: %1").arg(srcFileInfo.absoluteFilePath()).toUtf8().constData());
        return;
    }

    QString srcPath = QDir::toNativeSeparators(srcFileInfo.absoluteFilePath());

    if (!srcPath.size() || !destPath.size())
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Recursive upload failed: invalid parameters: %1 , %2").arg(srcPath).arg(destPath).toUtf8().constData());
        return;
    }

    if (srcFileInfo.isSymLink() || (!srcFileInfo.isFile() && !srcFileInfo.isDir()) ) //review if ever symlinks are supported
    {
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Recursive upload skipping non file/folder: %1").arg(srcPath).toUtf8().constData());
        return;
    }

    if (!megaApi->isSyncable(destPath.toUtf8().constData(), srcFileInfo.size())) //if not syncable, do not copy locally, but simply upload: we don't want to create it localle nor override current files/folders
    {
        //start upload to parent
        MegaApi::log(MegaApi::LOG_LEVEL_DEBUG, QString::fromUtf8("Recursive upload uploading non syncable path: %1").arg(srcPath).toUtf8().constData());
        startUpload(srcPath, QString(), appDataID, parent, nullptr);
        return;
    }

    QFileInfo dstfileinfo(destPath);

    if (srcFileInfo.isFile()) //if copying a file: replace (moving to debris if existing and different)
    {
        if (dstfileinfo.exists() && (dstfileinfo.isDir() || filesdiffer(srcFileInfo, dstfileinfo)))
        {
            megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        }
        QFile src(srcPath);
        src.copy(destPath); //This will fail if file exists, which should only happen if they don't differ
#ifndef _WIN32
       time_t t = srcFileInfo.lastModified().toTime_t();
       struct utimbuf times = { t, t };
       utime(destPath.toUtf8().constData(), &times);
#endif
    }
    else if (srcFileInfo.isDir()) // for folders we need to recurse
    {
        if (dstfileinfo.exists() && !dstfileinfo.isDir()) //if local destiny is a file, move it to debris
        {
            megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        }

        // get the corresponding remote parent or create if non existent
        std::unique_ptr<MegaNode> newParent(megaApi->getNodeByPath(srcFileInfo.fileName().toUtf8().constData(), parent) );
        if (!newParent || !newParent->isFolder()) //for files it will leave the file and create a folder, same as regular upload
        {
            newParent.reset(); //for files, we don't want them, we'll create a new folder

            unique_ptr<SynchronousRequestListener> srl(new SynchronousRequestListener());

            megaApi->createFolder(srcFileInfo.fileName().toUtf8().constData(), parent, srl.get());
            srl->wait();
            if (srl->getError()->getErrorCode() == MegaError::API_OK)
            {
                newParent.reset(megaApi->getNodeByHandle(srl->getRequest()->getNodeHandle()));
            }
            else
            {
                MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to create folder recursive upload: %1").arg(srcFileInfo.fileName()).toUtf8().constData());
                return;
            }
        }

        if (!newParent) //just in case getNodeByHandle for just created folder
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to obtain newfolder at recursive upload: %1").arg(srcFileInfo.fileName()).toUtf8().constData());
            return;
        }

        //create local folder if non existent. Note this should happen after creating remote folder, otherwise sync algorithm may produce duplicates
        QDir dstDir(destPath);
        dstDir.mkpath(QString::fromAscii(".")); //this will do nothing if already exists

        //loop all contents and call recursive algorithm
        QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            if (!di.fileInfo().isSymLink() && (di.filePath() != destPath))
            {
                if (newParent)
                {
                    uploadRecursivelyIntoASyncedLocation(di.fileInfo(), QDir::toNativeSeparators(destPath + QDir::separator() + di.fileName()), newParent.get(), appDataID);
                }
            }
        }
    }
}

void MegaUploader::upload(QFileInfo info, const QString &nodeName, MegaNode *parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch>& transferBatch)
{
    QPointer<MegaUploader> safePointer = this;

    if (!safePointer)
    {
        return;
    }

    QString fileName = info.fileName();
    if (fileName.isEmpty() && info.isRoot())
    {
        fileName = QDir::toNativeSeparators(info.absoluteFilePath())
                .replace(QString::fromUtf8("\\"), QString::fromUtf8(""))
                .replace(QString::fromUtf8("/"), QString::fromUtf8(""))
                .replace(QString::fromUtf8(":"), QString::fromUtf8(""));

        if (fileName.isEmpty())
        {
            fileName = QString::fromUtf8("Drive");
        }
    }

    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());
    string localPath = megaApi->getLocalPath(parent);
#ifdef WIN32
        QString destPath = QDir::toNativeSeparators(QString::fromWCharArray((const wchar_t *)localPath.data()) + QDir::separator() + fileName);
        if (destPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            destPath = destPath.mid(4);
        }
#else
        QString destPath = QDir::toNativeSeparators(QString::fromUtf8(localPath.data()) + QDir::separator() + fileName);
#endif

    if (localPath.size() && currentPath != destPath && megaApi->isSyncable(destPath.toUtf8().constData(), info.size()))
    {
        if (!destPath.startsWith(QFileInfo(currentPath).canonicalFilePath()))//to avoid recurses //note: destPath should have been cannonicalized already
        {
            std::shared_ptr<QFutureWatcher<void>> futureWatcher = std::make_shared<QFutureWatcher<void>>();
            futureWatcher->setProperty(FILE_PATH, info.absoluteFilePath());

            connect(futureWatcher.get(), &QFutureWatcher<void>::finished,
                    this, &MegaUploader::onUploadRecursivelyIntoASyncedLocationFinished);

            transferBatch->add(info.absoluteFilePath());
            futureWatcher->setFuture(QtConcurrent::run(this, &MegaUploader::uploadRecursivelyIntoASyncedLocation, QFileInfo(currentPath), destPath, parent->copy(), appDataID));
            mUploadRecursivelyIntoASyncedLocationFuture.insert(info.absoluteFilePath(), futureWatcher);

            emit startingTransfers(false);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Skiping local recursive copy to self contained path %1 to %2").arg(currentPath, destPath).toUtf8().constData());
            ((MegaApplication*)qApp)->showErrorMessage(tr("Upload failed") + QString::fromUtf8(": ") + QString::fromUtf8("Cannot upload to location synced with a descendant") );
        }

    }
    else if (info.isFile() || info.isDir())
    {
        QString msg = QString::fromLatin1("Starting upload : '%1' - '%2' - '%3'").arg(info.fileName(), currentPath).arg(appDataID);
        megaApi->log(MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
        transferBatch->add(info.absoluteFilePath());
        startUpload(currentPath, nodeName, appDataID, parent, transferBatch->getCancelTokenPtr());

        emit startingTransfers(true);
    }
}

void MegaUploader::onUploadRecursivelyIntoASyncedLocationFinished()
{
    auto futureWatcher = dynamic_cast<QFutureWatcher<void>*>(sender());
    if(futureWatcher)
    {
        auto filePath = futureWatcher->property(FILE_PATH).toString();
        emit uploadRecursivelyIntoASyncedLocationFinished(filePath);

        mUploadRecursivelyIntoASyncedLocationFuture.remove(filePath);
    }
}

void MegaUploader::startUpload(const QString& localPath, const QString &nodeName, unsigned long long appDataID, MegaNode* parent, MegaCancelToken* cancelToken)
{
    const bool startFirst = false;
    QByteArray localPathArray = localPath.toUtf8();

    const char* fileName = nullptr;
    QByteArray fileNameArray;
    if(!nodeName.isEmpty())
    {
        fileNameArray = nodeName.toUtf8();
        fileName = fileNameArray.constData();
    }

    QByteArray appData = (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8();
    const int64_t mtime = ::mega::MegaApi::INVALID_CUSTOM_MOD_TIME;
    const bool isSrcTemporary = false;
    MegaTransferListener* listener = nullptr;
    megaApi->startUpload(localPathArray.constData(), parent, fileName, mtime, appData.constData(), isSrcTemporary, startFirst, cancelToken, listener);
}
