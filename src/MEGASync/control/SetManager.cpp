#include "SetManager.h"

#include "MegaApplication.h"
#include "MegaDownloader.h"
#include "RequestListenerManager.h"
#include "TransferMetaData.h"

#include <QDir>

using namespace mega;

SetManager::SetManager(MegaApi* megaApi, MegaApi* megaApiFolders):
    AsyncHandler(),
    mMegaApi(megaApi),
    mMegaApiFolders(megaApiFolders),
    mDownloadedCounter(0),
    mAppId(TransferMetaData::INVALID_ID),
    mDownloader(std::make_shared<MegaDownloader>(megaApi, this)),
    mSetManagerState(SetManagerState::INIT)
{
    // Register for SDK Request callbacks
    mDelegateListener = RequestListenerManager::instance().registerAndGetFinishListener(this);
}

SetManager::~SetManager()
{
    // Stop the AsyncHandler Parent
    stop();
}

//!
//! \brief SetManager::requestFetchSetFromLink
//! \param link: link to the public Set
//! \Adds a request for information about the Set and Elements from link \@link.
//! \This function returns immediately: the request is added to a queue
//! \and handled asynchronously.
void SetManager::requestFetchSetFromLink(const QString& link)
{
    ActionParams action;
    action.type = ActionType::REQUEST_FETCH_SET_FROM_LINK;
    action.link = link;
    mRequestQueue.push(action);
    triggerHandlerThread(true);
}

//!
//! \brief SetManager::requestFetchSetFromLink
//! \param link: link to the public Set
//! \param downloadPath: the full path to the local folder where the Set
//! needs to be downloaded
//! \param elementIDList: list of IDs to Set Elements. If not empty, then only
//! \the Elements from this list will be downloaded. If empty, then all Set
//! \Elements will be downloaded.
//! \Adds a request to download the elements from @elementIDList, which belong
//! \to the Set with link \@link.
//! \This function returns immediately: the request is added to a queue
//! \and handled asynchronously.
void SetManager::requestDownloadSetFromLink(const QString& link,
                                            const QString& downloadPath,
                                            const QList<mega::MegaHandle>& elementHandleList)
{
    ActionParams action;
    action.type = ActionType::REQUEST_DOWNLOAD_SET_FROM_LINK;
    action.link = link;
    action.elementHandleList = elementHandleList;
    action.downloadPath = downloadPath;
    mRequestQueue.push(action);
    triggerHandlerThread(true);
}

//!
//! \brief SetManager::requestDownloadSet
//! \param set: the AlbumCollection to Download
//! \param downloadPath: the full path to the local folder where the Set
//! needs to be downloaded
//! \param elementIDList: list of IDs to Set Elements. If not empty, then only
//! \the Elements from this list will be downloaded. If empty, then all Set
//! \Elements will be downloaded.
//! \Adds a request to download the elements from @elementIDList, which belong
//! \to @set.
//! \This function returns immediately: the request is added to a queue
//! \and handled asynchronously.
void SetManager::requestDownloadSet(const AlbumCollection& set,
                                    const QString& downloadPath,
                                    const QList<mega::MegaHandle>& elementHandleList)
{
    ActionParams action;
    action.type = ActionType::REQUEST_DOWNLOAD_SET;
    action.set = set;
    action.elementHandleList = elementHandleList;
    action.downloadPath = downloadPath;
    mRequestQueue.push(action);
    triggerHandlerThread(true);
}

//!
//! \brief SetManager::requestImportSet
//! \param set: the AlbumCollection to Download
//! \param sip: contains the import parent destination folder on Cloud Drive
//! \param elementIDList: list of IDs to Set Elements. If not empty, then only
//! \the Elements from this list will be imported. If empty, then all Set
//! \Elements will be imported.
//! \Adds a request to import the elements from @elementIDList, which belong
//! \to @set.
//! \This function returns immediately: the request is added to a queue
//! \and handled asynchronously.
void SetManager::requestImportSet(const AlbumCollection& set,
                                  const SetImportParams& sip,
                                  const QList<mega::MegaHandle>& elementHandleList)
{
    ActionParams action;
    action.type = ActionType::REQUEST_IMPORT_SET;
    action.set = set;
    action.elementHandleList = elementHandleList;
    action.importParentNode = sip.importParentNode;
    mRequestQueue.push(action);
    triggerHandlerThread(true);
}

