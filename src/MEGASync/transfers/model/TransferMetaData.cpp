#include <TransferMetaData.h>

#include <QDir>

#include <MegaApplication.h>
#include <Preferences.h>
#include <Notificator.h>
#include <MegaNodeNames.h>

#include <megaapi.h>
#include <mega/types.h>


//CLASS TRANSFERMETADATAITEMID
bool TransferMetaDataItemId::operator==(const TransferMetaDataItemId& item) const
{
    if((item.tag < 0) && (item.handle == mega::INVALID_HANDLE))
    {
        return false;
    }
    else
    {
        return (tag > 0 && (item.tag == tag)) || (handle != mega::INVALID_HANDLE && (item.handle == handle));
    }
}

bool TransferMetaDataItemId::operator<(const TransferMetaDataItemId &item) const
{
    if(tag > 0 && item.tag > 0)
    {
        return tag < item.tag;
    }
    else if(handle != mega::INVALID_HANDLE && item.handle != mega::INVALID_HANDLE)
    {
        return handle > item.handle;
    }

    return false;
}

/////////////////////////////////

TransferMetaData::TransferMetaData(int direction, unsigned long long id)
    : mInitialPendingTransfers(-1), mInitialPendingFolderTransfersFromOtherSession(0), mTransferDirection(direction), mCreateRootFolder(false),
      mAppId(id), mCreatedFromOtherSession(false), mNotification(nullptr), mNonExistsFailAppId(0)
{
}

TransferMetaData::TransferMetaData()
    : mInitialPendingTransfers(-1), mInitialPendingFolderTransfersFromOtherSession(0), mTransferDirection(-1), mCreateRootFolder(false),
      mAppId(0), mCreatedFromOtherSession(false), mNotification(nullptr), mNonExistsFailAppId(0)
{
}

bool TransferMetaData::isRetriedFolder(mega::MegaTransfer* transfer)
{
    if(transfer->getNodeHandle() != mega::INVALID_HANDLE)
    {
        TransferMetaDataItemId id(-1, transfer->getNodeHandle());
        return mFolders.contains(id);
    }
    else
    {
        foreach(auto folder, mFolders)
        {
            if(folder->id.path == QString::fromUtf8(transfer->getPath()))
            {
                mFolders.remove(folder->id);
                //Update id with new tag (retried tag)
                TransferMetaDataItemId id(transfer->getTag(), folder->id.handle, folder->id.name, folder->id.path);
                mFolders.insert(id, folder);
                return true;
            }
        }
    }

    return false;
}

void TransferMetaData::remove()
{
    TransferMetaDataContainer::removeAppData(mAppId);
}

bool TransferMetaData::isNonExistData() const
{
    return !mFiles.nonExistFailedTransfers.isEmpty();
}

