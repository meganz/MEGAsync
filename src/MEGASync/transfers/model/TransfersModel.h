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

    int failedUploads;
    int failedDownloads;

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
        failedUploads(0),
        failedDownloads(0),
        completedUploadBytes(0),
        completedDownloadBytes(0),
        totalUploadBytes(0),
        totalDownloadBytes(0)
    {}

    int completedDownloads(){return totalDownloads - pendingDownloads - failedDownloads;}
    int completedUploads(){return totalUploads - pendingUploads - failedUploads;}

};

class TransferThread :  public QObject,public mega::MegaTransferListener
{
    Q_OBJECT
public:
    struct TransfersToProcess
    {
        QList<QExplicitlySharedDataPointer<TransferData>> updateTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> startTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> canceledTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;

        bool isEmpty(){return updateTransfersByTag.isEmpty()
                              && startTransfersByTag.isEmpty()
                              && canceledTransfersByTag.isEmpty()
                              && failedTransfersByTag.isEmpty();}
    };

    TransferThread(){auxCounter = 0;}
    ~TransferThread(){}

    TransfersCount getTransfersCount();
    void resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData> > transfersToReset);
    void resetCompletedDownloads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset);
    void resetCompletedTransfers();

    TransfersToProcess processTransfers();

public slots:
    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError*);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi*,mega::MegaTransfer* transfer,mega::MegaError*);

private:
    QExplicitlySharedDataPointer<TransferData> createData(mega::MegaTransfer* transfer);
    QExplicitlySharedDataPointer<TransferData> onTransferEvent(mega::MegaTransfer* transfer);
    QList<QExplicitlySharedDataPointer<TransferData>> extractFromCache(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, int spaceForTransfers);
    bool checkIfRepeatedAndRemove(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, mega::MegaTransfer *transfer);
    bool checkIfRepeatedAndSubstitute(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, mega::MegaTransfer *transfer);

    struct cacheTransfers
    {
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> updateTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> startTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> canceledTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;
    };

    cacheTransfers mTransfersToProcess;
    QMutex mCacheMutex;
    QMutex mCountersMutex;
    TransfersCount mTransfersCount;
    int auxCounter;

};


class TransfersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    static const char* SORTED_BY_STATE;

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
    void retryTransfers(QModelIndexList indexes);
    void cancelTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> uploads,
                        const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> downloads);
    void classifyUploadOrDownloadTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);
    void pauseResumeTransferByIndex(const QModelIndex& index, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForFileType(Utilities::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(Utilities::FileType fileType) const;
    TransfersCount getTransfersCount();

    void startTransfer(QExplicitlySharedDataPointer<TransferData> transfer);
    void updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer);

    void pauseModelProcessing(bool value);

    bool areAllPaused() const;

signals:
    void pauseStateChanged(bool pauseState);
    void transferPauseStateChanged();
    void transfersCountUpdated();
    void processTransferInThread();
    void pauseStateChangedByTransferResume();
    void blockUi();
    void unblockUi();
    void modelProcessingFinished();
    void transferFinished(const QModelIndex& index);

public slots:
    void pauseResumeAllTransfers(bool state);

private slots:
    void onPauseStateChanged();
    void processStartTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersToStart);
    void processActiveTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersActive);
    void processUpdateTransfers();
    void processCancelTransfers();
    void processFailedTransfers();
    void onProcessTransfers();

private:
    void updateTransfersCount();
    void removeRows(QModelIndexList &indexesToRemove);
    QExplicitlySharedDataPointer<TransferData> getTransfer(int row) const;
    void removeTransfer(int row);
    void sendDataChanged(int row);

    bool isFailingModeActive() const ;
    void setFailingMode(bool state);

    bool isCancelingModeActive() const ;
    void setCancelingMode(bool state);

    void updateTagsByOrder();

    void updateTransferPriority(QExplicitlySharedDataPointer<TransferData> transfer);

private:
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;
    QThread* mTransferEventThread;
    TransferThread* mTransferEventWorker;
    QTimer mTimer;

    QList<QExplicitlySharedDataPointer<TransferData>> mTransfers;

    TransferThread::TransfersToProcess mTransfersToProcess;

    uint8_t mCancelingMode;
    uint8_t mFailingMode;

    QHash<TransferTag, int> mTagByOrder;
    mutable QMutex mModelMutex;
    mega::QTMegaTransferListener *delegateListener;

    bool mAreAllPaused;
    bool stopModelProcessing;
};

#endif // TRANSFERSMODEL_H
