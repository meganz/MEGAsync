#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
#include <QMessageBox>
#include <QtCore>
#include <QApplication>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;
using namespace std;

MegaUploader::MegaUploader(MegaApi *megaApi) : QObject()
{
    this->megaApi = megaApi;
    delegateListener = new QTMegaRequestListener(megaApi, this);
}

MegaUploader::~MegaUploader()
{
    delete delegateListener;
}

void MegaUploader::upload(QString path, MegaNode *parent)
{
    return upload(QFileInfo(path), parent);
}

void MegaUploader::upload(QFileInfo info, MegaNode *parent)
{
    QApplication::processEvents();
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

    QByteArray utf8name = fileName.toUtf8();
    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());
    if (info.isDir())
    {
        MegaNode *child = megaApi->getChildNode(parent, utf8name.constData());
        if (child && child->getType() == MegaNode::TYPE_FOLDER)
        {
            QDir dir(info.absoluteFilePath());
            QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
            for (int i = 0; i < entries.size(); i++)
            {
                upload(entries[i], child);
            }
            delete child;
            return;
        }
        delete child;
    }

    string localPath = megaApi->getLocalPath(parent);
    if (localPath.size() && megaApi->isSyncable(utf8name.constData()))
    {
#ifdef WIN32
        QString destPath = QDir::toNativeSeparators(QString::fromWCharArray((const wchar_t *)localPath.data()) + QDir::separator() + fileName);
        if (destPath.startsWith(QString::fromAscii("\\\\?\\")))
        {
            destPath = destPath.mid(4);
        }
#else
        QString destPath = QDir::toNativeSeparators(QString::fromUtf8(localPath.data()) + QDir::separator() + fileName);
#endif
        megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        QtConcurrent::run(Utilities::copyRecursively, currentPath, destPath);
    }
    else if (info.isFile())
    {
        megaApi->startUpload(currentPath.toUtf8().constData(), parent);
    }
    else if (info.isDir())
    {
        folders.enqueue(info);
        megaApi->createFolder(utf8name.constData(), parent, delegateListener);
    }
}

void MegaUploader::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_CREATE_FOLDER:
            if (e->getErrorCode() == MegaError::API_OK)
            {
                MegaNode *parent = megaApi->getNodeByHandle(request->getNodeHandle());
                QDir dir(folders.dequeue().absoluteFilePath());
                QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                for (int i = 0; i < entries.size(); i++)
                {
                    QFileInfo info = entries[i];
                    if (info.isFile())
                    {
                        megaApi->startUpload(QDir::toNativeSeparators(info.absoluteFilePath()).toUtf8().constData(), parent);
                    }
                    else if (info.isDir())
                    {
                        folders.enqueue(info);
                        megaApi->createFolder(info.fileName().toUtf8().constData(),
                                              parent,
                                              delegateListener);
                    }
                }
                delete parent;
            }
            break;
    }
}