bool TransferMetaData::finish(mega::MegaTransfer *transfer, mega::MegaError* e)
{
    auto nonExistError = [this](mega::MegaTransfer* transfer, mega::MegaError* e) -> bool {
        if(e->getErrorCode() == mega::MegaError::API_EARGS
                || e->getErrorCode() == mega::MegaError::API_ENOENT || e->getErrorCode() == mega::MegaError::API_EREAD)
        {
            if(!isNonExistTransfer(transfer))
            {
                return false;
            }

            std::shared_ptr<TransferMetaData> nonExistData(nullptr);

            if(mNonExistsFailAppId == 0)
            {
                nonExistData = createNonExistData();
                mNonExistsFailAppId = nonExistData->getAppId();
            }

            return true;
        }

        return false;
    };

    if(transfer->isFolderTransfer())
    {
        TransferMetaDataItemId id(transfer->getTag(), transfer->getNodeHandle(), QString::fromUtf8(transfer->getFileName()), QString::fromUtf8(transfer->getPath()));

        auto value = mFolders.value(id);
        if(value)
        {
            mFolders.remove(id);

            value->id = id;
            TransferData::TransferState state(TransferData::TRANSFER_NONE);

            if(transfer->getState() == mega::MegaTransfer::STATE_FAILED
                    || e->getErrorCode() == mega::MegaError::API_EINCOMPLETE)
            {
                state = TransferData::TRANSFER_FAILED;
                value->failedTransfer = std::shared_ptr<mega::MegaTransfer>(transfer->copy());
            }
            else
            {
                state = TransferData::TRANSFER_COMPLETED;
            }


            if(!nonExistError(transfer, e))
            {
                //The folder has finished but it is empty, so it is added to the empty folders list
                if(value->files.size() == 0)
                {
                    mEmptyFolders.insertItem(state, value);
                }
                //Update Key
                else
                {
                    mFolders.insert(id, value);
                }
            }
            else
            {
                auto nonExistData = TransferMetaDataContainer::getAppData(mNonExistsFailAppId);
                if(nonExistData)
                {
                    if(value->files.size() == 0)
                    {
                        value->state = TransferData::TRANSFER_FAILED;
                        nonExistData->mEmptyFolders.nonExistFailedTransfers.insert(id, value);
                    }
                    else
                    {
                        nonExistData->mFolders.insert(id, value);
                    }
                }
            }
        }
    }
    else
    {
        TransferMetaDataItemId id(transfer->getTag(), transfer->getNodeHandle(), QString::fromUtf8(transfer->getFileName()), QString::fromUtf8(transfer->getPath()));

        auto item = mFiles.pendingTransfers.value(id);
        if(item)
        {
            item->id = id;
            mFiles.removeItem(item);

            TransferData::TransferState state = TransferData::convertState(transfer->getState());
            if(state == TransferData::TRANSFER_FAILED)
            {
                item->failedTransfer = std::shared_ptr<mega::MegaTransfer>(transfer->copy());
            }

            if(transfer->getFolderTransferTag() > 0)
            {
                TransferMetaDataItemId topLevelFolderId(transfer->getFolderTransferTag(), mega::INVALID_HANDLE);
                auto topLevelFolder = mFolders.value(topLevelFolderId, nullptr);
                if(topLevelFolder)
                {
                    item->topLevelFolderId = topLevelFolder->id;
                }
            }

            item->folderId.handle = transfer->getParentHandle();

            if(!nonExistError(transfer, e))
            {
                mFiles.insertItem(state, item);
            }
            else
            {
                auto nonExistData = TransferMetaDataContainer::getAppData(mNonExistsFailAppId);
                if(nonExistData)
                {
                    item->state = TransferData::TRANSFER_FAILED;
                    nonExistData->mFiles.nonExistFailedTransfers.insert(item->id, item);
                }
            }
        }
    }

    decreaseInitialPendingTransfers(transfer);

    checkAndSendNotification();

    return false;
}

void TransferMetaData::addInitialPendingTransfer()
{
    //It is init with -1
    if(mInitialPendingTransfers < 0)
    {
        mInitialPendingTransfers = 1;
    }
    else
    {
        mInitialPendingTransfers++;
    }
}

void TransferMetaData::decreaseInitialPendingTransfers(mega::MegaTransfer *transfer)
{
    //Reduce it only for top level transfers, as the initialPendingTransfers only counts files and folders (not files in folders)
    if(transfer->getFolderTransferTag() <= 0 && mInitialPendingTransfers > 0)
    {
        mInitialPendingTransfers--;
    }
    //If the transfermetadata has been created from other session from a folder download/upload
    //Reduce the mInitialPendingTransfers (which will be maximum 1, the folder) when all the files have finished
    else if(mCreatedFromOtherSession && mFiles.pendingTransfers.isEmpty())
    {
        if(transfer->getFolderTransferTag() > 0)
        {
            mInitialPendingTransfers -= mInitialPendingFolderTransfersFromOtherSession;
        }
    }
}

bool TransferMetaData::isSingleTransfer() const
{
    if(isNonExistData())
    {
        return mFiles.nonExistFailedTransfers.size() == 1;
    }

    return (mFiles.completedTransfers.size() + mFiles.failedTransfers.size() + getTotalEmptyFolders()) == 1;
}

int TransferMetaData::getTotalFiles() const
{
    return mFiles.size();
}

int TransferMetaData::getTotalEmptyFolders() const
{
    return mEmptyFolders.size();
}

