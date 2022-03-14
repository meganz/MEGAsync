#ifndef TRANSFERSMODEL_H
#define TRANSFERSMODEL_H

#include "QTMegaTransferListener.h"
#include "TransferItem.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QLinkedList>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>


#include <set>

struct TransfersCount
{
    int totalUploads;
    int totalDownloads;

    int pendingUploads;
    int pendingDownloads;

    long long completedUploadBytes;
    long long completedDownloadBytes;

    long long totalUploadBytes;
    long long totalDownloadBytes;

    QMap<Utilities::FileType, long long> transfersByType;
    QMap<Utilities::FileType, long long> transfersFinishedByType;

    TransfersCount():
        totalUploads(0),
        totalDownloads(0),
        pendingUploads(0),
        pendingDownloads(0),
        completedUploadBytes(0),
        completedDownloadBytes(0),
        totalUploadBytes(0),
        totalDownloadBytes(0)
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

    QList<QExplicitlySharedDataPointer<TransferData> > processUpdates();

private:
    QExplicitlySharedDataPointer<TransferData> createData(mega::MegaTransfer* transfer);

    QMap<int, QExplicitlySharedDataPointer<TransferData>> mCacheUpdateTransfersByTag;
    QExplicitlySharedDataPointer<TransferData> onTransferEvent(mega::MegaTransfer* transfer);

    QMutex mCacheMutex;
    QMutex mCountersMutex;
    TransfersCount mTransfersCount;
};

class TransfersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TransfersModel(QObject* parent = 0);
    ~TransfersModel();

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
    void openFolderByTag(TransferTag tag);
    void retryTransferByIndex(const QModelIndex& index);
    void cancelTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> uploads,
                        const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> downloads);
    void classifyUploadOrDownloadTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForFileType(Utilities::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(Utilities::FileType fileType) const;
    TransfersCount getTransfersCount();

    void initModel();

    void startTransfer(QExplicitlySharedDataPointer<TransferData> transfer);
    int updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer);

    void pauseModelProcessing(bool value);

    bool areAllPaused();

signals:
    void pauseStateChanged(bool pauseState);
    void transferPauseStateChanged();
    void transfersCountUpdated();
    void processTransferInThread();
    void pauseStateChangedByTransferResume();
    void transfersAboutToBeCanceled();
    void transfersCanceled();

public slots:
    void pauseResumeAllTransfers(bool state);

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
    QMutex mModelMutex;
    mega::QTMegaTransferListener *delegateListener;

    bool mAreAllPaused;
    bool stopModelProcessing;
};

#endif // TRANSFERSMODEL_H