// Callback from AsyncHandler
void SetManager::handleTriggerAction(bool&)
{
    // This function gets called in case of a user request
    // => only process user requests in INIT state, as otherwise
    // there is another request already in progress
    if (mSetManagerState == SetManagerState::INIT)
    {
        handleStates();
    }
}

// ----------------------------------------------------------------------------
//
// STATE MACHINE
//
// ----------------------------------------------------------------------------
void SetManager::handleStates()
{
    auto processInternalQueue = [&](ActionParams& action) -> bool {
        // Check if there is an internal action to handle
        if (mInternalActionQueue.empty())
        {
            // Return to INIT state and check if there is a Request to handle
            resetAndHandleStates();
            return false;
        }

        // Get the first action in the queue
        mInternalActionQueue.pop(action);
        return true;
    };

    ActionParams action = {};
    switch (mSetManagerState)
    {
    case SetManagerState::INIT:
    {
        // Check if there is a User Request to handle
        if (mRequestQueue.empty()) { return; }

        // Get the first action in the queue
        mRequestQueue.pop(action);
        handleStateInit(action);
        break;
    }

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA:
    {
        if (processInternalQueue(action))
        {
            handleStateWaitForPreviewSetToGetData(action);
        }
        break;
    }

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK:
    {
        if (processInternalQueue(action))
        {
            handleStateWaitForPreviewSetToDownloadFromLink(action);
        }
        break;
    }

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION:
    {
        if (processInternalQueue(action))
        {
            handleStateWaitForPreviewSetToDownloadCollection(action);
        }
        break;
    }

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION:
    {
        if (processInternalQueue(action))
        {
            handleStateWaitForPreviewSetToImportCollection(action);
        }
        break;
    }

    default:
        break;
    }
}

//!
//! \brief SetManager::handleStateInit
//! \param action: Set of action parameters to handle INIT state
//!
void SetManager::handleStateInit(const ActionParams& action)
{
    if (mSetManagerState != SetManagerState::INIT) { return; }

    auto handleTransferRequest = [action, this]() -> bool {
        mCurrentDownloadPath = action.downloadPath;
        mCurrentImportParentNode = action.importParentNode;

        // If elementHandleList is empty, ALL Set Elements will be downloaded
        mCurrentElementHandleList = action.elementHandleList;
        mCurrentSet = filterSet(action.set, mCurrentElementHandleList);

        if (!mCurrentSet.link.isEmpty() && mCurrentSet.isComplete())
        {
            mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
            return true;
        }

        return false;
    };

    switch (action.type)
    {
    case ActionType::REQUEST_FETCH_SET_FROM_LINK:
    {
        // Request to put a new Set in preview; reset current values
        mCurrentSet.reset();
        mCurrentSet.link = action.link;

        if (!mCurrentSet.link.isEmpty())
        {            
            mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA;
            mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
        }
    }
    break;

    case ActionType::REQUEST_DOWNLOAD_SET_FROM_LINK:
    {
        if (action.downloadPath.isEmpty()) { return; }

        // If elementHandleList is empty, ALL Set Elements will be downloaded
        mCurrentElementHandleList = action.elementHandleList;
        mCurrentDownloadPath = action.downloadPath;

        // Request to put a new Set in preview; reset current values
        mCurrentSet.reset();
        mCurrentSet.link = action.link;

        if (!mCurrentSet.link.isEmpty())
        {
            mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK;
            mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
        }
    }
    break;

    case ActionType::REQUEST_DOWNLOAD_SET:
    {
        if (action.downloadPath.isEmpty()) { return; }

        if (handleTransferRequest())
        {
            mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION;
        }
    }
    break;

    case ActionType::REQUEST_IMPORT_SET:
    {
        if (!action.importParentNode) { return; }

        if (handleTransferRequest())
        {
            mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION;
        }
    }
    break;

    default:
        break;
    }
}

