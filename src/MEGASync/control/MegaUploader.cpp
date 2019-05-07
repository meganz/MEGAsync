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


void MegaUploader::uploadRecursively(QString srcPath, QString dstPath, MegaNode *parent, unsigned long long appDataID)
{
    if (!srcPath.size() || !dstPath.size())
    {
        return;
    }

    QFileInfo srcInfo(srcPath);
    if (!srcInfo.exists())
    {
        return;
    }

    if (srcPath == dstPath)
    {
        return;
    }
    QFileInfo dstInfo(dstPath);
    QFile dst(dstPath);

    if (megaApi->isSyncable(dstPath.toUtf8().constData(), srcInfo.size()))
    {
        if (srcInfo.isFile())
        {
            if (dst.exists())
            {
                megaApi->moveToLocalDebris(dstPath.toUtf8().constData());
            }

            QFile src(srcPath);

            //do copy it!
            src.copy(dstPath);
#ifndef _WIN32
            QFileInfo info(src);
            time_t t = info.lastModified().toTime_t();
            struct utimbuf times = { t, t };
            utime(dstPath.toUtf8().constData(), &times);
#endif
            return;
        }
        else if (srcInfo.isDir())
        {
            QDir dstDir(dstPath);
            if (dst.exists() && dstInfo.isFile())
            {
                megaApi->moveToLocalDebris(dstPath.toUtf8().constData());
            }
            if (!dst.exists())
            {
                dstDir.mkpath(QString::fromAscii("."));
            }

            QDirIterator di(srcPath, QDir::AllEntries | QDir::Hidden | QDir::NoDotAndDotDot);
            while (di.hasNext())
            {
                di.next();
                if (!di.fileInfo().isSymLink())
                {
                    MegaNode *newparent = megaApi->getNodeByPath(di.fileName().toUtf8(), parent);
                    //TODO: problem: parent might not exist (it hasn't been created yet!!!!)
                    // we'd need something with listeners and subfolder creations and the like (too overcomplicated perhaps)
                    //TODO: non ascii filenames?
                    uploadRecursively(di.filePath(), dstPath + QDir::separator() + di.fileName(), newparent, appDataID);
                    delete newparent;
                }
            }
        }
    }
    else
    {
        megaApi->startUploadWithData(srcPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
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
        //QtConcurrent::run(this, &MegaUploader::uploadRecursively, currentPath, destPath, parent, appDataID);
        megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        QtConcurrent::run(Utilities::copyRecursively, currentPath, destPath);
        //TODO: add this
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
    else if (info.isFile() || info.isDir())
    {
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
}
