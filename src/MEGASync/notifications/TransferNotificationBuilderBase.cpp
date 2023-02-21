#include "TransferNotificationBuilderBase.h"

#include <megaapi.h>
#include "MegaApplication.h"

#include <QString>

TransferNotificationBuilderBase::TransferNotificationBuilderBase(const std::shared_ptr<TransferMetaData> &data)
    : data(data)
{
}

FinishedTransferNotificationInfo TransferNotificationBuilderBase::buildNotification()
{
    FinishedTransferNotificationInfo info;

    if(data->isSingleTransfer())
    {
        if (data->isUpload())
        {
            auto node = UploadTransferMetaData::getDestinationNodeByData(data);
            info.message = buildSingleUploadMessage(node);
            info.title = buildSingleUploadTitle(node);
            info.actions = buildSingleUploadActions(node);
        }
        else if (data->isDownload())
        {
            auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(data);
            info.message = buildSingleDownloadMessage(destinationPath);
            info.title = buildSingleDownloadTitle(destinationPath);
            info.actions = buildSingleDownloadActions(destinationPath);
        }
    }
    else
    {
        if (data->isUpload())
        {
            auto node = UploadTransferMetaData::getDestinationNodeByData(data);
            info.message = buildMultipleUploadMessage(node);
            info.title = buildMultipleUploadTitle(node);
            info.actions = buildMultipleUploadActions(node);
        }
        else if (data->isDownload())
        {
            auto destinationPath = DownloadTransferMetaData::getDestinationNodePathByData(data);
            info.message = buildMultipleDownloadMessage(destinationPath);
            info.title = buildMultipleDownloadTitle(destinationPath);
            info.actions = buildMultipleDownloadActions(destinationPath);
        }
    }

    info.imagePath = getImagePath();

    return info;
}

QString TransferNotificationBuilderBase::buildSingleUploadMessage(std::shared_ptr<mega::MegaNode> node)
{
    auto nodePath = QString::fromStdString(MegaSyncApp->getMegaApi()->getNodePath(node.get()));
    return data->isFileTransfer() ? tr("Uploaded 1 file to %1.").arg(nodePath)
                                  : tr("Uploaded 1 folder to %1.").arg(nodePath);
}

QString TransferNotificationBuilderBase::buildSingleUploadTitle(std::shared_ptr<mega::MegaNode>)
{
    return data->isFileTransfer() ? tr("File uploaded") : tr("Folder uploaded");
}

QStringList TransferNotificationBuilderBase::buildSingleUploadActions(std::shared_ptr<mega::MegaNode>)
{
    QStringList actions = QStringList() << tr("Show in MEGA");
    auto nodes = UploadTransferMetaData::getNodesByData(data);
    if(!nodes.isEmpty())
    {
        actions << tr("Get link");
    }
    return actions;
}

QString TransferNotificationBuilderBase::buildSingleDownloadMessage(const QString &destinationPath)
{
    return data->isFileTransfer() ? tr("Downloaded 1 file to %1.").arg(destinationPath)
                                  : tr("Downloaded 1 folder to %1.").arg(destinationPath);
}

QStringList TransferNotificationBuilderBase::buildSingleDownloadActions(const QString&)
{
    return QStringList() << tr("Show in folder") << tr("Open");
}

QString TransferNotificationBuilderBase::buildSingleDownloadTitle(const QString&)
{
    return data->isFileTransfer() ? tr("File downloaded") : tr("Folder downloaded");
}

QString TransferNotificationBuilderBase::buildMultipleUploadMessage(std::shared_ptr<mega::MegaNode> node)
{
    QString message;

    if(data->getTransfersOK() > 0)
    {
        if(data->getTransfersCancelled() > 0 || data->getTransfersFailed() > 0)
        {
            message = tr("%n upload successful", "", data->getTransfersOK());

            if(data->getTransfersCancelled() > 0)
            {
                auto cancelledStringPart = tr(", %n upload cancelled", "", data->getTransfersCancelled());
                message.append(cancelledStringPart);
            }

            if(data->getTransfersFailed() > 0)
            {
                auto failedStringPart = tr(", %n upload failed", "", data->getTransfersFailed());
                message.append(failedStringPart);
            }
        }
        else
        {
            auto nodePath = QString::fromStdString(MegaSyncApp->getMegaApi()->getNodePath(node.get()));

            message = tr("Uploaded %n items to %1", "", data->getTransfersOK()).arg(nodePath);
        }

        message.append(QString::fromLatin1("."));
    }

    return message;
}

QStringList TransferNotificationBuilderBase::buildMultipleUploadActions(std::shared_ptr<mega::MegaNode>)
{
    return QStringList() << tr("Show in MEGA");
}

QString TransferNotificationBuilderBase::buildMultipleUploadTitle(std::shared_ptr<mega::MegaNode>)
{
    return tr("Items uploaded");
}

QString TransferNotificationBuilderBase::buildMultipleDownloadMessage(const QString &destinationPath)
{
    QString message;

    if(data->getTransfersOK() > 0)
    {
        if(data->getTransfersCancelled() > 0 || data->getTransfersFailed() > 0)
        {
            message = tr("%n download successful", "", data->getTransfersOK());

            if(data->getTransfersCancelled() > 0)
            {
                auto cancelledStringPart = tr(", %n download cancelled", "", data->getTransfersCancelled());
                message.append(cancelledStringPart);
            }

            if(data->getTransfersFailed() > 0)
            {
                auto failedStringPart = tr(", %n download failed", "", data->getTransfersFailed());
                message.append(failedStringPart);
            }
        }
        else
        {
            message = tr("Downloaded %n items to %1", "", data->getTransfersOK()).arg(destinationPath);
        }

        message.append(QString::fromLatin1("."));
    }

    return message;
}

QStringList TransferNotificationBuilderBase::buildMultipleDownloadActions(const QString &)
{
    return QStringList() << tr("Show in folder");
}

QString TransferNotificationBuilderBase::buildMultipleDownloadTitle(const QString &)
{
    return tr("Items downloaded");
}

QString TransferNotificationBuilderBase::getImagePath()
{
    return QString();
}
