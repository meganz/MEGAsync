#include "MegaUploader.h"
#include <QThread>
#include "utils/AsyncFileCopy.h"

#include <QMessageBox>

MegaUploader::MegaUploader(MegaApi *megaApi) : QObject(), delegateListener(this)
{
    this->megaApi = megaApi;
}

bool MegaUploader::upload(QString path, Node *parent)
{
    return upload(QFileInfo(path), parent);
}

bool MegaUploader::upload(QFileInfo info, Node *parent)
{
    if(parent->localnode)
    {
        QFile file(info.absoluteFilePath());
        string localseparator;
        localseparator.assign((char*)L"\\",sizeof(wchar_t));
        string path;
        path.insert(0, localseparator);
        LocalNode* l = parent->localnode;
        while (l)
        {
            path.insert(0,l->localname);
            if ((l = l->parent)) path.insert(0, localseparator);
        }
        path.append("", 1);
        QString destPath = QString::fromWCharArray((const wchar_t *)path.data()) + info.fileName();

        if(QFileInfo(destPath).exists())
        {
            int res = QMessageBox::warning(NULL, tr("Warning"), tr("The destination folder is synced and you already have a file \n"
                                                         "inside it with the same name (%1).\n"
                                                         "If you continue the upload, the previous file will be overwritten.\n"
                                                         "Are you sure?").arg(info.fileName()), QMessageBox::Yes, QMessageBox::Cancel);
            if(res != QMessageBox::Yes) return false;
        }

        QThread *thread = new QThread();
        AsyncFileCopy *fileCopy = new AsyncFileCopy(info.absoluteFilePath(), destPath);
        fileCopy->moveToThread(thread);
        thread->start();
        connect(this, SIGNAL(startFileCopy()), fileCopy, SLOT(doWork()));
        connect(thread, SIGNAL(finished()), thread, SLOT(deleteLater()));
        connect(thread, SIGNAL(finished()), fileCopy, SLOT(deleteLater()));
        emit startFileCopy();
        //file.copy(destPath);
        return true;
    }

   if(info.isFile())
    {
        megaApi->startUpload(QDir::toNativeSeparators(info.absoluteFilePath()).toUtf8().constData(), parent);
        return true;
    }
    else if(info.isDir())
    {
        folders.enqueue(info);
        megaApi->createFolder(info.fileName().toUtf8().constData(), parent, &delegateListener);
        return true;
    }
    return false;
}

void MegaUploader::onRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
    switch(request->getType())
    {
        case MegaRequest::TYPE_MKDIR:
            if(e->getErrorCode() == MegaError::API_OK)
            {
                Node *parent = megaApi->getNodeByHandle(request->getNodeHandle());
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
                                              &delegateListener);
                    }
                }
            }
            break;
    }
}
