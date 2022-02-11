#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include "QTMegaTransferListener.h"
#include "TransferItem.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QLinkedList>
#include <QtConcurrent/QtConcurrent>
#include <QDebug>


#include <set>

struct TransfersCount
{
    long long leftUploadBytes;
    long long completedUploadBytes;
    long long leftDownloadBytes;
    long long completedDownloadBytes;

    int activeDownloadState;
    int activeUploadState;
    int remainingUploads;
    int remainingDownloads;
    int completedUploads;
    int completedDownloads;

    int totalUploads;
    int totalDownloads;
    int currentUpload;
    int currentDownload;

    TransfersCount():
        leftUploadBytes(0),
        completedUploadBytes(0),
        leftDownloadBytes(0),
        completedDownloadBytes(0),
        activeDownloadState(0),
        activeUploadState(0),
        remainingUploads(0),
        remainingDownloads(0),
        completedDownloads(0),
        completedUploads(0),
        totalUploads(0),
        totalDownloads(0),
        currentUpload(0),
        currentDownload(0)
    {}
};

class TransferThread :  public QObject,public mega::MegaTransferListener
{
    Q_OBJECT
public:
    TransferThread();
    ~TransferThread(){}
public slots:
    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError*);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi*,mega::MegaTransfer* transfer,mega::MegaError*);

    std::list<mega::MegaTransfer*> processUpdates();

private:
    std::list<mega::MegaTransfer*> mCacheUpdateTransfers;
    void onTransferEvent(mega::MegaTransfer* transfer);

    QReadWriteLock* mCacheMutex;
};

class QTransfersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit QTransfersModel(QObject* parent = 0);
    ~QTransfersModel();

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                                int column, const QModelIndex& parent);
    bool hasChildren(const QModelIndex& parent) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                  const QModelIndex& destinationParent, int destinationChild);
    bool areAllPaused();

    void getLinks(QList<int>& rows);
    void openFolderByIndex(const QModelIndex& index);
    void cancelClearTransfers(const QModelIndexList& indexes, bool clearAll);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForFileType(TransferData::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(TransferData::FileType fileType) const;

    const TransfersCount& getTransfersCount();
    void resetCompletedTransfersCount();

    void initModel();

    void startTransfer(mega::MegaTransfer *transfer);
    int updateTransfer(mega::MegaTransfer* transfer);

    void pauseModelProcessing(bool value);

    static QHash<QString, TransferData::FileType> mFileTypes;

signals:
    void pauseStateChanged(bool pauseState);
    void transfersDataUpdated();
    void processTransferInThread();

public slots:
    void onRetryTransfer(TransferTag tag);
    void pauseResumeAllTransfers();
    void cancelClearAllTransfers();

private slots:
    void onPauseStateChanged();
    void processStartTransfers(std::list<mega::MegaTransfer*> transferList);
    void processUpdateTransfers(std::list<mega::MegaTransfer*> transferList);
    void processCancelTransfers(std::list<mega::MegaTransfer*> transferList);

    void onProcessTransfers();

private:
    void insertTransfer(mega::MegaTransfer* transfer);
    void updateTransfersCount();

private:
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;
    QThread* mTransferEventThread;
    TransferThread* mTransferEventWorker;
    QTimer mTimer;

    QHash<TransferTag, QExplicitlySharedDataPointer<TransferData>> mTransfers;
    QList<TransferTag> mOrder;
    QHash<TransferTag, int> mTagByOrder;
    QList<int> mRowsToUpdate;
    ThreadPool* mThreadPool;
    QReadWriteLock* mModelMutex;

    long long mUpdateNotificationNumber;

    TransfersCount mTransfersCount;

    bool mAreAllPaused;
    bool stopModelProcessing;

    QMap<TransferData::FileType, long long> mNbTransfersPerFileType;
    QMap<TransferData::FileType, long long> mNbFinishedPerFileType;
};

#endif // QTRANSFERSMODEL_H