//!
//! \brief SetManager::handleStateWaitForPreviewSetToGetData
//! \param action: Set of action parameters to handle WAIT_FOR_PREVIEW_SET_TO_GET_DATA state
//!
void SetManager::handleStateWaitForPreviewSetToGetData(const ActionParams& action)
{
    if (mSetManagerState != SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA)
    {
        resetAndHandleStates();
        return;
    }

    switch (action.type)
    {
    case ActionType::HANDLE_SET_IN_PREVIEW_MODE:
        if (!handleFetchPublicSetResponseToGetData())
        {
            resetAndHandleStates();
        }
        break;

    case ActionType::HANDLE_ELEMENT_IN_PREVIEW_MODE:
        if (mCurrentSet.isComplete())
        {
            // Notify observers
            emit onFetchSetFromLink(mCurrentSet);

            // End preview
            mMegaApi->stopPublicSetPreview();

            // Return to INIT state
            resetAndHandleStates();
        }
        break;

    default:
        break;
    }
}

//!
//! \brief SetManager::handleStateWaitForPreviewSetToDownloadFromLink
//! \param action: Set of action parameters to handle WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK
//! state
//!
void SetManager::handleStateWaitForPreviewSetToDownloadFromLink(const ActionParams& action)
{
    if (mSetManagerState != SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK)
    {
        resetAndHandleStates();
        return;
    }

    switch (action.type)
    {
    case ActionType::HANDLE_SET_IN_PREVIEW_MODE:
        if (!handleFetchPublicSetResponseToDownloadFromLink())
        {
            resetAndHandleStates();
        }
        break;

    case ActionType::HANDLE_ELEMENT_IN_PREVIEW_MODE:
        // A new Set Element node has been fetched
        if (!mCurrentSet.nodeList.isEmpty())
        {
            setAppId();
            startDownload(mCurrentSet.nodeList, mCurrentDownloadPath);
            resetAppId();
        }
        break;

    default:
        break;
    }
}

//!
//! \brief SetManager::handleStateWaitForPreviewSetToDownloadCollection
//! \param action: Set of action parameters to handle WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION
//! state
//!
void SetManager::handleStateWaitForPreviewSetToDownloadCollection(const ActionParams& action)
{
    if (mSetManagerState != SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION)
    {
        resetAndHandleStates();
        return;
    }

    if ((action.type != ActionType::HANDLE_SET_IN_PREVIEW_MODE) ||
        (!handleFetchPublicSetResponseToDownloadCollection()))
    {
        resetAndHandleStates();
    }
}

//!
//! \brief SetManager::handleStateWaitForPreviewSetToImportCollection
//! \param action: Set of action parameters to handle WAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION
//! state
//!
void SetManager::handleStateWaitForPreviewSetToImportCollection(const ActionParams& action)
{
    if (mSetManagerState != SetManagerState::WAIT_FOR_PREVIEW_SET_TO_IMPORT_COLLECTION)
    {
        resetAndHandleStates();
        return;
    }

    if ((action.type != ActionType::HANDLE_SET_IN_PREVIEW_MODE) ||
        (!handleFetchPublicSetResponseToImportCollection()))
    {
        resetAndHandleStates();
    }
}

//!
//! \brief SetManager::onRequestFinish
//! \Callback from SDK: response to a request
//!
void SetManager::onRequestFinish(MegaRequest* request, MegaError* error)
{
    switch (request->getType())
    {
    // Response to MegaApi::fetchPublicSet() request
    case MegaRequest::TYPE_FETCH_SET:
        handleFetchPublicSetResponse(request, error);
        break;

    // Response to MegaApi::getPreviewElementNode() request
    case MegaRequest::TYPE_GET_EXPORTED_SET_ELEMENT:
        handleGetPreviewElementNodeResponse(request, error);
        break;

    // Response to MegaApi::createFolder() request
    case MegaRequest::TYPE_CREATE_FOLDER:
        handleCreateFolderResponse(request, error);
        break;

    // Response to MegaApi::copyNode() request
    case MegaRequest::TYPE_COPY:
        handleCopyNodeResponse(request, error);
        break;

    default:
        break;
    }
}

