#include "LinkProcessor.h"
#include "Preferences/Preferences.h"
#include "MegaApplication.h"
#include "CommonMessages.h"
#include <QDir>

using namespace mega;

LinkProcessor::LinkProcessor(const QStringList& linkList, MegaApi* megaApi, MegaApi* megaApiFolders)
    : mMegaApi(megaApi)
    , mMegaApiFolders(megaApiFolders)
    , mLinkList(linkList)
    , mImportParentFolder(mega::INVALID_HANDLE)
    , mDelegateListener(std::make_shared<QTMegaRequestListener>(megaApi, this))
    , mDelegateTransferListener(std::make_shared<QTMegaTransferListener>(megaApi, this))
    , mParentHandler(nullptr)
    , mRequestCounter(0)
    , mCurrentIndex(0)
{
    for (int i = 0; i < linkList.size(); i++)
    {
        mLinkObjects.append(std::make_shared<LinkInvalid>());
    }
}

LinkProcessor::~LinkProcessor()
{
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

void LinkProcessor::createInvalidLinkObject(int index, int error)
{
    if (!isValidIndex(mLinkObjects, index)) { return; }

    mLinkObjects[index] = std::make_shared<LinkInvalid>();

    if (error == MegaError::API_ETOOMANY) { return; }

    mLinkObjects[index]->setName(QCoreApplication::translate("MegaError",
                                                             MegaError::getErrorString(error, MegaError::API_EC_IMPORT)));
    mLinkObjects[index]->setLinkStatus((error == MegaError::API_ETEMPUNAVAIL) ?
                                           linkStatus::WARNING :
                                           linkStatus::FAILED);
}

void LinkProcessor::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* e)
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
            createInvalidLinkObject(mCurrentIndex, error);
        }
        else    // API_OK
        {
            MegaNode* node = request->getPublicMegaNode();

            if (!node)
            {
                // Invalid Link
                createInvalidLinkObject(mCurrentIndex, error);
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
        processNextTransfer();
        break;

    case MegaRequest::TYPE_LOGIN:
    {
        if (!isValidIndex(mLinkObjects, mCurrentIndex)) { return; }

        if (error == MegaError::API_OK)
        {
            mRequestCounter++;
            mMegaApiFolders->fetchNodes(this);
        }
        else
        {
            createInvalidLinkObject(mCurrentIndex, error);
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
            createInvalidLinkObject(mCurrentIndex, error);
        }

        mCurrentIndex++;
        sendLinkInfoAvailableSignal(mCurrentIndex - 1);
        continueOrFinishLinkInfoReq();

        break;
    }

    default:
        break;
    }

    mRequestCounter--;
    markForDeletionIfNoMoreRequests();
}

void LinkProcessor::addTransfersAndStartIfNotStartedYet(LinkTransferType transferType)
{
    bool noTransferInProgress = mTransferQueue.isEmpty();

    for (int i = 0; i < mLinkObjects.size(); i++)
    {
        if (isSelected(i))
        {
            mTransferQueue.push_back({mLinkObjects[i], transferType});
        }
    }

    if (noTransferInProgress)
    {
        // Start transfers
        processNextTransfer();
    }
}

