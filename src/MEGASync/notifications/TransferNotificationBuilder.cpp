#include "TransferNotificationBuilder.h"

#include "megaapi.h"
#include "MegaApplication.h"

#include <QString>

TransferNotificationBuilder::TransferNotificationBuilder(
    const std::shared_ptr<TransferMetaData>& data):
    mData(data)
{
}

DesktopNotifications::NotificationInfo TransferNotificationBuilder::buildNotification(const bool isLogged)
{
    DesktopNotifications::NotificationInfo info;

    if (mData->isUpload())
    {
        if (isLogged)
        {
            info.title = buildUploadTitle();
            if (mData->isSingleTransfer())
            {
                info.message = buildSingleUploadMessage();
                info.actions = buildSingleUploadActions();
            }
            else
            {
                info.message = buildMultipleUploadMessage();
                info.actions = buildMultipleUploadActions();
            }
        }
        else
        {
            info.title = tr("Upload stopped", "", mData->getTransfersCount());
            info.message = tr("You logged out of MEGA so your upload has stopped. You can resume "
                              "the upload after logging back in.",
                              "",
                              mData->getTransfersCount());
        }
    }
    else if (mData->isDownload())
    {
        if (auto downloadData = std::dynamic_pointer_cast<DownloadTransferMetaData>(mData))
        {
            auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(mData);

            if (downloadData->isImportedLink())
            {
                info.title = buildImportedLinkErrorTitle();
                info.message = buildImportedLinkError();
            }

            // No imported link failed, so it is a normal download
            if (info.message.isEmpty())
            {
                info.title = buildDownloadTitle();

                if (mData->isSingleTransfer())
                {
                    info.message = buildSingleDownloadMessage(destinationPath);
                    info.actions = buildSingleDownloadActions(destinationPath);
                }
                else
                {
                    info.message = buildMultipleDownloadMessage(destinationPath);
                    info.actions = buildMultipleDownloadActions(destinationPath);
                }
            }
        }
    }

    info.imagePath = getImagePath();

    return info;
}

QString TransferNotificationBuilder::getDownloadFailedTitle()
{
    return tr("Could not download");
}

QString TransferNotificationBuilder::getDownloadSomeFailedTitle()
{
    return tr("Download incomplete");
}

QString TransferNotificationBuilder::getDownloadSuccessTitle()
{
    return tr("Download complete");
}

QString TransferNotificationBuilder::getDownloadFailedText(int num, const QString& destPath)
{
    return tr("%n item couldn’t be downloaded to %1.", "", num).arg(destPath);
}

QString TransferNotificationBuilder::getSomeDownloadFailedText(int completed, int failed)
{
    QString successItems = tr("%n item downloaded", "", completed);
    return tr("%1, but %n item couldn’t be downloaded.", "", failed).arg(successItems);
}

QString TransferNotificationBuilder::getDownloadSuccessText(int num, const QString& destPath)
{
    return tr("%n item downloaded to %1.", "", num).arg(destPath);
}

QString TransferNotificationBuilder::getSingleDownloadFailed(const QString& fileName, const QString& destPath)
{
    return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 couldn’t be downloaded to %2.").arg(fileName, destPath);
}

QString TransferNotificationBuilder::getShowInFolderText()
{
    return tr("Show in folder");
}

QString TransferNotificationBuilder::buildUploadTitle()
{
    if (mData->allHaveFailed())
    {
        return tr("Could not upload");
    }
    else if (mData->someHaveFailed())
    {
        return tr("Upload incomplete");
    }
    else
    {
        return tr("Upload complete");
    }
}

QString TransferNotificationBuilder::buildDownloadTitle()
{
    if (mData->allHaveFailed())
    {
        return getDownloadFailedTitle();
    }
    else if (mData->someHaveFailed())
    {
        return getDownloadSomeFailedTitle();
    }
    else
    {
        return getDownloadSuccessTitle();
    }
}