bool TransferMetaData::isRetriedFolder(const TransferMetaDataItemId &folderId) const
{
    return mFolders.contains(folderId);
}

QList<TransferMetaDataItemId> TransferMetaData::getFileFailedTagsFromFolderTag(const TransferMetaDataItemId& folderId) const
{
    QList<TransferMetaDataItemId> ids;
    foreach(auto& file, mFiles.failedTransfers)
    {
        if(file->topLevelFolderId == folderId)
        {
            ids.append(file->id);
        }
    }

    return ids;
}

int TransferMetaData::getFileTransfersOK() const
{
    return mFiles.completedTransfers.size();
}

int TransferMetaData::getFileTransfersFailed() const
{
    return mFiles.failedTransfers.size() + mFiles.nonExistFailedTransfers.size();
}

void TransferMetaData::getFileTransferFailedTags(QList<std::shared_ptr<TransferMetaDataItem>>& files, QList<TransferMetaDataItemId>& folders) const
{
    if(isNonExistData())
    {
        //Retry only the non exist
        foreach(auto file, mFiles.nonExistFailedTransfers)
        {
            files.append(file);
        }
    }
    else
    {
        foreach(auto file, mFiles.failedTransfers)
        {
            files.append(file);
            //For future folder retry
            //        if(!file->topLevelFolderId.isValid())
            //        {
            //            if(!files.contains(file))
            //            {
            //                files.append(file);
            //            }
            //        }
            //        else if(!folders.contains(file->topLevelFolderId))
            //        {
            //            folders.append(file->topLevelFolderId);
            //        }
        }
    }
}

int TransferMetaData::getFileTransfersCancelled() const
{
    return mFiles.cancelledTransfers.size();
}

TransferMetaDataItemId TransferMetaData::getFirstTransferIdByState(TransferData::TransferState state) const
{
    TransferMetaDataItemId id = mFiles.getFirstTransferIdByState(state);
    if(!id.isValid())
    {
        id = mEmptyFolders.getFirstTransferIdByState(state);
    }

    return id;
}

QList<TransferMetaDataItemId> TransferMetaData::getTransferIdsByState(TransferData::TransferState state) const
{
    QList<TransferMetaDataItemId> ids = mFiles.getTransferIdsByState(state);
    if(ids.isEmpty())
    {
        ids = mEmptyFolders.getTransferIdsByState(state);
    }

    return ids;
}

int TransferMetaData::getEmptyFolderTransfersOK() const
{
    return getEmptyFolders(mEmptyFolders.completedTransfers);
}

int TransferMetaData::getEmptyFolderTransfersFailed() const
{
    return getEmptyFolders(mEmptyFolders.failedTransfers);
}

int TransferMetaData::getEmptyFolders(const QMap<TransferMetaDataItemId, std::shared_ptr<TransferMetaDataFolderItem>>& folders) const
{
    auto counter(0);

    foreach(auto& folder, folders)
    {
       folder->files.size() == 0 ? counter++ : counter;
    }

    return counter;
}

void TransferMetaData::setCreatedFromOtherSession()
{
    mCreatedFromOtherSession = true;
}

void TransferMetaData::addFile(int tag)
{
    TransferMetaDataItemId id(tag, mega::INVALID_HANDLE);
    auto fileItem = std::make_shared<TransferMetaDataItem>(id);
    mFiles.pendingTransfers.insert(id, fileItem);

    if(mCreatedFromOtherSession)
    {
        addInitialPendingTransfer();
    }
}

void TransferMetaData::addFileFromFolder(int folderTag, int fileTag)
{
    TransferMetaDataItemId fileId(fileTag, mega::INVALID_HANDLE);
    auto fileItem = std::make_shared<TransferMetaDataItem>(fileId);
    fileItem->topLevelFolderId.tag = folderTag;
    mFiles.pendingTransfers.insert(fileId, fileItem);

    TransferMetaDataItemId folderId(folderTag, mega::INVALID_HANDLE);
    auto folderItem = mFolders.value(folderId, nullptr);
    if(!folderItem)
    {
        folderItem = std::make_shared<TransferMetaDataFolderItem>(folderId);
        mFolders.insert(folderId,folderItem);

        if(mCreatedFromOtherSession)
        {
            mInitialPendingFolderTransfersFromOtherSession++;
            addInitialPendingTransfer();
        }
    }

    folderItem->files.pendingTransfers.insert(fileId, fileItem);
}

