#include "LinkProcessor.h"

#include "CommonMessages.h"
#include "MegaApplication.h"
#include "Preferences.h"
#include "RequestListenerManager.h"

#include <QDir>

using namespace mega;

LinkProcessor::LinkProcessor(MegaApi* megaApi, MegaApi* megaApiFolders)
    : LinkProcessor(QStringList(), megaApi, megaApiFolders) // Delegate to the second constructor with an empty QStringList
{
}

LinkProcessor::LinkProcessor(const QStringList& linkList,
                             MegaApi* megaApi,
                             MegaApi* megaApiFolders):
    mMegaApi(megaApi),
    mMegaApiFolders(megaApiFolders),
    mLinkList(linkList),
    mImportParentFolder(mega::INVALID_HANDLE),
    mDownloader(std::make_shared<MegaDownloader>(megaApi, this)),
    mCurrentIndex(0)
{
    resetAndSetLinkList(linkList);

    // Register for SDK Request callbacks
    mDelegateListener = RequestListenerManager::instance().registerAndGetFinishListener(this);
}

LinkProcessor::~LinkProcessor()
{
}

void LinkProcessor::resetAndSetLinkList(const QStringList& linkList)
{
    mLinkObjects.clear();
    mCurrentIndex = 0;
    mLinkList = linkList;

    for (int i = 0; i < linkList.size(); i++)
    {
        mLinkObjects.append(std::make_shared<LinkInvalid>());
    }
}

QString LinkProcessor::getLink(int index) const
{
    if (!(isValidIndex(mLinkList, index))) { return QString::fromUtf8(""); }

    return mLinkList[index];
}

inline bool LinkProcessor::isLinkObjectValid(int index) const
{
    return (isValidIndex(mLinkObjects, index) && mLinkObjects[index]);
}

bool LinkProcessor::isSelected(int index) const
{
    if (!isLinkObjectValid(index)) { return false; }

    return mLinkObjects[index]->isSelected();
}

MegaNodeSPtr LinkProcessor::getNode(int index) const
{
    if (!isLinkObjectValid(index)) { return nullptr; }

    return mLinkObjects[index]->getMegaNode();
}

void LinkProcessor::sendLinkInfoAvailableSignal(int index)
{
    if (!isLinkObjectValid(index)) { return; }

    std::shared_ptr<LinkObject> linkObject = mLinkObjects[index];

    emit onLinkInfoAvailable(index,
                             linkObject->getName(),
                             linkObject->getLinkStatus(),
                             linkObject->getSize(),
                             linkObject->showFolderIcon());
}

void LinkProcessor::continueOrFinishLinkInfoReq()
{
    if (mCurrentIndex == mLinkList.size())
    {
        emit onLinkInfoRequestFinish();
    }
    else
    {
        requestLinkInfo();
    }
}

void LinkProcessor::createInvalidLinkObject(int index, int error, const QString& name)
{
    if (!isValidIndex(mLinkObjects, index)) { return; }

    mLinkObjects[index] = std::make_shared<LinkInvalid>();

    QString newName = (name.isEmpty()) ?
                          (QCoreApplication::translate(
                              "MegaError",
                              MegaError::getErrorString(error, MegaError::API_EC_IMPORT))) :
                          name;

    mLinkObjects[index]->setName(newName);
    mLinkObjects[index]->setLinkStatus((error == MegaError::API_ETEMPUNAVAIL) ?
                                           linkStatus::WARNING :
                                           linkStatus::FAILED);
}

QString LinkProcessor::getReasonForExpiredLink(MegaRequest* request, MegaError* e)
{
    int error = e->getErrorCode();
    long long userStatus = e->getUserStatus();
    long long linkStatus = e->getLinkStatus();

    // First query the User Status for anomalies
    switch (userStatus)
    {
        case MegaError::UserErrorCode::USER_COPYRIGHT_SUSPENSION:
            return tr("Terms of Service breach");

        case MegaError::UserErrorCode::USER_ETD_SUSPENSION:
            return tr("Link owner terminated");

        case MegaError::UserErrorCode::USER_ETD_UNKNOWN:
        default:
            break;
    }

    // Then query the Link Status next
    switch (linkStatus)
    {
        case MegaError::LinkErrorCode::LINK_DELETED_DOWN:
            return tr("This link has been deleted");

        case MegaError::LinkErrorCode::LINK_DOWN_ETD:
            return tr("Link owner terminated");

        case MegaError::LinkErrorCode::LINK_UNKNOWN:
        case MegaError::LinkErrorCode::LINK_UNDELETED:
        default:
            break;
    }

    // Finally, query the Error Code
    switch (error)
    {
        case MegaError::API_EEXPIRED:
        {
            return tr("This link has expired");
        }

        case MegaError::API_ENOENT:
        {
            return tr("This link has been deleted");
        }

        case MegaError::API_EBLOCKED:
        {
            return tr("Copyright violation");
        }

        default:
            return tr("This link is invalid");
    }
}

