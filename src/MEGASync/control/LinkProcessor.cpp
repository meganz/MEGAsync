#include "LinkProcessor.h"
#include "Utilities.h"
#include "Preferences/Preferences.h"
#include "MegaApplication.h"
#include <QDir>
#include <QDateTime>
#include <QApplication>

using namespace mega;

LinkProcessor::LinkProcessor(QStringList linkList, MegaApi *megaApi, MegaApi *megaApiFolders)
    : mParentHandler(nullptr),
      mRequestCounter(0)
{
    this->megaApi = megaApi;
    this->megaApiFolders = megaApiFolders;
    this->linkList = linkList;
    for (int i = 0; i < linkList.size(); i++)
    {
        linkSelected.append(false);
        mLinkNode.append(nullptr);
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
    delegateListener->deleteLater();
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

std::shared_ptr<MegaNode> LinkProcessor::getNode(int id)
{
    return mLinkNode[id];
}

int LinkProcessor::size() const
{
    return linkList.size();
}

void LinkProcessor::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* e)
{
    if (request->getType() == MegaRequest::TYPE_GET_PUBLIC_NODE)
    {
        if (e->getErrorCode() != MegaError::API_OK)
        {
            mLinkNode[currentIndex].reset();
        }
        else
        {
            mLinkNode[currentIndex] = std::shared_ptr<mega::MegaNode>(request->getPublicMegaNode());
        }

        linkError[currentIndex] = e->getErrorCode();
        currentIndex++;
        emit onLinkInfoAvailable(currentIndex-1);
        if (currentIndex == linkList.size())
        {
            emit onLinkInfoRequestFinish();
        }
        else
        {
            requestLinkInfo();
        }
    }
    else if (request->getType() == MegaRequest::TYPE_CREATE_FOLDER)
    {
        std::unique_ptr<MegaNode>n(megaApi->getNodeByHandle(request->getNodeHandle()));
        importLinks(n.get());
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
    else if (request->getType() == MegaRequest::TYPE_LOGIN)
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            mRequestCounter++;
            megaApiFolders->fetchNodes(this);
        }
        else
        {
            mLinkNode[currentIndex].reset();
            linkError[currentIndex] = e->getErrorCode();
            currentIndex++;
            emit onLinkInfoAvailable(currentIndex - 1);
            if (currentIndex == linkList.size())
            {
                emit onLinkInfoRequestFinish();
            }
            else
            {
                requestLinkInfo();
            }
        }
    }
    else if (request->getType() == MegaRequest::TYPE_FETCH_NODES)
    {
        if (e->getErrorCode() == MegaError::API_OK)
        {
            std::unique_ptr<MegaNode> rootNode(nullptr);
            QString currentStr = linkList[currentIndex];
            QString splitSeparator;

            if (currentStr.count(QChar::fromLatin1('!')) == 3)
            {
                splitSeparator = QString::fromUtf8("!");
            }
            else if (currentStr.count(QChar::fromLatin1('!')) == 2
                     && currentStr.count(QChar::fromLatin1('?')) == 1)
            {
                splitSeparator = QString::fromUtf8("?");
            }
            else if (currentStr.count(QString::fromUtf8("/folder/")) == 2)
            {
                splitSeparator = QString::fromUtf8("/folder/");
            }
            else if (currentStr.count(QString::fromUtf8("/folder/")) == 1
                     && currentStr.count(QString::fromUtf8("/file/")) == 1)
            {
                splitSeparator = QString::fromUtf8("/file/");
            }

            if (splitSeparator.isEmpty())
            {
                rootNode.reset(megaApiFolders->getRootNode());
            }
            else
            {
                QStringList linkparts = currentStr.split(splitSeparator, Qt::KeepEmptyParts);
                MegaHandle handle = MegaApi::base64ToHandle(linkparts.last().toUtf8().constData());
                rootNode.reset(megaApiFolders->getNodeByHandle(handle));
            }

            Preferences::instance()->setLastPublicHandle(request->getNodeHandle(), MegaApi::AFFILIATE_TYPE_FILE_FOLDER);
            mLinkNode[currentIndex] = std::shared_ptr<mega::MegaNode>(megaApiFolders->authorizeNode(rootNode.get()));
        }
        else
        {
            mLinkNode[currentIndex].reset();
        }

        linkError[currentIndex] = e->getErrorCode();
        currentIndex++;
        emit onLinkInfoAvailable(currentIndex-1);
        if (currentIndex == linkList.size())
        {
            emit onLinkInfoRequestFinish();
        }
        else
        {
            requestLinkInfo();
        }
    }

    mRequestCounter--;

    if(!mParentHandler && mRequestCounter == 0)
    {
        deleteLater();
    }
}

