#include "LinkProcessor.h"
#include <QDir>

LinkProcessor::LinkProcessor(MegaApi *megaApi, QStringList linkList) : QTMegaRequestListener()
{
	this->megaApi = megaApi;
	this->linkList = linkList;
	for(int i=0; i<linkList.size();i++)
	{
		linkSelected.append(true);
		linkNode.append(NULL);
		linkError.append(MegaError::API_ENOENT);
	}

	importParentFolder = UNDEF;
	currentIndex = 0;
	remainingNodes = 0;
	importSuccess = 0;
	importFailed = 0;
}

LinkProcessor::~LinkProcessor()
{
	for(int i=0; i<linkNode.size();i++)
		delete linkNode[i];
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

void LinkProcessor::QTonRequestFinish(MegaApi *api, MegaRequest *request, MegaError *e)
{
	if(request->getType() == MegaRequest::TYPE_GET_PUBLIC_NODE)
	{
        if(e->getErrorCode() != API_OK)  linkNode[currentIndex] = NULL;
        else linkNode[currentIndex] = new MegaNode(request->getPublicNode());

		linkError[currentIndex] = e->getErrorCode();
		linkSelected[currentIndex] = (linkError[currentIndex]==MegaError::API_OK);
		if(!linkError[currentIndex])
		{
            QString name = QString::fromUtf8(linkNode[currentIndex]->getName());
            if(!name.compare(QString::fromAscii("NO_KEY")) || !name.compare(QString::fromAscii("DECRYPTION_ERROR")))
				linkSelected[currentIndex] = false;
		}
		currentIndex++;
		emit onLinkInfoAvailable(currentIndex-1);
		if(currentIndex==linkList.size())
			emit onLinkInfoRequestFinish();
	}
	else if(request->getType() == MegaRequest::TYPE_MKDIR)
	{
		importLinks(megaApi->getNodeByHandle(request->getNodeHandle()));
	}
	else if(request->getType() == MegaRequest::TYPE_IMPORT_NODE)
	{
		remainingNodes--;
		if(e->getErrorCode()==MegaError::API_OK) importSuccess++;
		else importFailed ++;
		if(!remainingNodes) emit onLinkImportFinish();
	}
}

void LinkProcessor::requestLinkInfo()
{
	for(int i=0; i<linkList.size(); i++)
	{
		megaApi->getPublicNode(linkList[i].toUtf8().constData(), this);
	}
}

void LinkProcessor::importLinks(QString megaPath)
{
    MegaNode *node = megaApi->getNodeByPath(megaPath.toUtf8().constData());
    if(node)
    {
        importLinks(node);
        delete node;
    }
    else
    {
        MegaNode *rootNode = megaApi->getRootNode();
        megaApi->createFolder("MEGAsync Imports", megaApi->getRootNode(), this);
        delete rootNode;
    }
}

void LinkProcessor::importLinks(MegaNode *node)
{
	if(!node) return;

    NodeList *children = megaApi->getChildren(node);
    importParentFolder = node->getHandle();

	for(int i=0; i<linkList.size(); i++)
	{
        bool dupplicate = false;
        long long dupplicateHandle;
        const char* name = linkNode[i]->getName();
        long long size = linkNode[i]->getSize();

        for(int j=0; j<children->size(); j++)
        {
            MegaNode *child = children->get(j);
            if(!strcmp(name, child->getName()) && (size == child->getSize()))
            {
                dupplicate = true;
                dupplicateHandle = child->getHandle();
            }
        }

        if(linkNode[i] && linkSelected[i] && !linkError[i])
		{
            if(!dupplicate)
            {
                remainingNodes++;
                megaApi->importPublicNode(linkNode[i], node, this);
            }
            else
            {
                emit onDupplicateLink(linkList[i], QString::fromUtf8(name), dupplicateHandle);
            }
		}
	}
    delete children;
}

handle LinkProcessor::getImportParentFolder()
{
	return importParentFolder;
}

void LinkProcessor::downloadLinks(QString localPath)
{
	for(int i=0; i<linkList.size(); i++)
	{
		if(linkNode[i] && linkSelected[i])
		{
            megaApi->startPublicDownload(linkNode[i], (localPath+QDir::separator()).toUtf8().constData());
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
