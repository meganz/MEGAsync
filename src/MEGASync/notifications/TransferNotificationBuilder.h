#ifndef TRANSFERNOTIFICATIONBUILDER_H
#define TRANSFERNOTIFICATIONBUILDER_H

#include "model/TransferMetaData.h"

#include <megaapi.h>
#include <QObject>

struct FinishedTransferNotificationInfo
{
    QString title;
    QString message;
    QString imagePath;
    QStringList actions;
};

class TransferNotificationBuilder : public QObject
{
    Q_OBJECT

public:
    TransferNotificationBuilder(const std::shared_ptr<TransferMetaData>& data);
    ~TransferNotificationBuilder() = default;

    FinishedTransferNotificationInfo buildNotification();

protected:
    QString buildUploadTitle();
    QString buildDownloadTitle();

    QString buildSingleUploadMessage();
    QStringList buildSingleUploadActions();

    QString buildSingleDownloadMessage(const QString& destinationPath);
    QStringList buildSingleDownloadActions(const QString &);

    QString buildMultipleUploadMessage();
    QStringList buildMultipleUploadActions();

    QString buildMultipleDownloadMessage(const QString& destinationPath);
    QStringList buildMultipleDownloadActions(const QString &);

    QString buildSingleNonExistentDataMessageUpload(const QString& itemName);
    QString buildSingleNonExistentDataMessageDownload(const QString& itemName);
    QString buildNonExistentItemsMessageUploads();
    QString buildNonExistentItemsMessageDownloads();

    QString getImagePath();

    bool isFolder() const;

    const std::shared_ptr<TransferMetaData> data;
};

#endif // TRANSFERNOTIFICATIONBUILDER_H
