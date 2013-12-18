#include "MegaUploader.h"

MegaUploader::MegaUploader(MegaApi *megaApi, Preferences *preferences) : delegateListener(this)
{
    this->megaApi = megaApi;
    this->preferences = preferences;
}

bool MegaUploader::upload(QString path, Node *parent)
{
    return upload(QFileInfo(path), parent);
}

bool MegaUploader::upload(QFileInfo info, Node *parent)
{
    if(info.isFile())
    {
        megaApi->startUpload(QDir::toNativeSeparators(info.absoluteFilePath()).toUtf8().constData(), parent,
                             preferences->uploadLimitKB());
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
