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

    if(data->isSingleTransfer())
    {
        if (data->isUpload())
        {
            info.title = buildSingleUploadTitle();
            info.message = buildSingleUploadMessage();
            info.actions = buildSingleUploadActions();
        }
        else if (data->isDownload())
        {
            auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(data);
            info.title = buildSingleDownloadTitle(destinationPath);
            info.message = buildSingleDownloadMessage(destinationPath);
            info.actions = buildSingleDownloadActions(destinationPath);
        }
    }
    else
    {
        if (data->isUpload())
        {
            info.title = buildMultipleUploadTitle();
            info.message = buildMultipleUploadMessage();
            info.actions = buildMultipleUploadActions();
        }
        else if (data->isDownload())
        {
            auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(data);
            info.title = buildMultipleDownloadTitle(destinationPath);
            info.message = buildMultipleDownloadMessage(destinationPath);
            info.actions = buildMultipleDownloadActions(destinationPath);
        }
    }

    info.imagePath = getImagePath();

    return info;
}

///////////////////////////////////////////////////////////////////////////////////////
QString TransferNotificationBuilder::buildSingleUploadTitle()
{
    if(data->allHaveFailed())
    {
        return tr("Upload failed");
    }
    else if(data->getEmptyFolderTransfersOK())
    {
        return tr("Folder uploaded");
    }
    else
    {
        return tr("File uploaded");
    }
}

