#include "TransferNotificationBuilder.h"

#include <megaapi.h>
#include "MegaApplication.h"
#include <MegaNodeNames.h>

#include <QString>

TransferNotificationBuilder::TransferNotificationBuilder(const std::shared_ptr<TransferMetaData> &data)
    : data(data)
{
}

FinishedTransferNotificationInfo TransferNotificationBuilder::buildNotification()
{
    FinishedTransferNotificationInfo info;

    if (data->isUpload())
    {
        info.title = buildUploadTitle();
        if(data->isSingleTransfer())
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
    else if (data->isDownload())
    {
        auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(data);
        info.title = buildDownloadTitle();
        if(data->isSingleTransfer())
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

    info.imagePath = getImagePath();

    return info;
}

QString TransferNotificationBuilder::buildUploadTitle()
{
    if (data->allHaveFailed())
    {
        return tr("Could not upload");
    }
    else if (data->someHaveFailed())
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
    if (data->allHaveFailed())
    {
        return tr("Could not download");
    }
    else if (data->someHaveFailed())
    {
        return tr("Download incomplete");
    }
    else
    {
        return tr("Download complete");
    }
}

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildSingleUploadMessage()
{
    QString path = UploadTransferMetaData::getDestinationNodePathByData(data);

    if(data->allHaveFailed())
    {
        auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if(data->isNonExistData())
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
        auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
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

    if(data->allHaveFailed())
    {
        actions << tr("Retry");
    }
    else
    {
        actions  << tr("Show in MEGA");

        auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
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
    if(data->allHaveFailed())
    {
        auto id = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if(data->isNonExistData())
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
                return QCoreApplication::translate("TransferNotificationBuilder_File", "%1 couldn’t be downloaded to %2.").arg(id.name, destinationPath);
            }
        }
    }
    else
    {
        auto id = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
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

    if(data->allHaveFailed())
    {
        if(!data->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry");
        }
    }
    else
    {
        actions << tr("Show in folder") << tr("Open");
    }

    return actions;
}
///////////////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildMultipleUploadMessage()
{
    QString message;

    if(data->allHaveFailed())
    {
        auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(data);
        if(data->isNonExistData())
        {
            message = buildNonExistentItemsMessageUploads();
        }
        else
        {
            message = tr("%n item couldn’t be uploaded to %1.", "", data->getTotalFiles() + data->getTotalEmptyFolders()).arg(nodePath);
        }
    }
    else
    {
        if(data->someHaveFailed())
        {
            auto completedItems = data->getFileTransfersOK() + data->getEmptyFolderTransfersOK();
            auto failedItems = data->getEmptyFolderTransfersFailed() + data->getFileTransfersFailed();

            QString successItems = tr("%n item uploaded", "", completedItems);
            message = tr("%1, but %n item couldn’t be uploaded.", "", failedItems).arg(successItems);
        }
        else
        {
            auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(data);
            message = tr("%n item uploaded to %1.", "", data->getFileTransfersOK() + data->getEmptyFolderTransfersOK()).arg(nodePath);
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

    if(data->allHaveFailed())
    {
        actions << tr("Retry");
    }
    else if(data->someHaveFailed())
    {
        actions << tr("Show in MEGA");
        actions << tr("Retry failed items", "", data->getFileTransfersFailed() + data->getEmptyFolderTransfersFailed());
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

    if(data->allHaveFailed())
    {
        if(data->isNonExistData())
        {
            message = buildNonExistentItemsMessageDownloads();
        }
        else
        {
            message = tr("%n item couldn’t be downloaded to %1.", "", data->getTotalFiles() + data->getTotalEmptyFolders()).arg(destinationPath);
        }
    }
    else
    {
        if(data->someHaveFailed())
        {
            auto completedItems = data->getFileTransfersOK() + data->getEmptyFolderTransfersOK();
            auto failedItems = data->getEmptyFolderTransfersFailed() + data->getFileTransfersFailed();

            QString successItems = tr("%n item downloaded", "", completedItems);
            message = tr("%1, but %n item couldn’t be downloaded.", "", failedItems).arg(successItems);
        }
        else
        {
            message = tr("%n item downloaded to %1.", "", data->getFileTransfersOK() + data->getEmptyFolderTransfersOK()).arg(destinationPath);
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

    if(data->allHaveFailed())
    {
        if(!data->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry");
        }
    }
    else if(data->someHaveFailed())
    {
        actions << tr("Show in folder");

        if(!data->isNonExistData() && MegaSyncApp->getMegaApi()->isLoggedIn())
        {
            actions << tr("Retry failed items", "", data->getFileTransfersFailed() + data->getEmptyFolderTransfersFailed());
        }

    }
    else
    {
        actions << tr("Show in folder");
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
    return tr("%n item no longer exist or was renamed.", "", data->getNonExistentCount());
}

QString TransferNotificationBuilder::buildNonExistentItemsMessageDownloads()
{
    return tr("%n item no longer exist.", "", data->getNonExistentCount());
}
///////////////////////////////////////////////////////////////////////////////////////

QString TransferNotificationBuilder::getImagePath()
{
    return MegaNotification::defaultImage;
}

bool TransferNotificationBuilder::isFolder() const
{
    return data->getEmptyFolderTransfersOK();
}
