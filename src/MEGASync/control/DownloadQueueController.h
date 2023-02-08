#ifndef DOWNLOADQUEUECONTROLLER_H
#define DOWNLOADQUEUECONTROLLER_H

#include "control/TransferBatch.h"
#include "control/Utilities.h"
#include "megaapi.h"
#include <QTMegaRequestListener.h>
#include "TransferMetaData.h"

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

    void addTransferBatch(std::shared_ptr<TransferBatch> batch);
    void removeBatch();

    int getDownloadQueueSize();
    bool isDownloadQueueEmpty();
    void clearDownloadQueue();
    WrappedNode* dequeueDownloadQueue();

    unsigned long long getCurrentAppDataId() const;
    const QString& getCurrentTargetPath() const;

signals:
    void finishedAvailableSpaceCheck(bool isDownloadPossible);

protected:
    void onRequestFinish(mega::MegaApi*, mega::MegaRequest *request, mega::MegaError *e) override;

private:

    bool isDownloadPossible();
    bool hasEnoughSpaceForDownloads();
    bool shouldRetryWhenNotEnoughSpace();

    mega::MegaApi *mMegaApi;
    const QMap<mega::MegaHandle, QString>& mPathMap;
    std::unique_ptr<mega::QTMegaRequestListener> mListener;
    std::atomic<long> mFolderCountPendingSizeComputation;
    std::atomic<qint64> mTotalQueueDiskSize;
    unsigned long long mCurrentAppDataId;
    QString mCurrentTargetPath;
    BlockingBatch* mDownloadBatches;
    QQueue<WrappedNode*>* mDownloadQueue;
};
#endif // DOWNLOADQUEUECONTROLLER_H