QString TransferNotificationBuilder::buildSingleUploadMessage()
{
    QString path = UploadTransferMetaData::getDestinationNodePathByData(data);

    if(data->allHaveFailed())
    {
        auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if(data->isNonExistData())
        {
            return tr("%1 no longer exists or was renamed.").arg(failedId.name);
        }
        else
        {
            return QString(tr("Couldn't upload %1 to %2.").arg(failedId.name,path));
        }
    }
    else
    {
        auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
        return tr("Uploaded %1 to %2.").arg(completedId.name,path);
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
QString TransferNotificationBuilder::buildSingleDownloadTitle(const QString&)
{
    if(data->allHaveFailed())
    {
        return tr("Download failed");
    }
    else if(data->getEmptyFolderTransfersOK())
    {
        return tr("Folder downloaded");
    }
    else
    {
        return tr("File downloaded");
    }
}

QString TransferNotificationBuilder::buildSingleDownloadMessage(const QString &destinationPath)
{
    if(data->allHaveFailed())
    {
        auto id = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
        if(data->isNonExistData())
        {
            return tr("%1 no longer exists.").arg(id.name);
        }
        else
        {
            return tr("Couldn't download %1 to %2").arg(id.name, destinationPath);
        }
    }
    else
    {
        auto id = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
        return tr("Downloaded %1 to %2.").arg(id.name, destinationPath);
    }
}

QStringList TransferNotificationBuilder::buildSingleDownloadActions(const QString&)
{
    QStringList actions;

    if(data->allHaveFailed())
    {
        if(!data->isNonExistData())
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
QString TransferNotificationBuilder::buildMultipleUploadTitle()
{
    if(data->allHaveFailed())
    {
        return tr("Upload failed");
    }
    else if(data->someHaveFailed())
    {
        return tr("Not all items were uploaded");
    }
    else
    {
        return tr("Items uploaded");
    }
}

QString TransferNotificationBuilder::buildMultipleUploadMessage()
{
    QString message;

    if(data->allHaveFailed())
    {
        auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(data);
        if(data->isNonExistData())
        {
            message = tr("Items no longer exist or were renamed");
        }
        else
        {
            message = tr("Couldn't upload %1 items to %2.").arg(QString::number(data->getTotalFiles() + data->getTotalEmptyFolders()), nodePath);
        }
    }
    else
    {
        if(data->someHaveFailed())
        {
            auto completedItems = data->getFileTransfersOK() + data->getEmptyFolderTransfersOK();
            auto failedItems = data->getEmptyFolderTransfersFailed() + data->getFileTransfersFailed();

            if(failedItems == 1 && completedItems == 1)
            {
                auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
                auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);

                if(completedId.isValid() && failedId.isValid())
                {
                    message = tr("Uploaded %1,  couldn't upload %2.").arg(completedId.name, failedId.name);
                }
            }
            else if(failedItems > 1 && completedItems == 1)
            {
                auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
                if(completedId.isValid())
                {
                    message = tr("Uploaded %1, couldn't upload %2 items.").arg(completedId.name, QString::number(failedItems));
                }
            }
            else if(failedItems == 1 && completedItems > 1)
            {
                auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);

                if(failedId.isValid())
                {
                    message = tr("Uploaded %1 items, couldn't upload %2.").arg(QString::number(completedItems), failedId.name);
                }
            }
            else if(failedItems > 1 && completedItems > 1)
            {
                message = tr("Uploaded %1 items, couldn't upload %2 items.").arg(QString::number(completedItems), QString::number(failedItems));
            }
        }
        else
        {
            auto nodePath = UploadTransferMetaData::getDestinationNodePathByData(data);
            message = tr("Uploaded %1 items to %2.").arg(QString::number(data->getFileTransfersOK() + data->getEmptyFolderTransfersOK()), nodePath);
        }
    }

    return message;
}

QStringList TransferNotificationBuilder::buildMultipleUploadActions()
{
    QStringList actions;

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
QString TransferNotificationBuilder::buildMultipleDownloadTitle(const QString &)
{
    if(data->allHaveFailed())
    {
        return tr("Download failed");
    }
    else if(data->someHaveFailed())
    {
        return tr("Not all items were downloaded");
    }
    else
    {
        return tr("Items downloaded");
    }
}

QString TransferNotificationBuilder::buildMultipleDownloadMessage(const QString &destinationPath)
{
    QString message;

    if(data->allHaveFailed())
    {
        if(data->isNonExistData())
        {
            message = tr("Items no longer exist");
        }
        else
        {
            message = tr("Couldn't download %1 items to %2.").arg(QString::number(data->getTotalFiles() + data->getTotalEmptyFolders()), destinationPath);
        }
    }
    else
    {
        if(data->someHaveFailed())
        {
            auto completedItems = data->getFileTransfersOK() + data->getEmptyFolderTransfersOK();
            auto failedItems = data->getEmptyFolderTransfersFailed() + data->getFileTransfersFailed();

            if(failedItems == 1 && completedItems == 1)
            {
                auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
                auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);

                if(completedId.isValid() && failedId.isValid())
                {
                    message = tr("Downloaded %1,  couldn't download %2.").arg(completedId.name, failedId.name);
                }
            }
            else if(failedItems > 1 && completedItems == 1)
            {
                auto completedId = data->getFirstTransferIdByState(TransferData::TRANSFER_COMPLETED);
                if(completedId.isValid())
                {
                    message = tr("Downloaded %1, couldn't download %2 items.").arg(completedId.name, QString::number(failedItems));
                }
            }
            else if(failedItems == 1 && completedItems > 1)
            {
               auto failedId = data->getFirstTransferIdByState(TransferData::TRANSFER_FAILED);
               if(failedId.isValid())
               {
                   message = tr("Downloaded %1 items, couldn't download %2.").arg(QString::number(completedItems), failedId.name);
               }
            }
            else if(failedItems > 1 && completedItems > 1)
            {
                message = tr("Downloaded %1 items, couldn't download %2 items.").arg(QString::number(completedItems), QString::number(failedItems));
            }
        }
        else
        {
            message = tr("Downloaded %1 items to %2.").arg(QString::number(data->getFileTransfersOK() + data->getEmptyFolderTransfersOK()), destinationPath);
        }
    }

    return message;
}

QStringList TransferNotificationBuilder::buildMultipleDownloadActions(const QString &)
{
    QStringList actions;

    if(data->allHaveFailed())
    {
        if(!data->isNonExistData())
        {
            actions << tr("Retry");
        }
    }
    else if(data->someHaveFailed())
    {
        actions << tr("Show in folder");

        if(!data->isNonExistData())
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
///////////////////////////////////////////////////////////////////////////////////////

QString TransferNotificationBuilder::getImagePath()
{
    return MegaNotification::defaultImage;
}