//!
//! \brief SetManager::onTransferFinish
//! \Callback from SDK after downloading a folder, file or set element
//!
void SetManager::onTransferFinish(MegaApi* api, MegaTransfer* transfer, MegaError* error)
{
    Q_UNUSED(api);
    Q_UNUSED(transfer);
    Q_UNUSED(error);

    mDownloadedCounter++;

    // Check if we are waiting for more Elements to download, or if we are done
    if (mDownloadedCounter == mCurrentSet.elementHandleList.size())
    {
        // Reset counter
        mDownloadedCounter = 0;

        // End preview
        mMegaApi->stopPublicSetPreview();

        // Return to INIT state
        resetAndHandleStates();
    }
}

bool SetManager::handleFetchPublicSetResponseToGetData()
{
    // Set is now in Preview: Get Set and its Elements
    if (!getPreviewSetData()) { return false; }

    // Request the nodes of Set Elements
    return getPreviewElementNodes();
}

bool SetManager::handleFetchPublicSetResponseToDownloadFromLink()
{
    // Set is now in Preview: Get Set and its Elements
    if (!getPreviewSetData()) { return false; }

    // All Elements will be downloaded in a folder with the name of the Set
    mCurrentDownloadPath = mCurrentDownloadPath + QDir::separator() + mCurrentSet.name;
    createDirectory(mCurrentDownloadPath);

    // Request the nodes of Set Elements
    return getPreviewElementNodes();
}

bool SetManager::handleFetchPublicSetResponseToDownloadCollection()
{
    // Set is now in Preview
    // All Elements will be downloaded in a folder with the name of the Set
    if (!createDirectory(mCurrentDownloadPath)) { return false; }

    // The requested Set has been put in preview, so download of (selected) Elements can start
    setAppId();
    startDownload(mCurrentSet.nodeList, mCurrentDownloadPath);
    resetAppId();

    return true;
}

bool SetManager::handleFetchPublicSetResponseToImportCollection()
{
    // Set is now in Preview, create import folder in the Cloud Drive
    mMegaApi->createFolder(mCurrentSet.name.toUtf8().constData(),
                           mCurrentImportParentNode.get(), mDelegateListener.get());
    return true;
}

// Response to MegaApi::fetchPublicSet() request
void SetManager::handleFetchPublicSetResponse(MegaRequest* request, MegaError* error)
{
    // Verify Precondition
    if (!request || !error ||
        (request->getType() != MegaRequest::TYPE_FETCH_SET) ||
        (error->getErrorCode() != MegaError::API_OK))
    {
        resetAndHandleStates();
        return;
    }

    // Delegate to State Machine
    ActionParams action;
    action.type = ActionType::HANDLE_SET_IN_PREVIEW_MODE;
    mInternalActionQueue.push(action);
    handleStates();
}

void SetManager::handleGetPreviewElementNodeResponse(MegaRequest* request, MegaError* error)
{
    // Verify Precondition
    if (!request || !error ||
        (request->getType() != MegaRequest::TYPE_GET_EXPORTED_SET_ELEMENT) ||
        (error->getErrorCode() != MegaError::API_OK))
    {
        resetAndHandleStates();
        return;
    }

    // Do not expose the raw pointer in a variable, to prevent 'double-free' vulnerability
    MegaNodeSPtr nodeSPtr(request->getPublicMegaNode());
    mCurrentSet.nodeList.push_back(WrappedNode(WrappedNode::FROM_LINK, nodeSPtr, false));

    // Delegate to State Machine
    ActionParams action;
    action.type = ActionType::HANDLE_ELEMENT_IN_PREVIEW_MODE;
    mInternalActionQueue.push(action);
    handleStates();
}

void SetManager::handleCreateFolderResponse(MegaRequest* request, MegaError* error)
{
    // Verify Precondition
    if (!request || !error || (request->getType() != MegaRequest::TYPE_CREATE_FOLDER))
    {
        resetAndHandleStates();
        return;
    }

    // Folder creation was successfull, if status code OK was returned OR if the folder
    // already exists at the target destination
    int errorCode = error->getErrorCode();
    if (errorCode != MegaError::API_OK &&
        !((request->getType() == MegaRequest::TYPE_CREATE_FOLDER) && (errorCode == MegaError::API_EEXIST)))
    {
        resetAndHandleStates();
        return;
    }

    MegaNodeSPtr createdNode (mMegaApi->getNodeByHandle(request->getNodeHandle()));
    if (!createdNode)
    {
        resetAndHandleStates();
        return;
    }

    // Copy (import) all Elements to the Cloud Drive
    for (const auto& wrappedNode: mCurrentSet.nodeList)
    {
        if (!copyNode(wrappedNode.getMegaNode(), createdNode))
        {
            // The Element node already exists in the target destination
            mAlreadyExistingImportElements.push_back(
                QString::fromUtf8(wrappedNode.getMegaNode()->getName()));
        }
    }

    // It could be, that all Elements already exist in the target destination
    // In that case, import has been completed
    checkandHandleFinishedImport();
}