void TransferMetaData::checkAndSendNotification()
{
    if (mInitialPendingTransfers == 0 && mFiles.pendingTransfers.isEmpty())
    {
        //If all the transfers have been cancelled, do not show any notification
        if (Preferences::instance()->isNotificationEnabled(Preferences::NotificationsTypes::COMPLETED_UPLOADS_DOWNLOADS))
        {
            std::shared_ptr<TransferMetaData> nonExistData(nullptr);
            if(mNonExistsFailAppId != 0)
            {
                nonExistData = TransferMetaDataContainer::getAppData(mNonExistsFailAppId);
            }

            QList<unsigned long long> appIds;
            if((getFileTransfersCancelled() != getTotalFiles() || getEmptyFolderTransfersOK() > 0)
                && (!isNonExistData() && (mFiles.size() == 0 || mFiles.hasChanged())))
            {
                mFiles.setHasChanged(false);
                appIds.append(getAppId());
            }
            else if(!nonExistData)
            {
                TransferMetaDataContainer::removeAppData(getAppId());
            }

            if(nonExistData)
            {
                //Do not overwrite non exist files
                auto nonExistFiles = nonExistData->mFiles.nonExistFailedTransfers;
                nonExistData->mFiles = mFiles;
                nonExistData->mFiles.nonExistFailedTransfers = nonExistFiles;

                nonExistData->mEmptyFolders = mEmptyFolders;
                nonExistData->mFolders = mFolders;

                nonExistData->mFiles.setHasChanged(false);

                //on macOS, notifications start from the end of the list,
                //so the "non exist" notification is prepended to be shown at the end
#ifdef __APPLE__
                appIds.prepend(mNonExistsFailAppId);
#else
                appIds.append(mNonExistsFailAppId);
#endif
            }

            if(!appIds.isEmpty())
            {
                //This method is called from the transfer model secondary thread, but the notification should be called from the main thread
                Utilities::queueFunctionInAppThread([this, appIds]()
                {
                    //Transfers finished and ready, show notification
                    foreach(auto& appId, appIds)
                    {
                        MegaSyncApp->showNotificationFinishedTransfers(appId);
                    }
                });
            }
        }
        else
        {
            TransferMetaDataContainer::removeAppData(getAppId());
        }
    }
}

bool TransferMetaData::isDownload() const
{
    return mTransferDirection != mega::MegaTransfer::TYPE_UPLOAD;
}

bool TransferMetaData::isUpload() const
{
    return mTransferDirection == mega::MegaTransfer::TYPE_UPLOAD;
}

bool TransferMetaData::allHaveFailed() const
{
    return ((getFileTransfersFailed() == getTotalFiles())
                && (getEmptyFolderTransfersFailed() == getTotalEmptyFolders()))
            || isNonExistData();
}

bool TransferMetaData::someHaveFailed() const
{
    return ((getFileTransfersFailed() > 0)
            || (getEmptyFolderTransfersFailed() > 0));
}

bool TransferMetaData::isEmpty() const
{
    return mFiles.size() == 0 && mEmptyFolders.size() == 0;
}

unsigned long long TransferMetaData::getAppId() const
{
    return mAppId;
}

void TransferMetaData::setInitialPendingTransfers(int newInitialPendingTransfers)
{
    mInitialPendingTransfers = newInitialPendingTransfers;
}

void TransferMetaData::setNotification(MegaNotification* newNotification)
{
    unlinkNotification();

    newNotification->setData(getAppId());

    mNotification = newNotification;
    mNotificationDestroyedConnection = MegaNotification::connect(newNotification, &MegaNotification::destroyed, [newNotification](){
        auto appDataId(newNotification->getData().toULongLong());
        auto data = TransferMetaDataContainer::getAppData(appDataId);
        if(data && data->isNonExistData())
        {
            //Also remove the source TransferMetaData
            TransferMetaDataContainer::removeAppData(appDataId - 1);
        }
        TransferMetaDataContainer::removeAppData(appDataId);
    });
}

