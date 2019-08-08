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

MegaUploader::MegaUploader(MegaApi *megaApi)
{
    this->megaApi = megaApi;

}

MegaUploader::~MegaUploader()
{

}

void MegaUploader::upload(QString path, MegaNode *parent, unsigned long long appDataID)
{
    return upload(QFileInfo(path), parent, appDataID);
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

void MegaUploader::uploadRecursivelyIntoASyncedLocation(QFileInfo srcFileInfo, QString destPath, MegaNode *parent, unsigned long long appDataID)
{
    if (!srcFileInfo.exists())
    {
        return;
    }

    QString srcPath = QDir::toNativeSeparators(srcFileInfo.absoluteFilePath());

    if (!srcPath.size() || !destPath.size())
    {
        return;
    }

//    if (srcPath == destPath) // returning here would discard uploading excluded files. e.g: I want to force a remote folder to have the same as in local (regardless of exclusions), by uploading into it's remote parent
//    {
//        return;
//    }

    if (srcFileInfo.isSymLink() || (!srcFileInfo.isFile() && !srcFileInfo.isDir()) ) //review if ever symlinks are supported
    {
        return;
    }

    if (!megaApi->isSyncable(destPath.toUtf8().constData(), srcFileInfo.size())) //if not syncable, do not copy locally, but simply upload
    {
        //start upload to parent
        megaApi->startUploadWithData(srcPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
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
    else if (srcFileInfo.isDir())
    {
        if (dstfileinfo.exists() && !dstfileinfo.isDir()) //if local destiny is a file, move it to debris
        {
            megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        }
        QDir dstDir(destPath);
        dstDir.mkpath(QString::fromAscii(".")); //this will do nothing if already exists
        QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            if (!di.fileInfo().isSymLink() && (di.filePath() != destPath))
            {
                MegaNode *newParent = megaApi->getNodeByPath(srcFileInfo.fileName().toUtf8().constData(), parent);
                if (!newParent || !newParent->isFolder()) //for files it will leave the file and create a folder, same as regular upload
                {
                    delete newParent; //for files, we don't want them
                    newParent = NULL;

                    SynchronousRequestListener *srl = new SynchronousRequestListener();
                    megaApi->createFolder(srcFileInfo.fileName().toUtf8().constData(), parent,srl);
                    srl->wait();
                    if (srl->getError()->getErrorCode() == MegaError::API_OK)
                    {
                        newParent=megaApi->getNodeByHandle(srl->getRequest()->getNodeHandle());
                    }
                    else
                    {
                        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, QString::fromUtf8("Failed to create folder recursive upload: %1").arg(srcFileInfo.fileName()).toUtf8().constData());
                    }
                    delete srl;

                }
                if (newParent)
                {
                    uploadRecursivelyIntoASyncedLocation(di.fileInfo(), QDir::toNativeSeparators(destPath + QDir::separator() + di.fileName()), newParent, appDataID);
                }

                delete newParent;
            }
        }
    }
}

void MegaUploader::upload(QFileInfo info, MegaNode *parent, unsigned long long appDataID)
{
    QPointer<MegaUploader> safePointer = this;
    QApplication::processEvents();
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
            QtConcurrent::run(this, &MegaUploader::uploadRecursivelyIntoASyncedLocation, QFileInfo(currentPath), destPath, parent->copy(), appDataID);
        }
        else
        {
            MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Skiping local recursive copy to self contained path %1 to %2").arg(currentPath).arg(destPath).toUtf8().constData());
            ((MegaApplication*)qApp)->showErrorMessage(tr("Upload failed") + QString::fromUtf8(": ") + QString::fromUtf8("Cannot upload to location synced with a descendant") );
        }

    }
    else if (info.isFile() || info.isDir())
    {
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
}
