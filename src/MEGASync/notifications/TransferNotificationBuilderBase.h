#ifndef TRANSFERNOTIFICATIONBUILDERBASE_H
#define TRANSFERNOTIFICATIONBUILDERBASE_H

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

class TransferNotificationBuilderBase : public QObject
{
    Q_OBJECT

public:
    TransferNotificationBuilderBase(const std::shared_ptr<TransferMetaData>& data);
    virtual ~TransferNotificationBuilderBase() = default;

    FinishedTransferNotificationInfo buildNotification();

protected:
    virtual QString buildSingleUploadMessage(std::shared_ptr<mega::MegaNode> node);
    virtual QString buildSingleUploadTitle(std::shared_ptr<mega::MegaNode>);
    virtual QStringList buildSingleUploadActions(std::shared_ptr<mega::MegaNode>);

    virtual QString buildSingleDownloadMessage(const QString& destinationPath);
    virtual QString buildSingleDownloadTitle(const QString &);
    virtual QStringList buildSingleDownloadActions(const QString &);

    virtual QString buildMultipleUploadMessage(std::shared_ptr<mega::MegaNode> node);
    virtual QString buildMultipleUploadTitle(std::shared_ptr<mega::MegaNode>);
    virtual QStringList buildMultipleUploadActions(std::shared_ptr<mega::MegaNode>);

    virtual QString buildMultipleDownloadMessage(const QString& destinationPath);
    virtual QString buildMultipleDownloadTitle(const QString &);
    virtual QStringList buildMultipleDownloadActions(const QString &);

    virtual QString getImagePath();

    const std::shared_ptr<TransferMetaData> data;
};

#endif // TRANSFERNOTIFICATIONBUILDERBASE_H