void SetManager::handleCopyNodeResponse(MegaRequest* request, MegaError* error)
{
    // Verify Precondition
    if (!request || !error || (request->getType() != MegaRequest::TYPE_COPY))
    {
        resetAndHandleStates();
        return;
    }

    // Do not expose the raw pointer in a variable, to prevent 'double-free' vulnerability
    MegaNodeSPtr nodeSPtr(request->getPublicMegaNode());
    QString elementNodeName = QString::fromUtf8(nodeSPtr->getName());

    if (error->getErrorCode() == MegaError::API_OK)
    {
        mSucceededImportElements.push_back(elementNodeName);
    }
    else
    {
        mFailedImportElements.push_back(elementNodeName);
    }

    checkandHandleFinishedImport();
}

//!
//! \brief SetManager::checkandHandleFinishedImport
//! \Checks if all Set Elements have been imported (whether successfull, failed or already existing)
//! \If import of teh current Set has been completed, then observers are notified, the Set preview
//! \will stop and all variables will be reset
void SetManager::checkandHandleFinishedImport()
{
    // Check if we are waiting for more Elements to import, or if we are done
    int nrElementsToImport = mCurrentSet.elementHandleList.size();
    int nrImportedElements = mSucceededImportElements.size() + mFailedImportElements.size() + mAlreadyExistingImportElements.size();

    if (nrImportedElements == nrElementsToImport)
    {
        // Notify observers about the successful import
        emit onSetImportFinished(mCurrentSet.name,
                                 mSucceededImportElements,
                                 mFailedImportElements,
                                 mAlreadyExistingImportElements,
                                 {mCurrentImportParentNode});

        // End preview
        mMegaApi->stopPublicSetPreview();

        // Return to INIT state
        resetAndHandleStates();
    }
}

void SetManager::setAppId()
{
    auto data = TransferMetaDataContainer::createTransferMetaData<DownloadTransferMetaData>(
        mCurrentDownloadPath);
    mAppId = data->getAppId();
}

void SetManager::resetAppId()
{
    mAppId = TransferMetaData::INVALID_ID;
}

//!
//! \brief SetManager::createDirectory
//! \param path: full path to the folder that needs to be created.
//! \Creates local directory @path if it does not exist
bool SetManager::createDirectory(const QString& path)
{
    if (path.isEmpty()) { return false; }

    QDir directory(path);
    return directory.mkpath(QString::fromUtf8("."));
}

//!
//! \brief SetManager::getPreviewSetData
//! \Gets the name of the Set that is currently in preview
//! \Creates a list of Set Element handles:
//! \If the user did not specify a selection of IDs of requested Elements,
//! \then all Elements of this Set are included.
bool SetManager::getPreviewSetData()
{
    MegaSet* set = mMegaApi->getPublicSetInPreview();
    if (!set) { return false; }

    mCurrentSet.name = QString::fromUtf8(set->name());
    delete set;

    MegaSetElementList* elements = mMegaApi->getPublicSetElementsInPreview();
    if (!elements) { return false; }

    const unsigned int nrElements = elements->size();
    for (unsigned int i = 0; i < nrElements; i++)
    {
        MegaHandle handle = elements->get(i)->id();

        // Only process Elements that were specifically requested by the user:
        // If no specific Elements were requested, then request all of them
        if (mCurrentElementHandleList.isEmpty() ||
            mCurrentElementHandleList.contains(handle))
        {
            // Avoid duplicates
            if (!mCurrentSet.elementHandleList.contains(handle))
            {
                mCurrentSet.elementHandleList.push_back(handle);
            }
        }
     }
    delete elements;

    return true;
}

