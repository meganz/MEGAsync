#include "MegaUploader.h"
#include <QThread>
#include "control/Utilities.h"
#include <QMessageBox>
#include <QtCore>

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
    if(localPath.size())
    {
    #ifdef WIN32
        QString destPath = QString::fromWCharArray((const wchar_t *)localPath.data()) + QDir::separator() + info.fileName();
        if(destPath.startsWith(QString::fromAscii("\\\\?\\"))) destPath = destPath.mid(4);
    #else
        QString destPath = QString::fromUtf8(localPath.data()) + QDir::separator() + info.fileName();
    #endif

        QFileInfo dstInfo(destPath);
        if(dstInfo.exists() && (dstInfo.size() != info.size()))
        {
            int res = QMessageBox::warning(NULL, tr("Warning"), tr("The destination folder is synced and you already have a file \n"
                                                         "inside it with the same name (%1).\n"
                                                         "If you continue the upload, the previous file will be overwritten.\n"
                                                         "Are you sure?").arg(info.fileName()), QMessageBox::Yes, QMessageBox::Cancel);
            if(res != QMessageBox::Yes) return;
        }

        QtConcurrent::run(Utilities::copyRecursively, info.absoluteFilePath(), destPath, true);
    }
    else if(info.isFile())
    {
        megaApi->startUpload(QDir::toNativeSeparators(info.absoluteFilePath()).toUtf8().constData(), parent);
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