void TransferMetaData::unlinkNotification()
{
    if(mNotification)
    {
        MegaNotification::disconnect(mNotificationDestroyedConnection);
        mNotification->deleteLater();
    }
}

void TransferMetaData::start(mega::MegaTransfer *transfer)
{
    if(transfer)
    {
        if(transfer->isFolderTransfer())
        {
            TransferMetaDataItemId id(transfer->getTag(), transfer->getNodeHandle(), QString::fromUtf8(transfer->getFileName()), QString::fromUtf8(transfer->getPath()));
            auto folderItem = std::make_shared<TransferMetaDataFolderItem>(id);
            mFolders.insert(id,folderItem);
        }
        else
        {
            if(transfer->getFolderTransferTag() > 0)
            {
                addFileFromFolder(transfer->getFolderTransferTag(), transfer->getTag());
            }
            else
            {
                addFile(transfer->getTag());
            }
        }
    }
}

void TransferMetaData::retryFileFromFolderFailingItem(int fileTag, int folderTag, int nodeHandle)
{
    TransferMetaDataItemId fileId(fileTag, nodeHandle);

    auto removed = mFiles.failedTransfers.remove(fileId);
    removed += mFiles.nonExistFailedTransfers.remove(fileId);

    if(removed != 0)
    {
        TransferMetaDataItemId folderId(folderTag, mega::INVALID_HANDLE);
        if(mFolders.contains(folderId))
        {
            mFolders.remove(folderId);
        }

        addInitialPendingTransfer();
    }
}

void TransferMetaData::retryFailingFile(int tag, mega::MegaHandle nodeHandle)
{
    TransferMetaDataItemId fileId(tag, nodeHandle);

    //Don´t use isSingleTransfer as this one takes into account empty folders
    auto isSingle(getTotalFiles() == 1);

    auto removed = mFiles.failedTransfers.remove(fileId);
    removed += mFiles.nonExistFailedTransfers.remove(fileId);

    if(removed != 0)
    {
        if(isSingle)
        {
            if(mNotification)
            {
                unlinkNotification();
            }
        }

        addInitialPendingTransfer();
    }
}

void TransferMetaData::retryAllPressed()
{
    mFiles.failedTransfers.clear();
    mFiles.nonExistFailedTransfers.clear();

    if(mNotification)
    {
        unlinkNotification();
    }
}

DownloadTransferMetaData::DownloadTransferMetaData(unsigned long long appId, const QString &path)
    : TransferMetaData(mega::MegaTransfer::TYPE_DOWNLOAD, appId),
      mLocalTargetPath(QDir::toNativeSeparators(path))
{
}

void DownloadTransferMetaData::start(mega::MegaTransfer *transfer)
{
    TransferMetaData::start(transfer);

    //The download path is the higher path of all the files taken from other session
    if(mCreatedFromOtherSession)
    {
        auto downloadPath = QString::fromUtf8(transfer->getParentPath());
        if(mLocalTargetPath != downloadPath)
        {
            mLocalTargetPath = Utilities::getCommonPath(downloadPath, mLocalTargetPath, false);
        }
    }
}

void DownloadTransferMetaData::updateForeignDir(const mega::MegaHandle&)
{
    mInitialPendingTransfers--;

    // Update pending transfers in metadata, and notify if this was the last.
    checkAndSendNotification();
}

QString DownloadTransferMetaData::getDestinationNodePathByData(const std::shared_ptr<TransferMetaData> &data)
{
    QString path;
    auto downloadData = std::dynamic_pointer_cast<DownloadTransferMetaData>(data);
    if(downloadData && !downloadData->mLocalTargetPath.isEmpty())
    {
        QFileInfo fileInfo(QString(downloadData->mLocalTargetPath + QDir::separator()));
        path = fileInfo.dir().dirName();
    }

    return path;
}

