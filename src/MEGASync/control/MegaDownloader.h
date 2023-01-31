#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include "control/DownloadQueueController.h"
#include "control/Utilities.h"
#include "control/TransferBatch.h"
#include "FolderTransferListener.h"
#include "TransferMetaData.h"
#include <QTMegaRequestListener.h>

#include "megaapi.h"

#include <QObject>
#include <QString>
#include <QFileInfo>
#include <QDir>
#include <QQueue>
#include <QMap>

class DownloadTransferMetaData;

class MegaDownloader : public QObject
{
    Q_OBJECT

public:
    // If you want to manage public transfers in a different MegaApi object,
    // provide megaApiGuest
    MegaDownloader(mega::MegaApi* _megaApi, std::shared_ptr<FolderTransferListener> _listener);
    virtual ~MegaDownloader() = default;

    bool processDownloadQueue(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                              const QString &path);
    bool isQueueProcessingOngoing();
protected:
    bool download(WrappedNode *parent, QFileInfo info, const std::shared_ptr<DownloadTransferMetaData>& data, mega::MegaCancelToken *cancelToken);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void startingTransfers();
    void folderTransferUpdated(const FolderTransferUpdateEvent& event);

private slots:
    void onAvailableSpaceCheckFinished(bool isDownloadPossible);

private:
    void startDownload(WrappedNode* parent, const QString &appData,
                       const QString &currentPathWithSep, mega::MegaCancelToken* cancelToken);
    void downloadForeignDir(mega::MegaNode *node, const std::shared_ptr<DownloadTransferMetaData>& data, const QString &currentPathWithSep);

    QString buildEscapedPath(const char* nodeName, QString currentPathWithSep);
    bool createDirIfNotPresent(const QString &path);
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);

    static QString createPathWithSeparator(const QString& path);

    bool mNoTransferStarted = true;
    bool mProcessingTransferQueue = false;
    std::shared_ptr<FolderTransferListener> listener;
    DownloadQueueController mQueueData;
};

#endif // MEGADOWNLOADER_H