QString TransferNotificationBuilder::buildImportedLinkErrorTitle()
{
    return MegaApplication::tr("Folder download error");
}

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildSingleUploadMessage()
{
    QString path = UploadTransferMetaData::getDestinationNodePathByData(mData);

    if (mData->allHaveFailed())
    {
        auto failedId = mData->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if (mData->isNonExistData())
        {
            return buildSingleNonExistentDataMessageUpload(failedId.name);
        }
        else
        {
            if (isFolder())
            {
                return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 couldn’t be uploaded to %2.").arg(failedId.name,path);
            }
            else
            {
                return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 couldn’t be uploaded to %2.").arg(failedId.name,path);
            }
        }
    }
    else
    {
        auto completedId = mData->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
        if (isFolder())
        {
            return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 uploaded to %2.").arg(completedId.name,path);
        }
        else
        {
            return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 uploaded to %2.").arg(completedId.name,path);
        }
    }
}

QStringList TransferNotificationBuilder::buildSingleUploadActions()
{
    QStringList actions;

    if(!MegaSyncApp->getMegaApi()->isLoggedIn())
    {
        return actions;
    }

    if (mData->allHaveFailed())
    {
        actions << tr("Retry");
    }
    else
    {
        actions  << tr("Show in MEGA");

        auto completedId = mData->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
        if(completedId.handle != mega::INVALID_HANDLE)
        {
            std::unique_ptr<mega::MegaNode> node(MegaSyncApp->getMegaApi()->getNodeByHandle(completedId.handle));

            if(!Utilities::isIncommingShare(node.get()))
            {
                actions << tr("Get link");
            }
        }
    }

    return actions;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////

QString TransferNotificationBuilder::buildSingleDownloadMessage(const QString &destinationPath)
{
    if (mData->allHaveFailed())
    {
        auto id = mData->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if (mData->isNonExistData())
        {
            return buildSingleNonExistentDataMessageDownload(id.name);
        }
        else
        {
            if (isFolder())
            {
                return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 couldn’t be downloaded to %2.").arg(id.name, destinationPath);
            }
            else
            {
                return getSingleDownloadFailed(id.name, destinationPath);
            }
        }
    }
    else
    {
        auto id = mData->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
        if (isFolder())
        {
            return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 downloaded to %2.").arg(id.name, destinationPath);
        }
        else
        {
            return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 downloaded to %2.").arg(id.name, destinationPath);
        }
    }
}

QStringList TransferNotificationBuilder::buildSingleDownloadActions(const QString&)
{
    QStringList actions;

    if (mData->allHaveFailed())
    {
        if (!mData->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry");
        }
    }
    else
    {
        actions << getShowInFolderText() << tr("Open");
    }

    return actions;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildMultipleUploadMessage()
{
    QString message;

    if (mData->allHaveFailed())
    {
        auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(mData);
        if (mData->isNonExistData())
        {
            message = buildNonExistentItemsMessageUploads();
        }
        else
        {
            message = tr("%n item couldn’t be uploaded to %1.",
                         "",
                         mData->getTotalFiles() + mData->getTotalEmptyFolders())
                          .arg(nodePath);
        }
    }
    else
    {
        if (mData->someHaveFailed())
        {
            auto completedItems = mData->getFileTransfersOK() + mData->getEmptyFolderTransfersOK();
            auto failedItems =
                mData->getEmptyFolderTransfersFailed() + mData->getFileTransfersFailed();

            QString successItems = tr("%n item uploaded", "", completedItems);
            message = tr("%1, but %n item couldn’t be uploaded.", "", failedItems).arg(successItems);
        }
        else
        {
            auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(mData);
            message = tr("%n item uploaded to %1.",
                         "",
                         mData->getFileTransfersOK() + mData->getEmptyFolderTransfersOK())
                          .arg(nodePath);
        }
    }

    return message;
}

QStringList TransferNotificationBuilder::buildMultipleUploadActions()
{
    QStringList actions;

    if(!MegaSyncApp->getMegaApi()->isLoggedIn())
    {
        return actions;
    }

    if (mData->allHaveFailed())
    {
        actions << tr("Retry");
    }
    else if (mData->someHaveFailed())
    {
        actions << tr("Show in MEGA");
        actions << tr("Retry failed items",
                      "",
                      mData->getFileTransfersFailed() + mData->getEmptyFolderTransfersFailed());
    }
    else
    {
        actions << tr("Show in MEGA");
    }

    return actions;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildMultipleDownloadMessage(const QString &destinationPath)
{
    QString message;

    if (mData->allHaveFailed())
    {
        if (mData->isNonExistData())
        {
            message = buildNonExistentItemsMessageDownloads();
        }
        else
        {
            message = getDownloadFailedText(mData->getTotalFiles() + mData->getTotalEmptyFolders(),
                                            destinationPath);
        }
    }
    else
    {
        if (mData->someHaveFailed())
        {
            auto completedItems = mData->getFileTransfersOK() + mData->getEmptyFolderTransfersOK();
            auto failedItems =
                mData->getEmptyFolderTransfersFailed() + mData->getFileTransfersFailed();
            message = getSomeDownloadFailedText(completedItems, failedItems);
        }
        else
        {
            message = getDownloadSuccessText(mData->getFileTransfersOK() +
                                                 mData->getEmptyFolderTransfersOK(),
                                             destinationPath);
        }
    }

    return message;
}

QStringList TransferNotificationBuilder::buildMultipleDownloadActions(const QString &)
{
    QStringList actions;

    if(!MegaSyncApp->getMegaApi()->isLoggedIn())
    {
        return actions;
    }

    if (mData->allHaveFailed())
    {
        if (!mData->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry");
        }
    }
    else if (mData->someHaveFailed())
    {
        actions << getShowInFolderText();

        if (!mData->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry failed items",
                          "",
                          mData->getFileTransfersFailed() + mData->getEmptyFolderTransfersFailed());
        }

    }
    else
    {
        actions << getShowInFolderText();
    }

    return actions;
}

QString TransferNotificationBuilder::buildSingleNonExistentDataMessageUpload(const QString &itemName)
{
    if (isFolder())
    {
        return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 no longer exists or was renamed.").arg(itemName);
    }
    return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 no longer exists or was renamed.").arg(itemName);
}

QString TransferNotificationBuilder::buildSingleNonExistentDataMessageDownload(const QString &itemName)
{
    if (isFolder())
    {
        return QCoreApplication::translate("TransferNotificationBuilder_Folder", "%1 no longer exists.").arg(itemName);
    }
    return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 no longer exists.").arg(itemName);
}

QString TransferNotificationBuilder::buildNonExistentItemsMessageUploads()
{
    return tr("%n item no longer exist or was renamed.", "", mData->getNonExistentCount());
}

QString TransferNotificationBuilder::buildNonExistentItemsMessageDownloads()
{
    return tr("%n item no longer exist.", "", mData->getNonExistentCount());
}

QString TransferNotificationBuilder::buildImportedLinkError()
{
    QString message;

    auto item = mData->getFirstTransferByState(TransferData::TRANSFER_FAILED);

    if (item)
    {
        auto errorCode(item->failedTransfer->getLastError().getErrorCode());
        auto path(item->id.path);

        if (errorCode == mega::MegaError::API_EWRITE)
        {
            QFileInfo info(path);
            if (info.exists())
            {
                message = MegaApplication::tr(
                              "The folder %1 can't be downloaded. The download may have failed due "
                              "to a "
                              "casing mismatch. Ensure the folders match exactly and try again.")
                              .arg(path);
            }
            else
            {
                message =
                    MegaApplication::tr(
                        "The folder %1 can't be downloaded. Check the download destination folder.")
                        .arg(path);
            }
        }
        else
        {
            const QString errorString =
                QString::fromUtf8(mega::MegaError::getErrorString(errorCode));
            message = MegaApplication::tr("The folder %1 can't be downloaded. Error received : %2.")
                          .arg(path, errorString);
        }
    }

    return message;
}
///////////////////////////////////////////////////////////////////////////////////////

QString TransferNotificationBuilder::getImagePath()
{
    return DesktopAppNotification::defaultImage;
}

bool TransferNotificationBuilder::isFolder() const
{
    return mData->getEmptyFolderTransfersOK();
}