bool DownloadTransferMetaData::finish(mega::MegaTransfer* transfer, mega::MegaError* e)
{
    auto result = TransferMetaData::finish(transfer, e);
    return result;
}

QStringList DownloadTransferMetaData::getLocalPaths() const
{
    QStringList localPaths;

    auto ids = getTransferIdsByState(TransferData::TRANSFER_COMPLETED);
    foreach(auto& id, ids)
    {
        QFileInfo fileInfo(id.path);
        QString localPath(fileInfo.path());

        char *escapedName = MegaSyncApp->getMegaApi()->escapeFsIncompatible(id.name.toStdString().c_str(),
                                                                            localPath.toStdString().c_str());
        QString nodeName = QString::fromUtf8(escapedName);
        delete [] escapedName;

        if(!localPath.endsWith(nodeName))
        {
            if (!localPath.endsWith(QDir::separator()))
            {
                localPath += QDir::separator();
            }
            localPath += nodeName;
        }

        localPaths.append(QDir::toNativeSeparators(localPath));
    }
    return localPaths;
}

const QString &DownloadTransferMetaData::getLocalTargetPath() const
{
    return mLocalTargetPath;
}

bool DownloadTransferMetaData::hasBeenPreviouslyCompleted(mega::MegaTransfer *transfer) const
{
    //If the file has been previously completed, the node is already on the CD.
    //If not, the file should be uploaded again
    TransferMetaDataItemId fileId(-1, transfer->getNodeHandle());
    return mFiles.completedTransfers.contains(fileId);
}

std::shared_ptr<TransferMetaData> DownloadTransferMetaData::createNonExistData()
{
    return TransferMetaDataContainer::createTransferMetaData<DownloadTransferMetaData>(mLocalTargetPath);
}

bool DownloadTransferMetaData::isNonExistTransfer(mega::MegaTransfer *transfer) const
{
    return true;

    //As the SDK is creating (%n) files when the folder download is retried, we cannot offer this possibility.
    //    TransferMetaDataItemId folderId(transfer->getFolderTransferTag(), mega::INVALID_HANDLE);
    //    auto topLevelFolder = mFolders.value(folderId);
    //    if(topLevelFolder)
    //    {
    //        std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(topLevelFolder->id.handle));
    //        return node ? false : true;
    //    }

    //    return true;
}

UploadTransferMetaData::UploadTransferMetaData(unsigned long long appId, const mega::MegaHandle handle)
 : TransferMetaData(mega::MegaTransfer::TYPE_UPLOAD, appId),
   mNodeTargetHandle(handle)
{}

void UploadTransferMetaData::start(mega::MegaTransfer *transfer)
{
    TransferMetaData::start(transfer);

    //The upload path is the higher path of all the files taken from other session
    if(mCreatedFromOtherSession)
    {
        auto parentHandle = transfer->getParentHandle();
        if(parentHandle != mNodeTargetHandle)
        {
            auto currentUploadNode = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(mNodeTargetHandle));
            auto newUploadNode = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(parentHandle));

            if(currentUploadNode && newUploadNode)
            {
                auto currentUploadPath = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(currentUploadNode.get()));
                auto newUploadPath = QString::fromUtf8(MegaSyncApp->getMegaApi()->getNodePath(newUploadNode.get()));

                if(currentUploadPath != newUploadPath)
                {
                    auto commonPath = Utilities::getCommonPath(newUploadPath, currentUploadPath, true);
                    auto newCommonPathNode = std::unique_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByPath(commonPath.toUtf8(), nullptr));
                    if(newCommonPathNode)
                    {
                        mNodeTargetHandle = newCommonPathNode->getHandle();
                    }
                }
            }
        }
    }
}

mega::MegaHandle UploadTransferMetaData::getNodeTarget() const
{
    return mNodeTargetHandle;
}

QList<std::shared_ptr<mega::MegaNode>> UploadTransferMetaData::getNodesByData(const std::shared_ptr<TransferMetaData>& data)
{
    QList<std::shared_ptr<mega::MegaNode>> nodes;
    auto ids = data->getTransferIdsByState(TransferData::TRANSFER_COMPLETED);
    foreach(auto& id, ids)
    {
        if(id.handle != mega::INVALID_HANDLE)
        {
            auto node = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(id.handle));
            if(node)
            {
                nodes.append(node);
            }
        }
    }

    return nodes;
}