//!
//! \brief SetManager::filterSet
//! \param srcSet: The AlbumCollection that needs to be filtered.
//! \param elementHandleList: The IDs (handles) of the Elements that should be kept
//! \Creates a filtered subset of @srcSet: If an element is present in @elementHandleList,
//! \then this Element (its handle and node) can be kept.
//! \NOTE: If @elementHandleList is empty, then there is NO filtering and the complete @srcSet is returned
AlbumCollection SetManager::filterSet(const AlbumCollection& srcSet, const QList<mega::MegaHandle>& elementHandleList)
{
    AlbumCollection dstSet;
    if (!srcSet.isComplete()) { return dstSet; }    // srcSet is corrupt
    if (elementHandleList.isEmpty()) { return srcSet; } // No filtering

    dstSet.name = srcSet.name;
    dstSet.link = srcSet.link;

    const int nrElements = srcSet.elementHandleList.size();
    for (int i = 0; i < nrElements; i++)
    {
        MegaHandle handle = srcSet.elementHandleList[i];
        if (elementHandleList.contains(handle))
        {
            // This handle and its corresponding node can be included
            dstSet.elementHandleList.push_back(handle);
            dstSet.nodeList.push_back(srcSet.nodeList[i]);
        }
    }

    return dstSet;
}

//!
//! \brief SetManager::getPreviewElementNodes
//! \Requests the nodes of the Set Elements with an ID in @mCurrentSet.elementIDList.
//! NOTE: Caller must ensure that the Set in preview, otherwise the request will fail.
bool SetManager::getPreviewElementNodes()
{
    if (mCurrentSet.elementHandleList.isEmpty()) { return false; }

    // Fetch all Elements
    for (const auto& handle : mCurrentSet.elementHandleList)
    {
        mMegaApi->getPreviewElementNode(handle, mDelegateListener.get());
    }

    return true;
}

void SetManager::reset()
{
    mSetManagerState = SetManagerState::INIT;
    mCurrentSet.reset();
    mCurrentElementHandleList.clear();
    mCurrentDownloadPath = QString::fromUtf8("");
    mCurrentImportParentNode = nullptr;
    mFailedDownloadedElements.clear();
    mSucceededDownloadedElements.clear();
    mSucceededImportElements.clear();
    mFailedImportElements.clear();
    mAlreadyExistingImportElements.clear();
}

void SetManager::resetAndHandleStates()
{
    reset();
    handleStates();
}

void SetManager::startDownload(QQueue<WrappedNode>& nodes, const QString& localPath)
{
    if (nodes.isEmpty() || localPath.isEmpty())
    {
        if (nodes.isEmpty())
        {
            MegaApi::log(MegaApi::LOG_LEVEL_ERROR,
                         QString::fromUtf8("Download set %1 failed. Nodes not available.")
                             .arg(localPath)
                             .toUtf8()
                             .constData());
        }
        else if (localPath.isEmpty())
        {
            MegaApi::log(
                MegaApi::LOG_LEVEL_ERROR,
                QString::fromUtf8("Download set failed. Local path not available. Nodes number: %1")
                    .arg(QString::number(nodes.size()))
                    .toUtf8()
                    .constData());
        }

        return;
    }

    MegaDownloader::DownloadInfo info;
    info.appId = mAppId;
    info.checkLocalSpace = false;
    info.downloadQueue = nodes;
    info.path = localPath + QDir::separator();
    mDownloader->processDownloadQueue(info);

    nodes.clear();
}

//!
//! \brief SetManager::copyNode
//! \param linkNode: the source node to copy
//! \param importParentNode: import parent destination folder on Cloud Drive
//! \Copies @linkNode to @importParentNode, if a node with the same name and
//! \size doesn't already exist at the destination.
//! \Returns true if a copy/import request was made to SDK, false otherwise
//!
bool SetManager::copyNode(MegaNode* linkNode, MegaNodeSPtr importParentNode)
{
    if (!linkNode || !importParentNode) { return false; }

    // Returns true if a similar node to @node, with the same name and size,
    // already exists in import folder @importNode. Returns false otherwise.
    auto alreadyExists = [&](MegaNode* node, MegaNodeSPtr importNode) -> bool
    {
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
        mMegaApi->copyNode(linkNode, importParentNode.get(), mDelegateListener.get());
        return true;
    }

    return false;
}
