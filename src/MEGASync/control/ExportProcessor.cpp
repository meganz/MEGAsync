#include "ExportProcessor.h"

using namespace mega;
using namespace std;

ExportProcessor::ExportProcessor(MegaApi *megaApi, QStringList fileList) : QObject()
{
    this->megaApi = megaApi;
    this->fileList = fileList;
    this->mode = MODE_PATHS;

    currentIndex = 0;
    remainingNodes = fileList.size();
    importSuccess = 0;
    importFailed = 0;

    delegateListener = new QTMegaRequestListener(megaApi, this);
}

ExportProcessor::ExportProcessor(MegaApi *megaApi, QList<MegaHandle> handleList)
{
    this->megaApi = megaApi;
    this->handleList = handleList;
    this->mode = MODE_HANDLES;

    currentIndex = 0;
    remainingNodes = handleList.size();
    importSuccess = 0;
    importFailed = 0;

    delegateListener = new QTMegaRequestListener(megaApi, this);
}

ExportProcessor::~ExportProcessor()
{
    delete delegateListener;
}

void ExportProcessor::requestLinks()
{
    int size = (mode == MODE_PATHS) ? fileList.size() : handleList.size();
    if (!size)
    {
        emit onRequestLinksFinished();
        return;
    }

    for (int i = 0; i < size; i++)
    {
        MegaNode *node = NULL;
        if (mode == MODE_PATHS)
        {
    #ifdef WIN32
            if (!fileList[i].startsWith(QString::fromAscii("\\\\")))
            {
                fileList[i].insert(0, QString::fromAscii("\\\\?\\"));
            }

            string tmpPath((const char*)fileList[i].utf16(), fileList[i].size()*sizeof(wchar_t));
    #else
            string tmpPath((const char*)fileList[i].toUtf8().constData());
    #endif

            node = megaApi->getSyncedNode(&tmpPath);
            if (!node)
            {
                const char *fpLocal = megaApi->getFingerprint(tmpPath.c_str());
                node = megaApi->getNodeByFingerprint(fpLocal);
                delete [] fpLocal;
            }
        }
        else
        {
            node = megaApi->getNodeByHandle(handleList[i]);
        }
        megaApi->exportNode(node, delegateListener);
        delete node;
    }
}

QStringList ExportProcessor::getValidLinks()
{
    return validPublicLinks;
}

void ExportProcessor::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    currentIndex++;
    remainingNodes--;
    if (e->getErrorCode() != MegaError::API_OK)
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

    if (!remainingNodes)
    {
        emit onRequestLinksFinished();
    }
}
