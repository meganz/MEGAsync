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
#include <QFutureWatcher>


#include <set>

struct TransfersCount
{
    int totalUploads;
    int totalDownloads;

    int pendingUploads;
    int pendingDownloads;

    int currentUpload;
    int currentDownload;

    long long completedUploadBytes;
    long long completedDownloadBytes;

    long long totalUploadBytes;
    long long totalDownloadBytes;

    QMap<TransferData::FileType, long long> transfersByType;
    QMap<TransferData::FileType, long long> transfersFinishedByType;

    TransfersCount():
        totalUploadBytes(0),
        completedUploadBytes(0),
        totalDownloadBytes(0),
        completedDownloadBytes(0),
        pendingUploads(0),
        pendingDownloads(0),
        totalUploads(0),
        totalDownloads(0),
        currentUpload(0),
        currentDownload(0)
    {}

    int completedDownloads(){return totalDownloads - pendingDownloads;}
    int completedUploads(){return totalUploads - pendingUploads;}

};

class TransferThread :  public QObject,public mega::MegaTransferListener
{
    Q_OBJECT
public:
    TransferThread();
    ~TransferThread(){}

    TransfersCount getTransfersCount();
    void resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData> > transfersToReset);
    void resetCompletedDownloads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset);
    void resetCompletedTransfers();

public slots:
    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError*);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi*,mega::MegaTransfer* transfer,mega::MegaError*);

    std::list<QExplicitlySharedDataPointer<TransferData>> processUpdates();

private:
    QExplicitlySharedDataPointer<TransferData> createData(mega::MegaTransfer* transfer);

    std::list<QExplicitlySharedDataPointer<TransferData>> mCacheUpdateTransfers;
    void onTransferEvent(mega::MegaTransfer* transfer);

    QReadWriteLock* mCacheMutex;
    TransfersCount mTransfersCount;
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

    void getLinks(QList<int>& rows);
    void openFolderByIndex(const QModelIndex& index);
    void cancelClearTransfers(const QModelIndexList& indexes, bool clearAll);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForFileType(TransferData::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(TransferData::FileType fileType) const;
    TransfersCount getTransfersCount();
    void resetCompletedTransfersCount();
    void resetCompletedUpload(long long transferBytes);
    void resetCompletedDownload(long long transferBytes);

    void initModel();

    void startTransfer(QExplicitlySharedDataPointer<TransferData> transfer);
    int updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer);

    void pauseModelProcessing(bool value);

    bool areAllPaused();

    static QHash<QString, TransferData::FileType> mFileTypes;


signals:
    void pauseStateChanged(bool pauseState);
    void transferPauseStateChanged();
    void transfersCountUpdated();
    void processTransferInThread();
    void pauseStateChangedByTransferResume();
    void transfersAboutToBeCanceled();
    void transfersCanceled();

public slots:
    void onRetryTransfer(TransferTag tag);
    void pauseResumeAllTransfers();
    void cancelClearAllTransfers();

private slots:
    void onPauseStateChanged();
    void processStartTransfers();
    void processUpdateTransfers();
    void processCancelTransfers();

    void onProcessTransfers();


private:
    void updateTransfersCount();
    void removeRows(QModelIndexList &indexesToRemove);
    QExplicitlySharedDataPointer<TransferData> getTransfer(int row) const;
    void removeTransfer(int row);
    void sendDataChanged();

private:
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;
    QThread* mTransferEventThread;
    TransferThread* mTransferEventWorker;
    QTimer mTimer;

    QList<QExplicitlySharedDataPointer<TransferData>> mTransfers;
    std::list<QExplicitlySharedDataPointer<TransferData>> mTransfersToUpdate;
    std::list<QExplicitlySharedDataPointer<TransferData>> mTransfersToCancel;
    std::list<QExplicitlySharedDataPointer<TransferData>> mTransfersToStart;
    bool mTransfersCancelling;
    QHash<TransferTag, int> mTagByOrder;
    QList<int> mRowsToUpdate;
    ThreadPool* mThreadPool;
    QReadWriteLock* mModelMutex;
    mega::QTMegaTransferListener *delegateListener;

    bool mAreAllPaused;
    bool stopModelProcessing;
};

#endif // QTRANSFERSMODEL_H