void LinkProcessor::requestLinkInfo()
{
    if (currentIndex < 0 || currentIndex >= linkList.size())
    {
        return;
    }

    QString link = linkList[currentIndex];
    if (link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/#F!"))
            || link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/folder/")))
    {
        std::unique_ptr<char []> authToken(megaApi->getAccountAuth());
        if (authToken)
        {
            megaApiFolders->setAccountAuth(authToken.get());
        }

        mRequestCounter++;
        megaApiFolders->loginToFolder(link.toUtf8().constData(), delegateListener);
    }
    else
    {
        mRequestCounter++;
        megaApi->getPublicNode(link.toUtf8().constData(), delegateListener);
    }
}

void LinkProcessor::importLinks(QString megaPath)
{
    std::unique_ptr<MegaNode>node(megaApi->getNodeByPath(megaPath.toUtf8().constData()));
    if (node)
    {
        importLinks(node.get());
    }
    else
    {
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();
        if (!rootNode)
        {
            emit onLinkImportFinish();
            return;
        }

        mRequestCounter++;
        megaApi->createFolder("MEGA Imports", rootNode.get(), delegateListener);
    }
}

void LinkProcessor::importLinks(MegaNode* node)
{
    if (!node)
    {
        return;
    }

    std::unique_ptr<MegaNodeList> children(megaApi->getChildren(node));
    importParentFolder = node->getHandle();

    for (int i = 0; i < linkList.size(); i++)
    {
        if (!mLinkNode[i])
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Trying to import a NULL node");
        }

        if (mLinkNode[i] && linkSelected[i] && !linkError[i])
        {
            bool dupplicate = false;
            MegaHandle duplicateHandle = INVALID_HANDLE;
            const char* name = mLinkNode[i]->getName();
            long long size = mLinkNode[i]->getSize();

            for (int j = 0; j < children->size(); j++)
            {
                MegaNode *child = children->get(j);
                if (!strcmp(name, child->getName()) && (size == child->getSize()))
                {
                    dupplicate = true;
                    duplicateHandle = child->getHandle();
                }
            }

            if (!dupplicate)
            {
                remainingNodes++;
                mRequestCounter++;
                megaApi->copyNode(mLinkNode[i].get(), node, delegateListener);
            }
        }
    }

    if(remainingNodes == 0)
    {
        emit onLinkImportFinish();
    }
}

MegaHandle LinkProcessor::getImportParentFolder()
{
    return importParentFolder;
}

void LinkProcessor::downloadLinks(const QString& localPath)
{
    for (int i = 0; i < linkList.size(); i++)
    {
        if (mLinkNode[i] && linkSelected[i])
        {
            startDownload(mLinkNode[i].get(), localPath);
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

bool LinkProcessor::atLeastOneLinkValidAndSelected() const
{
    for (int iLink = 0; iLink < size(); iLink++)
    {
        const bool isSelected{linkSelected.at(iLink)};
        if(mLinkNode.at(iLink) && isSelected)
        {
            return true;
        }
    }
    return false;
}

void LinkProcessor::setParentHandler(QObject *parent)
{
    connect(parent, &QObject::destroyed, this, [this](){
        if(mRequestCounter == 0)
        {
            deleteLater();
        }
    });

    mParentHandler = parent;
}

void LinkProcessor::startDownload(mega::MegaNode* linkNode, const QString &localPath)
{
    const bool startFirst = false;
    QByteArray path = (localPath + QDir::separator()).toUtf8();
    const char* name = nullptr;
    const char* appData = nullptr;
    MegaCancelToken* cancelToken = nullptr; // No cancellation possible
    MegaTransferListener* listener = nullptr;
    megaApi->startDownload(linkNode, path.constData(), name, appData, startFirst, cancelToken,
                           MegaTransfer::COLLISION_CHECK_FINGERPRINT,
                           MegaTransfer::COLLISION_RESOLUTION_NEW_WITH_N,
                           listener);
}
