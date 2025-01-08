#ifndef MEGADOWNLOADER_H
#define MEGADOWNLOADER_H

#include "DownloadQueueController.h"
#include "Utilities.h"
#include "TransferBatch.h"
#include "FolderTransferListener.h"
#include "TransferMetaData.h"
#include "QTMegaTransferListener.h"

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
    MegaDownloader(mega::MegaApi* megaApi, std::shared_ptr<mega::MegaTransferListener> listener);
    MegaDownloader(mega::MegaApi* megaApi, mega::MegaTransferListener* listener);
    virtual ~MegaDownloader() = default;

    struct DownloadInfo
    {
        QQueue<WrappedNode> downloadQueue;
        BlockingBatch* downloadBatches = nullptr;
        QString path;
        unsigned long long appId = TransferMetaData::INVALID_ID;
        bool createAppId = true;

        bool checkLocalSpace = true;
    };

    void processDownloadQueue(DownloadInfo& info);

protected:
    void download(const WrappedNode& parent,
                  QFileInfo info,
                  const std::shared_ptr<DownloadTransferMetaData>& data,
                  mega::MegaCancelToken* cancelToken);

    mega::MegaApi *megaApi;
    QMap<mega::MegaHandle, QString> pathMap;

signals:
    void startingTransfers();

private slots:
    void onAvailableSpaceCheckFinished(bool isDownloadPossible);

private:
    void startDownload(const WrappedNode& parent,
                       const QString& appData,
                       const QString& currentPathWithSep,
                       mega::MegaCancelToken* cancelToken);
    void downloadForeignDir(mega::MegaNode *node, const std::shared_ptr<DownloadTransferMetaData>& data, const QString &currentPathWithSep);

    QString buildEscapedPath(const char* nodeName, QString currentPathWithSep);
    bool createDirIfNotPresent(const QString &path);
    static bool hasTransferPriority(const WrappedNode::TransferOrigin& origin);

    static QString createPathWithSeparator(const QString& path);

    bool mNoTransferStarted = true;
    std::shared_ptr<mega::MegaTransferListener> mTransferListener;
    std::shared_ptr<mega::QTMegaTransferListener> mTransferListenerDelegate;
    DownloadQueueController mQueueData;
};

#endif // MEGADOWNLOADER_H
