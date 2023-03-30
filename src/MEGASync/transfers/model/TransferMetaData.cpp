#include <TransferMetaData.h>

#include <QDir>

#include <MegaApplication.h>
#include <Preferences.h>
#include <Notificator.h>

#include <megaapi.h>
#include <mega/types.h>

TransferMetaData::TransferMetaData(int8_t direction, unsigned long long id)
    : mPendingTransfers(0),
      mTotalFiles(0), mTotalFolders(0),
      mTransfersFileOK(0), mTransfersFolderOK(0),
      mTransfersCancelled(0),
      mTransferDirection(direction), mCreateRootFolder(false),
      mNotificationBlocked(false), mForSpareTransfers(false),
      mAppId(id), mNotification(nullptr)
{
}

TransferMetaData::TransferMetaData()
    : mPendingTransfers(0),
      mTotalFiles(0), mTotalFolders(0),
      mTransfersFileOK(0), mTransfersFolderOK(0),
      mTransfersCancelled(0),
      mTransferDirection(-1), mCreateRootFolder(false),
      mNotificationBlocked(false), mForSpareTransfers(false),
      mAppId(0), mNotification(nullptr)
{
}

bool TransferMetaData::remove()
{
    return TransferMetaDataContainer::removeAppData(mAppId);
}

void TransferMetaData::updateOnTransferFinish(mega::MegaTransfer *transfer, mega::MegaError* e)
{
    // check if it's a top level transfer
    if (transfer->getFolderTransferTag() <= 0)
    {
        auto idResult= TransferMetaDataContainer::appDataToId(transfer->getAppData());

        if (idResult.first)
        {
            //API_EINCOMPLETE for folders, cancelled for files
            if (e->getErrorCode() == mega::MegaError::API_EINCOMPLETE
                    || transfer->getState() == mega::MegaTransfer::STATE_CANCELLED)
            {
                mTransfersCancelled++;
            }
            else if (e->getErrorCode() != mega::MegaError::API_OK)
            {
                mTransfersFailed.append(transfer->getNodeHandle());
            }
            else
            {
                transfer->isFolderTransfer() ? mTransfersFolderOK++ : mTransfersFileOK++;
            }

            removePendingFile();
        }
    }
}

bool TransferMetaData::isSingleTransfer() const
{
    return getTotalTransfers() == 1;
}

int TransferMetaData::getTotalTransfers() const
{
    return mTotalFiles + mTotalFolders;
}

bool TransferMetaData::readyForNotification() const
{
    return mPendingTransfers == 0 && getTransfersOK() > 0 && !mNotificationBlocked;
}

int TransferMetaData::getPendingTransfers() const
{
    return mPendingTransfers;
}

int TransferMetaData::getTotalFiles() const
{
    return mTotalFiles;
}

int TransferMetaData::getTotalFolders() const
{
    return mTotalFolders;
}

int TransferMetaData::getTransfersFileOK() const
{
    return mTransfersFileOK;
}

int TransferMetaData::getTransfersFolderOK() const
{
    return mTransfersFolderOK;
}

int TransferMetaData::getTransfersOK() const
{
    return (getTransfersFileOK() + getTransfersFolderOK());
}

int TransferMetaData::getTransfersFailed() const
{
    return mTransfersFailed.size();
}

int TransferMetaData::getTransfersCancelled() const
{
    return mTransfersCancelled;
}

void TransferMetaData::addFile()
{
    mTotalFiles++;
    if(mForSpareTransfers)
    {
        mPendingTransfers++;
    }
}

void TransferMetaData::addFolder()
{
    mTotalFolders++;
    if(mForSpareTransfers)
    {
        mPendingTransfers++;
    }
}

void TransferMetaData::removePendingFile()
{
    if(mPendingTransfers > 0)
    {
        mPendingTransfers--;
    }

    checkAndSendNotification();
}

void TransferMetaData::removeFailingItem(const mega::MegaHandle &handle)
{
    mTransfersFailed.removeOne(handle);
}

void TransferMetaData::checkAndSendNotification()
{
    if (readyForNotification())
    {
        if (Preferences::instance()->isNotificationEnabled(Preferences::NotificationsTypes::COMPLETED_UPLOADS_DOWNLOADS))
        {
            //Transfers finished and ready, show notification
            MegaSyncApp->showNotificationFinishedTransfers(getAppId());
        }
        else
        {
            TransferMetaDataContainer::removeAppData(getAppId());
        }
    }
}

bool TransferMetaData::isDownload() const
{
    return mTransferDirection == mega::MegaTransfer::TYPE_DOWNLOAD;
}

bool TransferMetaData::isUpload() const
{
    return mTransferDirection == mega::MegaTransfer::TYPE_UPLOAD;
}

bool TransferMetaData::isFolderTransfer() const
{
    return getTotalFolders() == getTotalTransfers();
}

bool TransferMetaData::isFileTransfer() const
{
    return getTotalFiles() == getTotalTransfers();
}

unsigned long long TransferMetaData::getAppId() const
{
    return mAppId;
}

void TransferMetaData::setPendingTransfers(int newPendingTransfers)
{
    mPendingTransfers = newPendingTransfers;
}

void TransferMetaData::setForSpareTransfers(bool newForSpareTransfers)
{
    mForSpareTransfers = newForSpareTransfers;
}

