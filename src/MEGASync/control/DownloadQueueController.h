#ifndef DOWNLOADQUEUECONTROLLER_H
#define DOWNLOADQUEUECONTROLLER_H

#include "control/TransferBatch.h"
#include "control/Utilities.h"
#include "megaapi.h"
#include <QTMegaRequestListener.h>
#include "TransferMetadata.h"

#include <QMap>
#include <QObject>
#include <QString>

class DownloadQueueController : public QObject, public mega::MegaRequestListener
{
    Q_OBJECT

public:
    DownloadQueueController(mega::MegaApi* _megaApi, const QMap<mega::MegaHandle, QString>& pathMap);

    void initialize(QQueue<WrappedNode*>* downloadQueue, BlockingBatch& downloadBatches,
                    unsigned long long appDataId, const QString& path);

    void startAvailableSpaceChecking();

    QList<QString>::ConstIterator getAppDatasBegin();
    QList<QString>::ConstIterator getPathsBegin();

    void addTransferBatch(std::shared_ptr<TransferBatch> batch);
    void removeBatch();

    int getDownloadQueueSize();
    bool isDownloadQueueEmpty();
    void clearDownloadQueue();
    WrappedNode* dequeueDownloadQueue();

signals:
    void finishedAvailableSpaceCheck(bool isDownloadPossible);

protected:
    void onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError *e) override;

private:
    void prepareAppDatas(const QString& appId, TransferMetaData* metadata);
    void preparePaths(const QString &path, TransferMetaData* metadata);
    void update(TransferMetaData* dataToUpdate, mega::MegaNode* node, const QString& path);

    bool isDownloadPossible();
    bool hasEnoughSpaceForDownloads();
    bool shouldRetryWhenNotEnoughSpace();

    mega::MegaApi *mMegaApi;
    const QMap<mega::MegaHandle, QString>& mPathMap;
    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    std::atomic<long> mFolderCountPendingSizeComputation;
    std::atomic<qint64> mTotalQueueDiskSize;
    QStringList mAppDatas;
    QStringList mPaths;
    BlockingBatch* mDownloadBatches;
    QQueue<WrappedNode*>* mDownloadQueue;
};
#endif // DOWNLOADQUEUECONTROLLER_H
