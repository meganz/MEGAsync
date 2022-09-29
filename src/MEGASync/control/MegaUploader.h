#ifndef MEGAUPLOADER_H
#define MEGAUPLOADER_H

#include <TransferBatch.h>
#include "Preferences.h"
#include "megaapi.h"
#include <ThreadPool.h>

#include <QFutureWatcher>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>

class MegaUploader : public QObject
{
    Q_OBJECT

public:
    MegaUploader(mega::MegaApi *megaApi);
    virtual ~MegaUploader();

    void upload(QString path, const QString &nodeName, std::shared_ptr<mega::MegaNode> parent, unsigned long long appDataID, const std::shared_ptr<TransferBatch> &transferBatch);

signals:
    void startingTransfers(bool canBeCancelled);

private:
    void startUpload(const QString& localPath, const QString& nodeName, unsigned long long appDataID, mega::MegaNode* parent, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    ThreadPool* mThreadPool;
};

#endif // MEGAUPLOADER_H