unsigned long long LinkProcessor::getAppDataId()
{
    auto data = TransferMetaDataContainer::createImportedLinkTransferMetaData(mDownloadPath);
    return data->getAppId();
}

void LinkProcessor::onRequestFinish(MegaRequest* request, MegaError* e)
{
    const int error = e->getErrorCode();

    switch (request->getType())
    {
    case MegaRequest::TYPE_GET_PUBLIC_NODE:
    {
        if (!isValidIndex(mLinkObjects, mCurrentIndex)) { return; }

        if (error != MegaError::API_OK)
        {
            // Invalid Link
            createInvalidLinkObject(mCurrentIndex, error, getReasonForExpiredLink(request, e));
        }
        else    // API_OK
        {
            MegaNode* node = request->getPublicMegaNode();

            if (!node)
            {
                // Invalid Link
                createInvalidLinkObject(mCurrentIndex, error, getReasonForExpiredLink(request, e));
            }
            else    // Valid Link
            {
                mLinkObjects[mCurrentIndex] = std::make_shared<LinkNode>(mMegaApi,
                                                                         MegaNodeSPtr(node),
                                                                         mLinkList[mCurrentIndex]);
            }
        }

        mCurrentIndex++;
        sendLinkInfoAvailableSignal(mCurrentIndex - 1);
        continueOrFinishLinkInfoReq();
        break;
    }

    // Response to MegaApi::createFolder() request
    case MegaRequest::TYPE_CREATE_FOLDER:
    {
        // The created Node is the importParentNode
        MegaNodeSPtr createdNode (mMegaApi->getNodeByHandle(request->getNodeHandle()));

        if (createdNode)
        {
            setImportParentNode(createdNode);
            mImportParentFolder = createdNode->getHandle();
            addTransfersAndStartIfNotStartedYet(LinkTransferType::IMPORT);
        }
        break;
    }

    case MegaRequest::TYPE_COPY:
    {
        if (error != MegaError::API_OK)
        {
            MegaNode* node = request->getPublicMegaNode();
            QString nodeName =
                (node) ? QString::fromUtf8(node->getName()) : QString::fromUtf8("Invalid Node");
            emit linkCopyErrorDetected(nodeName, error);
        }
        processNextTransfer();
        break;
    }

    case MegaRequest::TYPE_LOGIN:
    {
        if (!isValidIndex(mLinkObjects, mCurrentIndex)) { return; }

        if (error == MegaError::API_OK)
        {
            mMegaApiFolders->fetchNodes(mDelegateListener.get());
        }
        else
        {
            createInvalidLinkObject(mCurrentIndex, error, getReasonForExpiredLink(request, e));
            mCurrentIndex++;
            sendLinkInfoAvailableSignal(mCurrentIndex - 1);
            continueOrFinishLinkInfoReq();
        }
        break;
    }

    case MegaRequest::TYPE_FETCH_NODES:
    {
        if (!isValidIndex(mLinkObjects, mCurrentIndex)) { return; }

        if (error == MegaError::API_OK)
        {
            std::unique_ptr<MegaNode> rootNode(nullptr);
            QString currentStr = mLinkList[mCurrentIndex];
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
                rootNode.reset(mMegaApiFolders->getRootNode());
            }
            else
            {
                QStringList linkparts = currentStr.split(splitSeparator, Qt::KeepEmptyParts);
                MegaHandle handle = MegaApi::base64ToHandle(linkparts.last().toUtf8().constData());
                rootNode.reset(mMegaApiFolders->getNodeByHandle(handle));
            }

            Preferences::instance()->setLastPublicHandle(request->getNodeHandle(), MegaApi::AFFILIATE_TYPE_FILE_FOLDER);
            mega::MegaNode* node = mMegaApiFolders->authorizeNode(rootNode.get());
            mLinkObjects[mCurrentIndex] = std::make_shared<LinkNode>(mMegaApi,
                                                                     MegaNodeSPtr(node),
                                                                     mLinkList[mCurrentIndex]);
        }
        else
        {
            // Invalid Link
            createInvalidLinkObject(mCurrentIndex, error, getReasonForExpiredLink(request, e));
        }

        mCurrentIndex++;
        sendLinkInfoAvailableSignal(mCurrentIndex - 1);
        continueOrFinishLinkInfoReq();

        break;
    }

    default:
        break;
    }
}

