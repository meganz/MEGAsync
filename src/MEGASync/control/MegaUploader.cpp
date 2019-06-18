#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
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

void MegaUploader::uploadWithOptimizedLocalRecursiveCopy(QString currentPath, QString destPath, MegaNode *parent, unsigned long long appDataID)
{
    //first copy recursively attending to sync exclusion criteria
    if (!destPath.startsWith(QFileInfo(currentPath).canonicalFilePath()))//to avoid recurses //note: destPath should have been cannonicalized already
    {
        MegaUploader::copyRecursivelyIfSyncable(currentPath, destPath);
    }
    else
    {
        MegaApi::log(MegaApi::LOG_LEVEL_WARNING, QString::fromUtf8("Skiping local recursive copy to self contained path %1 to %2").arg(currentPath).arg(destPath).toUtf8().constData());
    }

    // then, do manual upload (we want files to be uploaded nevertheless)
    // notice, sync engine might revert changes afterwards for corner cases:
    //      e.g: uploading an older version that doesn't match size criteria
    //     Still, the same would happen if uploading with another client
    megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    delete parent;
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

void MegaUploader::copyRecursivelyIfSyncable(QString srcPath, QString dstPath)
{
    if (!srcPath.size() || !dstPath.size())
    {
        return;
    }

    QFileInfo source(srcPath);
    if (!source.exists())
    {
        return;
    }

    if (srcPath == dstPath)
    {
        return;
    }


    if (source.isSymLink() || (!source.isFile() && !source.isDir()) ) //review if ever symlinks are supported
    {
        return;
    }

    if (!megaApi->isSyncable(dstPath.toUtf8().constData(), source.size()))
    {
        return;
    }
    QFileInfo dstfileinfo(dstPath);

    if (source.isFile())
    {
        if (dstfileinfo.exists() && (dstfileinfo.isDir() || filesdiffer(source, dstfileinfo)))
        {
            megaApi->moveToLocalDebris(dstPath.toUtf8().constData());
        }
        QFile src(srcPath);
        src.copy(dstPath); //This will fail if file exists, which should only happen if they don't differ
#ifndef _WIN32
       time_t t = source.lastModified().toTime_t();
       struct utimbuf times = { t, t };
       utime(dstPath.toUtf8().constData(), &times);
#endif
    }
    else if (source.isDir())
    {
        if (dstfileinfo.exists() && !dstfileinfo.isDir())
        {
            megaApi->moveToLocalDebris(dstPath.toUtf8().constData());
        }
        QDir dstDir(dstPath);
        dstDir.mkpath(QString::fromAscii("."));
        QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
        while (di.hasNext())
        {
            di.next();
            if (!di.fileInfo().isSymLink() && (di.filePath() != dstPath))
            {
                copyRecursivelyIfSyncable(di.filePath(), dstPath + QDir::separator() + di.fileName());
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
        QtConcurrent::run(this, &MegaUploader::uploadWithOptimizedLocalRecursiveCopy, currentPath, destPath, parent->copy(), appDataID);
    }
    else if (info.isFile() || info.isDir())
    {
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
}
