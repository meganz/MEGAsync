#include "QTMegaApi.h"

using namespace mega;

#include "control/Preferences.h"
#include "platform/Platform.h"

QTMegaApi::QTMegaApi(const char *appKey, const char *basePath, const char *userAgent) : MegaApi(appKey, basePath, userAgent)
{ }

void QTMegaApi::fetchnodes_result(error e)
{
    MegaApi::fetchnodes_result(e);

    Preferences *preferences = Preferences::instance();
    if(preferences->logged() && preferences->wasPaused())
        this->pauseTransfers(true);

    if(preferences->logged() && !client->syncs.size())
    {
        //Start syncs
        for(int i=0; i<preferences->getNumSyncedFolders(); i++)
        {
            Node *node = client->nodebyhandle(preferences->getMegaFolderHandle(i));
            QString localFolder = preferences->getLocalFolder(i);
            MegaRequest *syncRequest = new MegaRequest(MegaRequest::TYPE_ADD_SYNC);
            syncRequest->setNodeHandle(preferences->getMegaFolderHandle(i));
            syncRequest->setFile(localFolder.toUtf8().constData());
            client->restag = client->nextreqtag();
            requestMap[client->restag]=syncRequest;

            MegaNode *megaNode = getNodeByHandle(preferences->getMegaFolderHandle(i));
            const char *nodePath = getNodePath(megaNode);
            if(!nodePath || preferences->getMegaFolder(i).compare(QString::fromUtf8(nodePath)))
            {
                fireOnRequestFinish(this, syncRequest, MegaError(API_ENOENT));
                delete megaNode;
                delete[] nodePath;
                continue;
            }
            delete megaNode;
            delete[] nodePath;

            string localname;
            string utf8name(localFolder.toUtf8().constData());
    #ifdef WIN32
            if((utf8name.size()<2) || utf8name.compare(0, 2, "\\\\"))
                utf8name.insert(0, "\\\\?\\");
    #endif
            client->fsaccess->path2local(&utf8name, &localname);
            error syncError = client->addsync(&localname, DEBRISFOLDER, NULL, node, -1);
            fireOnRequestFinish(this, syncRequest, MegaError(syncError));
        }
    }
}

void QTMegaApi::syncupdate_treestate(LocalNode *l)
{
    MegaApi::syncupdate_treestate(l);

    string path;
    l->getlocalpath(&path, true);

    sdkMutex.unlock();

    #ifdef WIN32
        path.append("", 1);
        QString localPath = QString::fromWCharArray((const wchar_t *)path.data());
    #else
        QString localPath = QString::fromUtf8(path.data());
    #endif
    Platform::notifyItemChange(localPath);

    sdkMutex.lock();
}
