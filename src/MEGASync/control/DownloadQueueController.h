#ifndef DOWNLOADQUEUECONTROLLER_H
#define DOWNLOADQUEUECONTROLLER_H

#include "TransferBatch.h"
#include "Utilities.h"
#include "megaapi.h"
#include "drivedata.h"

#include <QMap>
#include <QObject>
#include <QString>
#include <QStorageInfo>

class DownloadQueueController : public QObject
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

    void onRequestFinish(mega::MegaRequest *request, mega::MegaError *e);

signals:
    void finishedAvailableSpaceCheck(bool isDownloadPossible);

private:

    void tryDownload();
    bool hasEnoughSpaceForDownloads();
    void askUserForChoice();
    DriveDisplayData getDriveDisplayData(const QStorageInfo& driveInfo) const;
    QString getDefaultDriveName() const;
    QString getDriveIcon() const;
    DriveSpaceData getDriveSpaceDataFromQt();

    mega::MegaApi *mMegaApi;
    const QMap<mega::MegaHandle, QString>& mPathMap;
    std::atomic<long> mFolderCountPendingSizeComputation;
    std::atomic<long long> mTotalQueueDiskSize;
    unsigned long long mCurrentAppDataId;
    QString mCurrentTargetPath;
    BlockingBatch* mDownloadBatches;
    QQueue<WrappedNode*>* mDownloadQueue;

    DriveSpaceData mCachedDriveData;
};
#endif // DOWNLOADQUEUECONTROLLER_H
