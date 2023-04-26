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
    QString buildSingleUploadMessage();
    QString buildSingleUploadTitle();
    QStringList buildSingleUploadActions();

    QString buildSingleDownloadMessage(const QString& destinationPath);
    QString buildSingleDownloadTitle(const QString &);
    QStringList buildSingleDownloadActions(const QString &);

    QString buildMultipleUploadMessage();
    QString buildMultipleUploadTitle();
    QStringList buildMultipleUploadActions();

    QString buildMultipleDownloadMessage(const QString& destinationPath);
    QString buildMultipleDownloadTitle(const QString &);
    QStringList buildMultipleDownloadActions(const QString &);

    QString getImagePath();

    const std::shared_ptr<TransferMetaData> data;
};

#endif // TRANSFERNOTIFICATIONBUILDER_H
