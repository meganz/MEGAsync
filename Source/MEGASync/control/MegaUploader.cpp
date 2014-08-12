#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
#include <QMessageBox>
#include <QtCore>

#if QT_VERSION >= 0x050000
#include <QtConcurrent/QtConcurrent>
#endif

using namespace mega;

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
    NodeList *children =  megaApi->getChildren(parent);
    QByteArray utf8name = info.fileName().toUtf8();
    QString currentPath = QDir::toNativeSeparators(info.absoluteFilePath());
    MegaNode *dupplicate = NULL;
    for(int i=0; i<children->size(); i++)
    {
        MegaNode *child = children->get(i);
        if(!strcmp(utf8name.constData(), child->getName()) &&
            ((info.isDir() && (child->getType()==MegaNode::TYPE_FOLDER)) ||
            (info.isFile() && (child->getType()==MegaNode::TYPE_FILE) && (info.size() == child->getSize()))))
        {
            dupplicate = new MegaNode(child);
            break;
        }
    }
    delete children;

    if(dupplicate)
    {
        if(dupplicate->getType() == MegaNode::TYPE_FILE)
        {
            emit dupplicateUpload(info.absoluteFilePath(), info.fileName(), dupplicate->getHandle());
        }
        if(dupplicate->getType() == MegaNode::TYPE_FOLDER)
        {
            QDir dir(info.absoluteFilePath());
            QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::Hidden | QDir::System | QDir::NoDotAndDotDot);
            for(int i=0; i<entries.size(); i++)
                upload(entries[i], dupplicate);
        }
        delete dupplicate;
        return;
    }

    string localPath = megaApi->getLocalPath(parent);
    if(localPath.size() && megaApi->is_syncable(info.fileName().toUtf8().constData()))
    {
    #ifdef WIN32
        QString destPath = QDir::toNativeSeparators(QString::fromWCharArray((const wchar_t *)localPath.data()) + QDir::separator() + info.fileName());
        if(destPath.startsWith(QString::fromAscii("\\\\?\\"))) destPath = destPath.mid(4);
    #else
        QString destPath = QDir::toNativeSeparators(QString::fromUtf8(localPath.data()) + QDir::separator() + info.fileName());
    #endif

        QFileInfo dstInfo(destPath);
        if(dstInfo.exists() && (destPath != currentPath))
        {
            int res = QMessageBox::warning(NULL, tr("Warning"), tr("The destination folder is synced and you already have a file \n"
                                                         "inside it with the same name (%1).\n"
                                                         "If you continue the upload, the previous file will be overwritten.\n"
                                                         "Are you sure?").arg(info.fileName()), QMessageBox::Yes, QMessageBox::Cancel);
            if(res != QMessageBox::Yes) return;
            megaApi->moveToLocalDebris(destPath.toUtf8().constData());
        }

        QtConcurrent::run(Utilities::copyRecursively, currentPath, destPath);
    }
    else if(info.isFile())
    {
        megaApi->startUpload(currentPath.toUtf8().constData(), parent);
    }
    else if(info.isDir())
    {
        folders.enqueue(info);
        megaApi->createFolder(info.fileName().toUtf8().constData(), parent, delegateListener);
    }
}

void MegaUploader::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_MKDIR:
            if(e->getErrorCode() == MegaError::API_OK)
            {
                MegaNode *parent = megaApi->getNodeByHandle(request->getNodeHandle());
                QDir dir(folders.dequeue().absoluteFilePath());
                QFileInfoList entries = dir.entryInfoList(QDir::AllEntries | QDir::NoDotAndDotDot);
                for(int i=0; i<entries.size(); i++)
                {
                    QFileInfo info = entries[i];
                    if(info.isFile()) megaApi->startUpload(QDir::toNativeSeparators(info.absoluteFilePath()).toUtf8().constData(), parent);
                    else if(info.isDir())
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