void TransferMetaData::setNotification(MegaNotification* newNotification)
{
    if(mNotification)
    {
        MegaNotification::disconnect(mNotificationDestroyed);
        mNotification->deleteLater();
    }

    mNotification = newNotification;

    newNotification->setData(getAppId());
    mNotificationDestroyed = MegaNotification::connect(newNotification, &MegaNotification::destroyed, [newNotification](){
        TransferMetaDataContainer::removeAppData(newNotification->getData().toULongLong());
    });
}

bool TransferMetaData::getCreateRootFolder() const
{
    return mCreateRootFolder;
}

void TransferMetaData::setCreateRootFolder(bool newCreateRootFolder)
{
    mCreateRootFolder = newCreateRootFolder;
}

void TransferMetaData::blockNotification()
{
    mNotificationBlocked = true;
}

DownloadTransferMetaData::DownloadTransferMetaData(unsigned long long appId, const QString &path)
    : TransferMetaData(mega::MegaTransfer::TYPE_DOWNLOAD, appId),
      mLocalTargetPath(QDir::toNativeSeparators(path))
{
}

void DownloadTransferMetaData::update(mega::MegaNode *node)
{
    // Update transfer metadata according to node type.
    if(node)
    {
        node->isFolder() ? addFolder() : addFile();
    }
}

void DownloadTransferMetaData::updateForeignDir()
{
    // Thus, this was a "root folder", and we successfully transfered it.
    if (getCreateRootFolder())
    {
        mTransfersFolderOK++;
    }

    // Update pending transfers in metadata, and notify if this was the last.
    removePendingFile();
}

QVariant DownloadTransferMetaData::getData() const
{
    return isSingleTransfer() ? mLocalPaths.first() : mLocalTargetPath;
}

QString DownloadTransferMetaData::getDestinationNodePathByData(const std::shared_ptr<TransferMetaData> &data)
{
    QString path;
    auto downloadData = std::dynamic_pointer_cast<DownloadTransferMetaData>(data);
    if(downloadData)
    {
        path = downloadData->mLocalTargetPath;
    }

    return path;
}

void DownloadTransferMetaData::updateOnTransferFinish(mega::MegaTransfer* transfer, mega::MegaError* e)
{
    // If there is only 1 transfer, set localPath to full path
    std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(transfer->getNodeHandle()));
    if(node && node->isNodeKeyDecrypted())
    {
        QString localPath(mLocalTargetPath);
        char *escapedName = MegaSyncApp->getMegaApi()->escapeFsIncompatible(node->getName(),
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

        mLocalPaths.append(localPath);
    }

    TransferMetaData::updateOnTransferFinish(transfer, e);
}

const QStringList &DownloadTransferMetaData::getLocalPaths() const
{
    return mLocalPaths;
}

const QString &DownloadTransferMetaData::getLocalTargetPath() const
{
    return mLocalTargetPath;
}

UploadTransferMetaData::UploadTransferMetaData(unsigned long long appId, const mega::MegaHandle handle)
 : TransferMetaData(mega::MegaTransfer::TYPE_UPLOAD, appId),
   mNodeTargetHandle(handle)
{}

void UploadTransferMetaData::update(const QString& nodePath)
{
    QFileInfo filePathInfo(nodePath);
    update(filePathInfo.isDir());
}

void UploadTransferMetaData::update(bool isDir)
{
    isDir ? addFolder() : addFile();
}

QVariant UploadTransferMetaData::getData() const
{
    return QVariant::fromValue(mNodeHandles);
}

mega::MegaHandle UploadTransferMetaData::getNodeTarget() const
{
    return mNodeTargetHandle;
}

QList<std::shared_ptr<mega::MegaNode>> UploadTransferMetaData::getNodesByData(const std::shared_ptr<TransferMetaData>& data)
{
    QList<std::shared_ptr<mega::MegaNode>> nodes;
    const auto nodeHandles = data->getData().value<QList<mega::MegaHandle>>();
    foreach(const auto& nodeHandle, nodeHandles)
    {
        std::shared_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(nodeHandle));
        if(node)
        {
            nodes.append(node);
        }
    }

    return nodes;
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

void UploadTransferMetaData::updateOnTransferFinish(mega::MegaTransfer *transfer, mega::MegaError *e)
{
    mNodeHandles.append(transfer->getNodeHandle());

    TransferMetaData::updateOnTransferFinish(transfer, e);
}

//////////CONTAINER AND MANAGER

QHash<unsigned long long, std::shared_ptr<TransferMetaData>> TransferMetaDataContainer::transferAppData = QHash<unsigned long long, std::shared_ptr<TransferMetaData>>();

bool TransferMetaDataContainer::addAppData(unsigned long long appId, std::shared_ptr<TransferMetaData> data)
{
    return transferAppData.insert(appId, data) != transferAppData.end();
}

bool TransferMetaDataContainer::removeAppData(unsigned long long appId)
{
    return transferAppData.remove(appId) > 0;
}

void TransferMetaDataContainer::updateOnTransferFinish(unsigned long long appId, mega::MegaTransfer *transfer, mega::MegaError *e)
{
    auto data = getAppData(appId);

    if (transfer->isFolderTransfer())
    {
        if (e->getErrorCode() != mega::MegaError::API_OK)
        {
            //If we show this error notification, we will not show the normal transfer notification
            if(data)
            {
                data->blockNotification();
            }

            MegaSyncApp->showErrorMessage(MegaApplication::tr("Error transferring folder: %1").arg(QCoreApplication::translate("MegaError", mega::MegaError::getErrorString(e->getErrorCode(), mega::MegaError::API_EC_UPLOAD))));
        }
    }

    if(data)
    {
        data->updateOnTransferFinish(transfer, e);
    }
}