void LinkProcessor::markForDeletionIfNoMoreRequests()
{
    // If this instance of LinkProcessor does not need to wait for
    // responses to outstanding requests, then it can be destroyed
    if (!mParentHandler && mRequestCounter == 0)
    {
        deleteLater();
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

        mRequestCounter++;
        mMegaApiFolders->loginToFolder(link.toUtf8().constData(), mDelegateListener.get());
    }
    else if (link.startsWith(Preferences::BASE_URL + QString::fromUtf8("/collection/")))
    {
        mRequestCounter++;
        emit requestFetchSetFromLink(link);
    }
    else
    {
        mRequestCounter++;
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
    // We received a response to a request
    mRequestCounter--;

    if (mCurrentIndex >= mLinkObjects.size()) { return; }

    mLinkObjects[mCurrentIndex] = std::make_shared<LinkSet>(mMegaApi, collection);
    mCurrentIndex++;

    sendLinkInfoAvailableSignal(mCurrentIndex - 1);
    continueOrFinishLinkInfoReq();
    markForDeletionIfNoMoreRequests();
}

void LinkProcessor::onSetDownloadFinished(const QString& setName,
                                          const QStringList& succeededDownloadedElements,
                                          const QStringList& failedDownloadedElements,
                                          const QString& destinationPath)
{
    (void) setName;
    (void) succeededDownloadedElements;
    (void) failedDownloadedElements;
    (void) destinationPath;

    // We received a response to a request
    mRequestCounter--;

    // A public link (to a Set) has been downloaded, proceed to the next one
    processNextTransfer();
    markForDeletionIfNoMoreRequests();
}

void LinkProcessor::onSetImportFinished(const QString& setName,
                                        const QStringList& succeededImportElements,
                                        const QStringList& failedImportElements,
                                        const QStringList& alreadyExistingImportElements,
                                        const SetImportParams& sip)
{
    (void) setName;
    (void) succeededImportElements;
    (void) failedImportElements;
    (void) alreadyExistingImportElements;
    (void) sip;

    // We received a response to a request
    mRequestCounter--;

    // A public link (to a Set) has been imported, proceed to the next one
    processNextTransfer();
    markForDeletionIfNoMoreRequests();
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

        mRequestCounter++;
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

    for (const auto& linkObjectPtr : mLinkObjects)
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
        mRequestCounter++;
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
    setDownloadPaths(localPath);
    addTransfersAndStartIfNotStartedYet(LinkTransferType::DOWNLOAD);
}

//!
//! \brief LinkProcessor::setDownloadPaths
//! \param downloadPath: download destination folder on local pc
//! \Iterates over all LinkObjects and sets the download path for every NODE or SET:
//! \In case of a SET, a directory with the name of the set is created.
//!
void LinkProcessor::setDownloadPaths(const QString& downloadPath)
{
    for (int i = 0; i < mLinkObjects.size(); i++)
    {
        if (!isSelected(i)) { continue; }

        auto linkObject = mLinkObjects[i];

        switch (linkObject->getLinkType())
        {
        case linkType::NODE:
            linkObject->setDownloadPath(downloadPath);
            break;

        case linkType::SET:
        {
            auto set = std::dynamic_pointer_cast<LinkSet>(linkObject);
            if (set)
            {
                QString setDownloadPath = downloadPath + QDir::separator() + set->getName();
                set->setDownloadPath(setDownloadPath);
            }
            break;
        }

        default:
            break;
        }
    }
}

void LinkProcessor::processNextTransfer()
{
    if (mTransferQueue.isEmpty())
    {
        markForDeletionIfNoMoreRequests();
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
                startDownload(linkNodePtr->getMegaNode(), linkNodePtr->getDownloadPath());
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
                mRequestCounter++;
                emit requestDownloadSet(set->getSet(),
                                        set->getDownloadPath(),
                                        QList<mega::MegaHandle>()); // Empty list, request all Elements
                return;
            }
            else if (firstItem.transferType == LinkTransferType::IMPORT)
            {
                // Request to put this set in preview and import all its elements
                mRequestCounter++;
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
void LinkProcessor::startDownload(MegaNodeSPtr linkNode, const QString &localPath)
{
    if (!linkNode || localPath.isEmpty()) { return; }

    const bool startFirst = false;
    QByteArray path = (localPath + QDir::separator()).toUtf8();
    const char* name = nullptr;
    const char* appData = nullptr;
    MegaCancelToken* cancelToken = nullptr; // No cancellation possible
    const bool undelete = false;

    mRequestCounter++;
    mMegaApi->startDownload(linkNode.get(), path.constData(), name, appData, startFirst, cancelToken,
                            MegaTransfer::COLLISION_CHECK_FINGERPRINT,
                            MegaTransfer::COLLISION_RESOLUTION_NEW_WITH_N,
                            undelete,
                            mDelegateTransferListener.get());
}

//!
//! \brief LinkProcessor::onTransferFinish
//! \Callback after downloading a folder, file or set element
//!
void LinkProcessor::onTransferFinish(MegaApi* api, MegaTransfer *transfer, MegaError* error)
{
    (void) api;
    (void) transfer;
    (void) error;

    // We received a response to a request
    mRequestCounter--;

    processNextTransfer();
    markForDeletionIfNoMoreRequests();
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



