#include "TransferNotificationMessageBuilder.h"

#include <megaapi.h>

#include <QString>

using namespace mega;

TransferNotificationMessageBuilder::TransferNotificationMessageBuilder(const TransferMetaData *_data)
    : data(_data)
{
}


QString TransferNotificationMessageBuilder::buildTitle()
{
    if (data->transfersFileOK && data->transfersFolderOK)
    {
        if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
        {
            return tr("Upload");
        }
        else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
        {
            return tr("Download");
        }
    }
    else if (data->transfersFileOK)
    {
        if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
        {
            return tr("File Upload");
        }
        else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
        {
            return tr("File Download");
        }
    }
    else if (data->transfersFolderOK)
    {
        if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
        {
            return tr("Folder Upload");
        }
        else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
        {
            return tr("Folder Download");
        }
    }

    return QString();
}

QString TransferNotificationMessageBuilder::buildMessage()
{
    if (data->transfersFileOK && data->transfersFolderOK)
    {
        return buildFileAndFolderMessage();
    }
    else if (data->transfersFileOK)
    {
        return buildFileMessage();
    }
    else if (data->transfersFolderOK)
    {
        return buildFolderMessage();
    }
    return QString();
}

QString TransferNotificationMessageBuilder::buildFileAndFolderMessage()
{
    // Multi plural issue : can't resolve in one single string.
    // see https://stackoverflow.com/questions/51889719/localisation-of-multiple-plurals-in-qt
    QString fileStringPart = tr("%n file", "", data->transfersFileOK);
    QString folderStringPart = tr("%n folder", "", data->transfersFolderOK);

    if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
    {
        if (data->transfersCancelled)
        {
            return tr("%1 and %2 were successfully uploaded, %3").arg(fileStringPart).arg(folderStringPart)
                                                                 .arg(buildCancelStringPart());
        }
        return tr("%1 and %2 were successfully uploaded").arg(fileStringPart).arg(folderStringPart);
    }
    else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (data->transfersCancelled)
        {
            return tr("%1 and %2 were successfully downloaded, %3").arg(fileStringPart).arg(folderStringPart)
                                                                   .arg(buildCancelStringPart());
        }
        return tr("%1 and %2 were successfully downloaded").arg(fileStringPart).arg(folderStringPart);
    }
    return QString();
}

QString TransferNotificationMessageBuilder::buildFileMessage()
{
    if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
    {
        if (data->transfersCancelled)
        {
            QString message = tr("%n file was successfully uploaded, %1", "", data->transfersFileOK);
            return message.arg(buildCancelStringPart());
        }
        else
        {
            return tr("%n file was successfully uploaded", "", data->transfersFileOK);
        }
    }
    else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (data->transfersCancelled)
        {
            QString message = tr("%n file was successfully downloaded, %1", "", data->transfersFileOK);
            return message.arg(buildCancelStringPart());
        }
        else
        {
            return tr("%n file was successfully downloaded", "", data->transfersFileOK);
        }
    }
    return QString();
}

QString TransferNotificationMessageBuilder::buildFolderMessage()
{
    if (data->transferDirection == MegaTransfer::TYPE_UPLOAD)
    {
        if (data->transfersCancelled)
        {
            QString message = tr("%n folder was successfully uploaded, %1", "", data->transfersFolderOK);
            return message.arg(buildCancelStringPart());
        }
        else
        {
            return tr("%n folder was successfully uploaded", "", data->transfersFolderOK);
        }
    }
    else if (data->transferDirection == MegaTransfer::TYPE_DOWNLOAD)
    {
        if (data->transfersCancelled)
        {
            QString message = tr("%n folder was successfully downloaded, %1", "", data->transfersFolderOK);
            return message.arg(buildCancelStringPart());
        }
        else
        {
            return tr("%n folder was successfully downloaded", "", data->transfersFolderOK);
        }
    }
    return QString();
}

QString TransferNotificationMessageBuilder::buildCancelStringPart()
{
    QString cancelledStringPart;
    if (data->transfersCancelled > 0)
    {
        cancelledStringPart = tr("%n transfer was cancelled", "", data->transfersCancelled);
    }
    return cancelledStringPart;
}
