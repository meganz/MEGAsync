#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include "control/Utilities.h"
#include "control/TransferBatch.h"
#include "FolderTransferListener.h"
#include "TransferMetadata.h"

#include "megaapi.h"

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QMap>

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    // If you want to manage public transfers in a different MegaApi object,
    // provide megaApiGuest
    MegaDownloader(mega::MegaApi* _megaApi, std::shared_ptr<FolderTransferListener> _listener);
    virtual ~MegaDownloader() = default;

    bool processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                              const QString &path, unsigned long long appDataId);
    bool isQueueProcessingOngoing();

protected:
    bool download(WrappedNode *parent, QFileInfo info, QString appData, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void finishedTransfers(unsigned long long appDataId);
    void startingTransfers();
    void folderTransferUpdated(const FolderTransferUpdateEvent& event);

private:
    void startDownload(WrappedNode* parent, const QString &appData,
                       const QString &currentPathWithSep, mega::MegaCancelToken* cancelToken);
    void downloadForeignDir(mega::MegaNode *node, const QString &appData, const QString &currentPathWithSep);
    QString buildEscapedPath(const char* nodeName, QString currentPathWithSep);
    bool createDirIfNotPresent(const QString &path);
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);

    void update(TransferMetaData* dataToUpdate, QString& appData, mega::MegaNode* node, const QString& path);

    bool mNoTransferStarted = true;
    bool mProcessingTransferQueue = false;
    std::shared_ptr<FolderTransferListener> listener;
};

#endif // MEGADOWNLOADER_H
