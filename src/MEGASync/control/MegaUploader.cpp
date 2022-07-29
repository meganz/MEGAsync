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

bool MegaUploader::upload(QString path, MegaNode *parent, unsigned long long appDataID, MegaCancelToken *cancelToken)
{
    return upload(QFileInfo(path), parent, appDataID, cancelToken);
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

bool MegaUploader::upload(QFileInfo info, MegaNode *parent, unsigned long long appDataID, MegaCancelToken* cancelToken)
{
    QPointer<MegaUploader> safePointer = this;

    if (!safePointer)
    {
        return false;
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
        QString msg = QString::fromLatin1("Starting upload : '%1' - '%2' - '%3'").arg(info.fileName(), currentPath).arg(appDataID);
        megaApi->log(MegaApi::LOG_LEVEL_DEBUG, msg.toUtf8().constData());
        startUpload(currentPath, appDataID, parent, cancelToken);
        return true;
    }
    return false;
}

void MegaUploader::startUpload(const QString& localPath, unsigned long long appDataID, MegaNode* parent, MegaCancelToken* cancelToken)
{
    const bool startFirst = false;
    QByteArray localPathArray = localPath.toUtf8();
    QByteArray appData = (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8();
    const char* filename = nullptr;
    const int64_t mtime = ::mega::MegaApi::INVALID_CUSTOM_MOD_TIME;
    const bool isSrcTemporary = false;
    MegaTransferListener* listener = nullptr;
    megaApi->startUpload(localPathArray.constData(), parent, filename, mtime, appData.constData(), isSrcTemporary, startFirst, cancelToken, listener);
}