bool UploadTransferMetaData::hasBeenPreviouslyCompleted(mega::MegaTransfer *transfer) const
{
    //If the file has been previously completed, the node is already on the CD.
    //If not, the file should be uploaded again
    foreach(auto& file, mFiles.completedTransfersByFolderHandle.values(transfer->getParentHandle()))
    {
        if(file->id.name == QString::fromUtf8(transfer->getFileName()))
        {
            return true;
        }
    }

    return false;
}

std::shared_ptr<TransferMetaData> UploadTransferMetaData::createNonExistData()
{
    return TransferMetaDataContainer::createTransferMetaData<UploadTransferMetaData>(mNodeTargetHandle);
}

bool UploadTransferMetaData::isNonExistTransfer(mega::MegaTransfer *transfer) const
{
    if(transfer->getFolderTransferTag() > 0)
    {
        TransferMetaDataItemId folderId(transfer->getFolderTransferTag(), mega::INVALID_HANDLE);
        auto topLevelFolder = mFolders.value(folderId);
        if(topLevelFolder)
        {
            QFileInfo topLevelFolderPath(topLevelFolder->id.name);
            return !topLevelFolderPath.exists();
        }
    }
    else
    {
        QFileInfo topLevelFilePath(QString::fromUtf8(transfer->getPath()));
        return !topLevelFilePath.exists();
    }

    return true;
}

std::shared_ptr<mega::MegaNode> UploadTransferMetaData::getDestinationNodeByData(const std::shared_ptr<TransferMetaData>& data)
{
    std::shared_ptr<mega::MegaNode> parentNode;
    auto uploadData = std::dynamic_pointer_cast<UploadTransferMetaData>(data);
    if(uploadData)
    {
        auto targetHandle = uploadData->getNodeTarget();
        parentNode = std::shared_ptr<mega::MegaNode>(MegaSyncApp->getMegaApi()->getNodeByHandle(targetHandle));
    }

    return parentNode;
}

QString UploadTransferMetaData::getDestinationNodePathByData(const std::shared_ptr<TransferMetaData> &data)
{
    auto destinationNode = getDestinationNodeByData(data);
    if(destinationNode)
    {
        QString path;

        if(destinationNode->getHandle() == MegaSyncApp->getMegaApi()->getRootNode()->getHandle())
        {
            path = MegaNodeNames::getRootNodeName(destinationNode.get());
        }
        else
        {
            path = QString::fromStdString(MegaSyncApp->getMegaApi()->getNodePath(destinationNode.get()));
        }

        return path;
    }
    return QCoreApplication::translate("MegaError", "Decryption error");
}

bool UploadTransferMetaData::finish(mega::MegaTransfer *transfer, mega::MegaError *e)
{
    return TransferMetaData::finish(transfer, e);
}

//////////CONTAINER AND MANAGER

QHash<unsigned long long, std::shared_ptr<TransferMetaData>> TransferMetaDataContainer::transferAppData = QHash<unsigned long long, std::shared_ptr<TransferMetaData>>();
QMutex TransferMetaDataContainer::mMutex;

