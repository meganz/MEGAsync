#include "LinkProcessor.h"
#include "Utilities.h"
#include <QDir>
#include <QDateTime>
#include <QApplication>

using namespace mega;

LinkProcessor::LinkProcessor(QStringList linkList, MegaApi *megaApi, MegaApi *megaApiGuest) : QObject()
{
    this->megaApi = megaApi;
    this->megaApiGuest = megaApiGuest;
    this->linkList = linkList;
    for (int i = 0; i < linkList.size(); i++)
    {
        linkSelected.append(true);
        linkNode.append(NULL);
        linkError.append(MegaError::API_ENOENT);
    }

    importParentFolder = mega::INVALID_HANDLE;
    currentIndex = 0;
    remainingNodes = 0;
    importSuccess = 0;
    importFailed = 0;

    delegateListener = new QTMegaRequestListener(megaApi, this);
}

LinkProcessor::~LinkProcessor()
{
    delete delegateListener;
    for (int i = 0; i < linkNode.size(); i++)
    {
        delete linkNode[i];
    }
}

QString LinkProcessor::getLink(int id)
{
    return linkList[id];
}

bool LinkProcessor::isSelected(int id)
{
    return linkSelected[id];
}

int LinkProcessor::getError(int id)
{
    return linkError[id];
}

MegaNode *LinkProcessor::getNode(int id)
{
    return linkNode[id];
}

int LinkProcessor::size()
{
    return linkList.size();
}

void LinkProcessor::onRequestFinish(MegaApi *, MegaRequest *request, MegaError *e)
{
    if (request->getType() == MegaRequest::TYPE_GET_PUBLIC_NODE)
    {
        if (e->getErrorCode() != MegaError::API_OK)
        {
            linkNode[currentIndex] = NULL;
        }
        else
        {
            linkNode[currentIndex] = request->getPublicMegaNode();
        }

        linkError[currentIndex] = e->getErrorCode();
        linkSelected[currentIndex] = (linkError[currentIndex] == MegaError::API_OK);
        if (!linkError[currentIndex])
        {
            QString name = QString::fromUtf8(linkNode[currentIndex]->getName());
            if (!name.compare(QString::fromAscii("NO_KEY")) || !name.compare(QString::fromAscii("DECRYPTION_ERROR")))
            {
                linkSelected[currentIndex] = false;
            }
        }
        currentIndex++;
        emit onLinkInfoAvailable(currentIndex-1);
        if (currentIndex == linkList.size())
        {
            emit onLinkInfoRequestFinish();
        }
    }
    else if (request->getType() == MegaRequest::TYPE_CREATE_FOLDER)
    {
        MegaNode *n = megaApi->getNodeByHandle(request->getNodeHandle());
        importLinks(n);
        delete n;
    }
    else if (request->getType() == MegaRequest::TYPE_COPY)
    {
        remainingNodes--;
        if (e->getErrorCode()==MegaError::API_OK)
        {
            importSuccess++;
        }
        else
        {
            importFailed++;
        }

        if (!remainingNodes)
        {
            emit onLinkImportFinish();
        }
    }
}

void LinkProcessor::requestLinkInfo()
{
    for (int i = 0; i < linkList.size(); i++)
    {
        if (megaApiGuest)
        {
            megaApiGuest->getPublicNode(linkList[i].toUtf8().constData(), delegateListener);
        }
        else
        {
            megaApi->getPublicNode(linkList[i].toUtf8().constData(), delegateListener);
        }
    }
}

void LinkProcessor::importLinks(QString megaPath)
{
    MegaNode *node = megaApi->getNodeByPath(megaPath.toUtf8().constData());
    if (node)
    {
        importLinks(node);
        delete node;
    }
    else
    {
        MegaNode *rootNode = megaApi->getRootNode();
        if (!rootNode)
        {
            emit onLinkImportFinish();
            return;
        }

        megaApi->createFolder("MEGAsync Imports", rootNode, delegateListener);
        delete rootNode;
    }
}

void LinkProcessor::importLinks(MegaNode *node)
{
    if (!node)
    {
        return;
    }

    MegaNodeList *children = megaApi->getChildren(node);
    importParentFolder = node->getHandle();

    for (int i = 0; i < linkList.size(); i++)
    {
        if (!linkNode[i])
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Trying to import a NULL node");
        }

        if (linkNode[i] && linkSelected[i] && !linkError[i])
        {
            bool dupplicate = false;
            long long dupplicateHandle;
            const char* name = linkNode[i]->getName();
            long long size = linkNode[i]->getSize();

            for (int j = 0; j < children->size(); j++)
            {
                MegaNode *child = children->get(j);
                if (!strcmp(name, child->getName()) && (size == child->getSize()))
                {
                    dupplicate = true;
                    dupplicateHandle = child->getHandle();
                }
            }

            if (!dupplicate)
            {
                remainingNodes++;
                megaApi->copyNode(linkNode[i], node, delegateListener);
            }
            else
            {
                emit onDupplicateLink(linkList[i], QString::fromUtf8(name), dupplicateHandle);
            }
        }
    }
    delete children;
}

MegaHandle LinkProcessor::getImportParentFolder()
{
    return importParentFolder;
}

void LinkProcessor::downloadLinks(QString localPath)
{
    for (int i = 0; i < linkList.size(); i++)
    {
        if (linkNode[i] && linkSelected[i])
        {
            QApplication::processEvents();

            QDir dir(localPath);

            char *escapedName = megaApi->escapeFsIncompatible(linkNode[i]->getName());
            QString fullPath = dir.filePath(QString::fromUtf8(escapedName));
            delete [] escapedName;

            QFileInfo info(fullPath);
            if (info.exists())
            {
                const char *fpLocal = megaApi->getFingerprint(fullPath.toUtf8().constData());
                const char *fpRemote = megaApi->getFingerprint(linkNode[i]);
                const char *key = linkNode[i]->getBase64Key();

                if (key && ((fpLocal && fpRemote && !strcmp(fpLocal,fpRemote))
                        || (!fpRemote && linkNode[i]->getSize() == info.size()
                            && linkNode[i]->getModificationTime() == (info.lastModified().toMSecsSinceEpoch()/1000))))
                {
                    delete [] fpLocal;
                    delete [] fpRemote;
                    emit dupplicateDownload(QDir::toNativeSeparators(fullPath),
                                            QString::fromUtf8(linkNode[i]->getName()),
                                            linkNode[i]->getHandle(),
                                            QString::fromUtf8(key));
                    delete [] key;
                    continue;
                }
                delete [] fpLocal;
                delete [] fpRemote;
                delete [] key;
            }

            if (megaApiGuest)
            {
                megaApiGuest->startDownload(linkNode[i], (localPath + QDir::separator()).toUtf8().constData());
            }
            else
            {
                megaApi->startDownload(linkNode[i], (localPath + QDir::separator()).toUtf8().constData());
            }
        }
    }
}

void LinkProcessor::setSelected(int linkId, bool selected)
{
    linkSelected[linkId] = selected;
}

int LinkProcessor::numSuccessfullImports()
{
    return importSuccess;
}

int LinkProcessor::numFailedImports()
{
    return importFailed;
}

int LinkProcessor::getCurrentIndex()
{
    return currentIndex;
}
