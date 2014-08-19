#include "ExportProcessor.h"

using namespace mega;
using namespace std;

ExportProcessor::ExportProcessor(MegaApi *megaApi, QStringList fileList) :
    QTMegaRequestListener(megaApi)
{
    this->megaApi = megaApi;
    this->fileList = fileList;

    currentIndex = 0;
    remainingNodes = fileList.size();
    importSuccess = 0;
    importFailed = 0;
}

void ExportProcessor::requestLinks()
{
    for(int i=0; i<fileList.size(); i++)
    {
#ifdef WIN32
        if(!fileList[i].startsWith(QString::fromAscii("\\\\")))
            fileList[i].insert(0, QString::fromAscii("\\\\?\\"));
        string tmpPath((const char*)fileList[i].utf16(), fileList[i].size()*sizeof(wchar_t));
#else
        string tmpPath((const char*)fileList[i].toUtf8().constData());
#endif
        MegaNode *node = megaApi->getSyncedNode(&tmpPath);
        megaApi->exportNode(node, this);
        delete node;
    }
}

QStringList ExportProcessor::getValidLinks()
{
    return validPublicLinks;
}

void ExportProcessor::QTonRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    currentIndex++;
    remainingNodes--;
    if(e->getErrorCode() != MegaError::API_OK)
    {
        publicLinks.append(QString());
        importFailed++;
    }
    else
    {
        publicLinks.append(QString::fromAscii(request->getLink()));
        validPublicLinks.append(QString::fromAscii(request->getLink()));
        importSuccess++;
    }

    if(!remainingNodes)
    {
        emit onRequestLinksFinished();
    }
}
