#include "SetManager.h"
#include <QDir>

using namespace mega;

SetManager::SetManager(MegaApi* megaApi, MegaApi* megaApiFolders) :
    AsyncHandler(),
    mMegaApi(megaApi),
    mMegaApiFolders(megaApiFolders),
    mDelegateListener(std::make_shared<QTMegaRequestListener>(megaApi, this)),
    mDelegateTransferListener(std::make_shared<QTMegaTransferListener>(megaApi, this)),
    mSetManagerState(SetManagerState::INIT),
    mNrDownloadedElements(0)
{
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
    struct CallbackAction action;
    action.type = CallbackType::REQUEST_FETCH_SET_FROM_LINK;
    action.link = link;
    triggerHandlerThread(action);
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
    struct CallbackAction action;
    action.type = CallbackType::REQUEST_DOWNLOAD_SET_FROM_LINK;
    action.link = link;
    action.elementHandleList = elementHandleList;
    action.downloadPath = downloadPath;
    triggerHandlerThread(action);
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
    struct CallbackAction action;
    action.type = CallbackType::REQUEST_DOWNLOAD_SET;
    action.set = set;
    action.elementHandleList = elementHandleList;
    action.downloadPath = downloadPath;
    triggerHandlerThread(action);
}

void SetManager::onRequestFinish(MegaApi*, MegaRequest* request, MegaError* error)
{
    if (error->getErrorCode() != MegaError::API_OK)
    {
        MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "A Request Finished Unsuccessfully");
        reset();
        return;
    }

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

    default:
        break;
    }
}

//!
//! \brief LinkProcessor::onTransferFinish
//! \Callback after downloading a folder, file or set element
//!
void SetManager::onTransferFinish(MegaApi* api, MegaTransfer* transfer, MegaError* error)
{
    (void) api;
    (void) transfer;

     if (error->getErrorCode() != MegaError::API_OK)
    {
        // Something is wrong: abort downloads by returning to INIT state
        reset();
        return;
    }

    // Successful download
    int nrElementsToDownload = mCurrentSet.elementHandleList.size();

    if (++mNrDownloadedElements == nrElementsToDownload)
    {
        // Notify observers about the successful download: pass Set name and nr downloaded Elements
        emit onSetDownloaded(mCurrentSet.name,
                             static_cast<int>(nrElementsToDownload));

        // End preview
        mMegaApi->stopPublicSetPreview();

        // Return to INIT state
        reset();
    }
}

void SetManager::handleTriggerAction(struct CallbackAction& action)
{
    QMutexLocker locker(&mSetManagerStateMutex);
    if (mSetManagerState == SetManagerState::INIT)
    {
        switch (action.type)
        {
        case CallbackType::REQUEST_FETCH_SET_FROM_LINK:
        {
            // Request to put a new Set in preview; reset current values
            mCurrentSet.reset();
            mCurrentSet.link = action.link;

            if (!mCurrentSet.link.isEmpty())
            {
                mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
                mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA;
            }
        }
            break;

        case CallbackType::REQUEST_DOWNLOAD_SET_FROM_LINK:
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
                mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
                mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK;
            }
        }
        break;

        case CallbackType::REQUEST_DOWNLOAD_SET:
        {
            if (action.downloadPath.isEmpty()) { return; }

            mCurrentSet = action.set;

            // If elementHandleList is empty, ALL Set Elements will be downloaded
            mCurrentElementHandleList = action.elementHandleList;
            mCurrentDownloadPath = action.downloadPath;

            if (!mCurrentSet.link.isEmpty())
            {
                mMegaApi->fetchPublicSet(mCurrentSet.link.toUtf8().constData(), mDelegateListener.get());
                mSetManagerState = SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION;
            }
        }
        break;

        default:
            break;
        }
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

    mCurrentDownloadPath = mCurrentDownloadPath + QDir::separator() + mCurrentSet.name;
    createDirectory(mCurrentDownloadPath);

    // Request the nodes of Set Elements
    return getPreviewElementNodes();
}

bool SetManager::handleFetchPublicSetResponseToDownloadCollection()
{
    // Set is now in Preview: check if all Elements require downloading or only a subset
    mCurrentSet = filterSet(mCurrentSet, mCurrentElementHandleList);

    if (!mCurrentSet.isComplete()) { return false; }

    // All Elements will be downloaded in a folder with the name of the Set
    mCurrentDownloadPath = mCurrentDownloadPath + QDir::separator() + mCurrentSet.name;
    if (!createDirectory(mCurrentDownloadPath)) { return false; }

    // The requested Set has been put in preview, so download of (selected) Elements can start
    for (MegaNodeSPtr node : mCurrentSet.nodeList)
    {
        startDownload(node.get(), mCurrentDownloadPath);
    }

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
        reset();
        return;
    }

    switch (mSetManagerState)
    {
    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA:
        if (!handleFetchPublicSetResponseToGetData())
        {
            reset();
        }
        break;

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK:
        if (!handleFetchPublicSetResponseToDownloadFromLink())
        {
            reset();
        }
        break;

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION:
        if (!handleFetchPublicSetResponseToDownloadCollection())
        {
            reset();
        }
        break;

    default:
        break;
    }
}

void SetManager::handleGetPreviewElementNodeResponse(MegaRequest* request, MegaError* error)
{
    // Verify Precondition
    if (!request || !error ||
        (request->getType() != MegaRequest::TYPE_GET_EXPORTED_SET_ELEMENT) ||
        (error->getErrorCode() != MegaError::API_OK))
    {
        reset();
        return;
    }

    // Do not expose the raw pointer in a variable, to prevent 'double-free' vulnerability
    MegaNodeSPtr nodeSPtr(request->getPublicMegaNode());
    mCurrentSet.nodeList.push_back(nodeSPtr);

    switch (mSetManagerState)
    {
    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_GET_DATA:
        if (mCurrentSet.isComplete())
        {
            // Notify observers
            emit onFetchSetFromLink(mCurrentSet);

            // End preview
            mMegaApi->stopPublicSetPreview();

            // Return to INIT state
            mSetManagerState = SetManagerState::INIT;
        }
        break;

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_FROM_LINK:
        startDownload(mCurrentSet.nodeList.last().get(), mCurrentDownloadPath);
        break;

    case SetManagerState::WAIT_FOR_PREVIEW_SET_TO_DOWNLOAD_COLLECTION:
    default:
        break;
    }
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
    mNrDownloadedElements = 0;

}

void SetManager::startDownload(MegaNode* linkNode, const QString& localPath)
{
    if (!linkNode || localPath.isEmpty()) { return; }

    const bool startFirst = false;
    QByteArray path = (localPath + QDir::separator()).toUtf8();
    const char* name = nullptr;
    const char* appData = nullptr;
    MegaCancelToken* cancelToken = nullptr; // No cancellation possible
    const bool undelete = false;
    mMegaApi->startDownload(linkNode, path.constData(), name, appData, startFirst, cancelToken,
                            MegaTransfer::COLLISION_CHECK_FINGERPRINT,
                            MegaTransfer::COLLISION_RESOLUTION_NEW_WITH_N,
                            undelete,
                            mDelegateTransferListener.get());
}