bool TransferMetaDataContainer::start(mega::MegaTransfer *transfer)
{
    //Sync transfers are not included in notifications
    if(!transfer->isSyncTransfer())
    {
        auto idResult = appDataToId(transfer->getAppData());
        //TOP LEVEL
        if(idResult.first)
        {
            auto data = getAppData(idResult.second);

            if(!data)
            {
                if(transfer->getType() == mega::MegaTransfer::TYPE_UPLOAD)
                {
                    data = TransferMetaDataContainer::createTransferMetaDataWithappDataId<UploadTransferMetaData>(idResult.second, transfer->getParentHandle());
                }
                else
                {
                    data = TransferMetaDataContainer::createTransferMetaDataWithappDataId<DownloadTransferMetaData>(idResult.second, QString::fromUtf8(transfer->getParentPath()));
                }

                data->setCreatedFromOtherSession();
            }

            data->start(transfer);
        }
        //NESTED FILES IN A FOLDER
        else if(transfer->getFolderTransferTag() > 0)
        {
            auto data = TransferMetaDataContainer::getAppDataByFolderTransferTag(transfer->getFolderTransferTag());
            if(!data)
            {
                if(transfer->getType() == mega::MegaTransfer::TYPE_UPLOAD)
                {
                    data = TransferMetaDataContainer::createTransferMetaData<UploadTransferMetaData>(transfer->getParentHandle());
                }
                else
                {
                    data = TransferMetaDataContainer::createTransferMetaData<DownloadTransferMetaData>(QString::fromUtf8(transfer->getParentPath()));
                }

                data->setCreatedFromOtherSession();
            }

            data->start(transfer);
        }
    }

    return false;
}

void TransferMetaDataContainer::retryTransfer(mega::MegaTransfer *transfer, unsigned long long appDataId)
{
    if(appDataId > 0)
        {
        auto data = TransferMetaDataContainer::getAppData(appDataId);
        if(data)
        {
            if(!transfer->isFolderTransfer())
            {
                QMutexLocker lock(&mMutex);
                data->retryFailingFile(transfer->getTag(), transfer->getNodeHandle());
            }

            if(data->mNonExistsFailAppId != 0)
            {
                auto nonExistData = TransferMetaDataContainer::getAppData(data->mNonExistsFailAppId);
                if(nonExistData)
                {
                    {
                        QMutexLocker lock(&mMutex);
                        nonExistData->retryFailingFile(transfer->getTag(), transfer->getNodeHandle());
                    }

                    if(nonExistData->isEmpty())
                    {
                        removeAppData(data->mNonExistsFailAppId);
                        removeAppData(data->getAppId());
                        data->mNonExistsFailAppId = 0;
                    }
                }
            }
        }
    }
    else if(transfer->getFolderTransferTag() > 0)
    {
        auto data = TransferMetaDataContainer::getAppDataByFolderTransferTag(transfer->getFolderTransferTag());
        if(data)
        {
            {
                QMutexLocker lock(&mMutex);
                data->retryFileFromFolderFailingItem(transfer->getTag(), transfer->getFolderTransferTag(),transfer->getNodeHandle());
            }

            if(data->mNonExistsFailAppId != 0)
            {
                auto nonExistData = TransferMetaDataContainer::getAppData(data->mNonExistsFailAppId);
                if(nonExistData)
                {
                    {
                        QMutexLocker lock(&mMutex);
                        nonExistData->retryFileFromFolderFailingItem(transfer->getTag(), transfer->getFolderTransferTag(), transfer->getNodeHandle());
                    }

                    if(nonExistData->isEmpty())
                    {
                        removeAppData(data->mNonExistsFailAppId);
                        data->mNonExistsFailAppId = 0;
                    }
                }
            }
        }
    }
}

bool TransferMetaDataContainer::addAppData(unsigned long long appId, std::shared_ptr<TransferMetaData> data)
{
    QMutexLocker lock(&mMutex);
    return transferAppData.insert(appId, data) != transferAppData.end();
}

void TransferMetaDataContainer::removeAppData(unsigned long long appId)
{
    auto data = TransferMetaDataContainer::getAppData(appId);
    if(data)
    {
        QMutexLocker lock(&mMutex);
        transferAppData.remove(appId);
    }
}

bool TransferMetaDataContainer::finishFromFolderTransfer(mega::MegaTransfer *transfer, mega::MegaError *e)
{
    auto folderTag = transfer->getFolderTransferTag();
    if(folderTag > 0)
    {
        auto data = getAppDataByFolderTransferTag(folderTag);
        if(data)
        {
            return data->finish(transfer, e);
        }
    }

    return false;
}

void TransferMetaDataContainer::finish(unsigned long long appId, mega::MegaTransfer *transfer, mega::MegaError *e)
{
    auto data = getAppData(appId);

    if(data)
    {
        data->finish(transfer, e);
    }
}
