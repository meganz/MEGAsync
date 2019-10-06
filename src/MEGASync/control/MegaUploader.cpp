#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
#include <QMessageBox>
#include <QtCore>
#include <QApplication>
#include <QPointer>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
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
        if (destPath.startsWith(QString::fromLatin1("\\\\?\\")))
        {
            destPath = destPath.mid(4);
        }
#else
        QString destPath = QDir::toNativeSeparators(QString::fromUtf8(localPath.data()) + QDir::separator() + fileName);
#endif

    if (localPath.size() && currentPath != destPath && megaApi->isSyncable(destPath.toUtf8().constData(), info.size()))
    {
        megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        QtConcurrent::run(Utilities::copyRecursively, currentPath, destPath);
    }
    else if (info.isFile() || info.isDir())
    {
        megaApi->startUploadWithData(currentPath.toUtf8().constData(), parent, (QString::number(appDataID) + QString::fromUtf8("*")).toUtf8().constData());
    }
}
