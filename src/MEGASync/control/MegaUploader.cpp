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

    if (info.isFile() || info.isDir())
    {
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
}
