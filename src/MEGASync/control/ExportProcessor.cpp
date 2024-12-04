#include "ExportProcessor.h"

#include "RequestListenerManager.h"

using namespace mega;

ExportProcessor::ExportProcessor(MegaApi* megaApi, QStringList fileList)
    : QObject()
    , fileList(fileList)
{
    init(megaApi, MODE_PATHS, fileList.size());
}

ExportProcessor::ExportProcessor(MegaApi* megaApi, QList<MegaHandle> handleList)
    : QObject()
    , handleList(handleList)
{
    init(megaApi, MODE_HANDLES, handleList.size());
}

void ExportProcessor::init(MegaApi *megaApi, int mode, int size)
{
    this->megaApi = megaApi;
    this->mode = mode;
    this->currentIndex = 0;
    this->remainingNodes = size;
    this->importSuccess = 0;
    this->importFailed = 0;
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
            if (!fileList[i].startsWith(QString::fromLatin1("\\\\")))
            {
                fileList[i].insert(0, QString::fromLatin1("\\\\?\\"));
            }

            std::string tmpPath((const char*)fileList[i].utf16(), fileList[i].size()*sizeof(wchar_t));
    #else
            std::string tmpPath((const char*)fileList[i].toUtf8().constData());
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
        megaApi->exportNode(node, RequestListenerManager::instance().registerAndGetFinishListener(this, true).get());
        delete node;
    }
}

QStringList ExportProcessor::getValidLinks()
{
    return validPublicLinks;
}

void ExportProcessor::onRequestFinish(MegaRequest *request, MegaError *e)
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
        publicLinks.append(QString::fromLatin1(request->getLink()));
        validPublicLinks.append(QString::fromLatin1(request->getLink()));
        importSuccess++;
    }

    if (!remainingNodes)
    {
        emit onRequestLinksFinished();
    }
}