void LinkProcessor::addTransfersAndStartIfNotStartedYet(LinkTransferType transferType)
{
    for (int i = 0; i < mLinkObjects.size(); i++)
    {
        if (isSelected(i))
        {
            mTransferQueue.push_back({mLinkObjects[i], transferType});
            processNextTransfer();
        }
    }
}

void LinkProcessor::requestLinkInfo()
{
    if (!isValidIndex(mLinkList, mCurrentIndex)) { return; }

    const QString link = mLinkList[mCurrentIndex];
    if (link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/#F!")) ||
        link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/folder/")))
    {
        std::unique_ptr<char []> authToken(mMegaApi->getAccountAuth());
        if (authToken)
        {
            mMegaApiFolders->setAccountAuth(authToken.get());
        }

        mMegaApiFolders->loginToFolder(link.toUtf8().constData(), mDelegateListener.get());
    }
    else if (link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/collection/")))
    {
        emit requestFetchSetFromLink(link);
    }
    else
    {
        mMegaApi->getPublicNode(link.toUtf8().constData(), mDelegateListener.get());
    }
}

// ----------------------------------------------------------------------------
//
// Callbacks from Sets & Elements
//
// ----------------------------------------------------------------------------
void LinkProcessor::onFetchSetFromLink(const AlbumCollection& collection)
{
    if (mCurrentIndex >= mLinkObjects.size()) { return; }

    mLinkObjects[mCurrentIndex] = std::make_shared<LinkSet>(mMegaApi, collection);
    mCurrentIndex++;

    sendLinkInfoAvailableSignal(mCurrentIndex - 1);
    continueOrFinishLinkInfoReq();
}

// ----------------------------------------------------------------------------
//
// IMPORT
//
// ----------------------------------------------------------------------------

//!
//! \brief LinkProcessor::importLinks
//! \param megaPath: path to import parent destination folder on Cloud Drive
//! \If @megaPath is a valid MegaNode, then:
//! \ * An attempt is made to create import folders for all sets
//! \ * Then if all nodes and sets have import folders, an ImportQueue will
//! \   be created and the actual import will be kicked off
//! \If @megaPath is NOT a valid MegaNode, then:
//!   * An attempt will be made to create an import parent destination folder
//!
void LinkProcessor::importLinks(const QString& megaPath)
{
    MegaNodeSPtr importParentNode(mMegaApi->getNodeByPath(megaPath.toUtf8().constData()));

    if (importParentNode)
    {
        setImportParentNode(importParentNode);
        mImportParentFolder = importParentNode->getHandle();        
        addTransfersAndStartIfNotStartedYet(LinkTransferType::IMPORT);
    }
    else    // No parent import folder, try to create it
    {
        auto rootNode = ((MegaApplication*)qApp)->getRootNode();
        if (!rootNode)
        {
            emit onLinkImportFinish();
            return;
        }

        mMegaApi->createFolder(CommonMessages::getDefaultImportFolderName().toUtf8().constData(),
                               rootNode.get(), mDelegateListener.get());
    }
}

//!
//! \brief LinkProcessor::setImportParentNode
//! \param importParentNode: import parent destination folder on Cloud Drive
//! \Iterates over all NODEs in LinkObjects and store the @importParentNode
//!
void LinkProcessor::setImportParentNode(MegaNodeSPtr importParentNode)
{
    if (!importParentNode) { return; }

    for (const auto& linkObjectPtr: qAsConst(mLinkObjects))
    {
        // Verify that linkObjectPtr is selected and without errors
        if (!linkObjectPtr ||
            !linkObjectPtr->readyForProcessing())
        {
            continue;
        }

        linkObjectPtr->setImportNode(importParentNode);
    }
}

//!
//! \brief SetManager::copyNode
//! \param linkNode: the source node to copy
//! \param importParentNode: import parent destination folder on Cloud Drive
//! \Copies @linkNode to @importParentNode, if a node with the same name and
//! \size doesn't already exist at the destination.
//! \Returns true if a copy/import request was made to SDK, false otherwise
//!
bool LinkProcessor::copyNode(MegaNodeSPtr linkNode, MegaNodeSPtr importParentNode)
{
    if (!linkNode || !importParentNode) { return false; }

    // Returns true if a similar node to @node, with the same name and size,
    // already exists in import folder @importNode. Returns false otherwise.
    auto alreadyExists = [&](MegaNodeSPtr node, MegaNodeSPtr importNode) -> bool {
        std::unique_ptr<MegaNodeList> children(mMegaApi->getChildren(importNode.get()));
        const char* srcName = node->getName();
        const long long srcSize = node->getSize();
        const int nrChildren = children->size();

        for (int i = 0; i < nrChildren; i++)
        {
            MegaNode* child = children->get(i);
            if (!strcmp(srcName, child->getName()) && (srcSize == child->getSize()))
            {
                return true;
            }
        }

        return false;
    };

    if (!alreadyExists(linkNode, importParentNode))
    {
        mMegaApi->copyNode(linkNode.get(), importParentNode.get(), mDelegateListener.get());
        return true;
    }

    return false;
}

MegaHandle LinkProcessor::getImportParentFolder()
{
    return mImportParentFolder;
}

// ----------------------------------------------------------------------------
//
// DOWNLOAD
//
// ----------------------------------------------------------------------------
//!
//! \brief LinkProcessor::downloadLinks
//! \param localPath: download destination folder on local pc
//! \Sets the download paths for all LinkObjects and kicks off the download for each of them
//!
void LinkProcessor::downloadLinks(const QString& localPath)
{
    mDownloadPath = localPath + QDir::separator();
    addTransfersAndStartIfNotStartedYet(LinkTransferType::DOWNLOAD);
}

void LinkProcessor::processNextTransfer()
{
    if (mTransferQueue.isEmpty())
    {
        return;
    }

    LinkTransfer firstItem = mTransferQueue.dequeue();
    auto linkObject = firstItem.linkObject;

    switch (linkObject->getLinkType())
    {
    case linkType::NODE:
    {
        auto linkNodePtr = std::dynamic_pointer_cast<LinkNode>(linkObject);
        if (linkNodePtr)
        {
            if (firstItem.transferType == LinkTransferType::DOWNLOAD)
            {
                mNodesToDownload.append(WrappedNode(WrappedNode::TransferOrigin::FROM_LINK,
                                                    linkNodePtr->getMegaNode(),
                                                    false));

                if (mTransferQueue.isEmpty())
                {
                    startDownload(mNodesToDownload);
                    mNodesToDownload.clear();
                }
                return;
            }
            else if ((firstItem.transferType == LinkTransferType::IMPORT) &&
                       (copyNode(linkNodePtr->getMegaNode(), linkNodePtr->getImportNode())))
            {
                return;
            }
        }
        break;
    }

    case linkType::SET:
    {
        auto set = std::dynamic_pointer_cast<LinkSet>(linkObject);
        if (set)
        {
            if (firstItem.transferType == LinkTransferType::DOWNLOAD)
            {
                // Request to put this set in preview and download all its elements
                emit requestDownloadSet(
                    set->getSet(),
                    mDownloadPath,
                    QList<mega::MegaHandle>()); // Empty list, request all Elements
                return;
            }
            else if (firstItem.transferType == LinkTransferType::IMPORT)
            {
                // Request to put this set in preview and import all its elements
                emit requestImportSet(set->getSet(),
                                      {set->getImportNode()},
                                      QList<mega::MegaHandle>());
                return;
            }
        }
        break;
    }

    default:
        break;
    }

    // There's something wrong with this link, try the next one
    processNextTransfer();
}

//!
//! \brief LinkProcessor::startDownload
//! \param linkNode: MegaNode to download
//! \param localPath: download destination folder on local pc
//! \Requests the SDK to download @linkNode to @localPath
//!
void LinkProcessor::startDownload(const QQueue<WrappedNode>& nodes)
{
    if (nodes.isEmpty())
    {
        return;
    }

    MegaDownloader::DownloadInfo info;
    info.appId = getAppDataId();
    info.checkLocalSpace = false;
    info.downloadQueue = nodes;
    info.path = mDownloadPath;
    mDownloader->processDownloadQueue(info);

    auto appData = TransferMetaDataContainer::getAppDataById<DownloadTransferMetaData>(info.appId);
    if (appData)
    {
        appData->setIsImportedLink();
    }
}

// ----------------------------------------------------------------------------

//!
//! \brief LinkProcessor::onLinkSelected
//! \param index: the index of the link
//! \param selected: whether or not the user selected the link at @index
//! \Called when a user checked or unchecked the box next to a link:
//! \Book keeping: marks the LinkObject at index @index as @selected
//!
void LinkProcessor::onLinkSelected(int index, bool selected)
{
    if (isLinkObjectValid(index))
    {
        mLinkObjects[index]->setSelected(selected);
    }
}

//!
//! \brief LinkProcessor::refreshLinkInfo
//! \I'm not sure why this is required, but copied this
//! \mechanism from the previous developer...
//!
void LinkProcessor::refreshLinkInfo()
{
    for (int i = 0; i < mCurrentIndex; i++)
    {
        sendLinkInfoAvailableSignal(i);
    }
}
