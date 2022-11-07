#ifndef TRANSFERSMODEL_H
#define TRANSFERSMODEL_H

#include "QTMegaTransferListener.h"
#include "TransferItem.h"
#include "TransferRemainingTime.h"
#include "control/Preferences.h"

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

    int completedDownloads()const {return totalDownloads - pendingDownloads - failedDownloads;}
    int completedUploads() const {return totalUploads - pendingUploads - failedUploads;}
    int pendingTransfers() const {return pendingDownloads + pendingUploads;}

    long long totalFailedTransfers() const {return failedUploads + failedDownloads;}

    void clear()
    {
        totalUploads = 0;
        totalDownloads = 0;
        pendingUploads = 0;
        pendingDownloads = 0;
        failedUploads = 0;
        failedDownloads = 0;
        completedUploadBytes = 0;
        completedDownloadBytes = 0;
        totalUploadBytes = 0;
        totalDownloadBytes = 0;
        transfersByType.clear();
        transfersFinishedByType.clear();
    }
};

struct LastTransfersCount : public TransfersCount
{
    QSet<int> completedUploadsByTag;
    QSet<int> completedDownloadsByTag;

    void clear()
    {
        TransfersCount::clear();
        completedUploadsByTag.clear();
        completedDownloadsByTag.clear();
    }

};

class TransferThread :  public QObject,public mega::MegaTransferListener
{
    Q_OBJECT
public:
    struct TransfersToProcess
    {
        QList<QExplicitlySharedDataPointer<TransferData>> updateTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> startTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> startSyncTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> canceledTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;

        bool isEmpty(){return updateTransfersByTag.isEmpty()
                              && startTransfersByTag.isEmpty()
                              && startSyncTransfersByTag.isEmpty()
                              && canceledTransfersByTag.isEmpty()
                              && failedTransfersByTag.isEmpty();}

        void clear(){
            updateTransfersByTag.clear();
            startTransfersByTag.clear();
            startSyncTransfersByTag.clear();
            canceledTransfersByTag.clear();
            failedTransfersByTag.clear();
        }
    };

    TransferThread();
    ~TransferThread(){}

    TransfersCount getTransfersCount();
    LastTransfersCount getLastTransfersCount();

    void resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData> > transfersToReset);
    void resetCompletedDownloads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset);
    void resetCompletedTransfers();

    void setMaxTransfersToProcess(uint16_t max);

    TransfersToProcess processTransfers();
    void clear();

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
    bool checkIfRepeatedAndSubstituteInStartTransfers(mega::MegaTransfer *transfer);

    struct cacheTransfers
    {
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> updateTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> startTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> startSyncTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> canceledTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;

        void clear()
        {
            updateTransfersByTag.clear();
            startTransfersByTag.clear();
            startTransfersByTag.clear();
            canceledTransfersByTag.clear();
            failedTransfersByTag.clear();
        }
    };

    cacheTransfers mTransfersToProcess;
    QMutex mCacheMutex;
    QMutex mCountersMutex;
    TransfersCount mTransfersCount;
    LastTransfersCount mLastTransfersCount;
    std::atomic<int16_t> mMaxTransfersToProcess;
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
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                                int column, const QModelIndex& parent);
    bool hasChildren(const QModelIndex& parent) const;
    int rowCount(const QModelIndex& parent = QModelIndex()) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;

    void ignoreMoveRowsSignal(bool state);
    void inverseMoveRowsSignal(bool state);
    bool moveTransferPriority(const QModelIndex& sourceParent, const QList<int>& rows,
                  const QModelIndex& destinationParent, int destinationChild);

    void resetModel();

    void getLinks(QList<int>& rows);
    void openInMEGA(QList<int>& rows);
    void openFolderByIndex(const QModelIndex& index);
    void openFolderByTag(TransferTag tag);
    void retryTransferByIndex(const QModelIndex& index);
    void retryTransfers(QModelIndexList indexes);
    void cancelAndClearTransfers(const QModelIndexList& indexes, QWidget *canceledFrom);
    void cancelAllTransfers(QWidget *canceledFrom);
    void clearAllTransfers();
    void clearTransfers(const QModelIndexList& indexes);
    void clearFailedTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> uploads,
                        const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> downloads);
    void performClearTransfers(const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> uploads,
                        const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>> downloads);
    void classifyUploadOrDownloadCompletedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void classifyUploadOrDownloadFailedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);
    void pauseResumeTransferByIndex(const QModelIndex& index, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForFileType(Utilities::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(Utilities::FileType fileType) const;
    TransfersCount getTransfersCount();
    TransfersCount getLastTransfersCount();
    long long failedTransfers();

    void startTransfer(QExplicitlySharedDataPointer<TransferData> transfer);
    void updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer, int row);

    void pauseModelProcessing(bool value);

    bool areAllPaused() const;

    QExplicitlySharedDataPointer<TransferData> getTransferByTag(int tag) const;
    int getRowByTransferTag(int tag) const;
    void sendDataChangedByTag(int tag);

    void blockModelSignals(bool state);

    int hasActiveTransfers() const;
    void setActiveTransfer(TransferTag tag);
    void unsetActiveTransfer(TransferTag tag);
    void checkActiveTransfer(TransferTag tag, bool isActive);

    void uiUnblocked();

    bool syncsInRowsToCancel() const;
    QWidget *cancelledFrom() const;
    void resetSyncInRowsToCancel();
    void showSyncCancelledWarning();

    QList<int> getDragAndDropRows(const QMimeData* data);

signals:
    void pauseStateChanged(bool pauseState);
    void transferPauseStateChanged();
    void transfersCountUpdated();
    void processTransferInThread();
    void pauseStateChangedByTransferResume();
    void blockUi();
    void unblockUi();
    void unblockUiAndFilter();
    void modelProcessingFinished();
    void transferFinished(const QModelIndex& index);
    void internalMoveStarted() const;
    void internalMoveFinished() const;
    void mostPriorityTransferUpdate(int uploadTag, int downloadTag);
    void transfersProcessChanged();
    void showInFolderFinished(bool);
    void activeTransfersChanged();
    void rowsAboutToBeMoved(TransferTag firstRowTag);

public slots:
    void pauseResumeAllTransfers(bool state);
    void askForMostPriorityTransfer();

private slots:
    void processStartTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersToStart);
    void processUpdateTransfers();
    void processCancelTransfers();
    void processSyncFailedTransfers();
    void cacheCancelTransfersTags();
    void processFailedTransfers();
    void onProcessTransfers();
    void updateTransfersCount();
    void onClearTransfersFinished();
    void onAskForMostPriorityTransfersFinished();
    void onKeepPCAwake();

private:
    void removeRows(QModelIndexList &indexesToRemove);
    QExplicitlySharedDataPointer<TransferData> getTransfer(int row) const;
    void addTransfer(QExplicitlySharedDataPointer<TransferData>);
    void removeTransfer(int row);
    void sendDataChanged(int row);

    bool isUiBlockedModeActive() const ;
    void setUiBlockedMode(bool state);

    void setUiBlockedModeByCounter(uint32_t transferCount);
    void updateUiBlockedByCounter(int updates);
    bool isUiBlockedByCounter() const;
    void setUiBlockedByCounterMode(bool state);

    void modelHasChanged(bool state);

    void mostPriorityTransferMayChanged(bool state);

    int performPauseResumeAllTransfers(int activeTransfers, bool useEventUpdater);
    int performPauseResumeVisibleTransfers(const QModelIndexList& indexes, bool pauseState, bool useEventUpdater);

private:
    mega::MegaApi* mMegaApi;
    std::shared_ptr<Preferences> mPreferences;
    QThread* mTransferEventThread;
    TransferThread* mTransferEventWorker;
    mega::QTMegaTransferListener *mDelegateListener;
    QTimer mProcessTransfersTimer;
    TransfersCount mTransfersCount;
    LastTransfersCount mLastTransfersCount;

    QList<QExplicitlySharedDataPointer<TransferData>> mTransfers;

    TransferThread::TransfersToProcess mTransfersToProcess;
    QFutureWatcher<void> mUpdateTransferWatcher;
    QFutureWatcher<void> mClearTransferWatcher;
    QFutureWatcher<QPair<int, int>> mAskForMostPriorityTransfersWatcher;

    uint8_t mTransfersProcessChanged;
    uint8_t mUpdateMostPriorityTransfer;
    uint8_t mUiBlockedCounter;

    int mUiBlockedByCounter;
    uint8_t  mUiBlockedByCounterSafety;

    QHash<TransferTag, QPersistentModelIndex> mTagByOrder;
    QList<TransferTag> mRowsToCancel;
    QWidget* mCancelledFrom;
    bool mSyncsInRowsToCancel;

    QList<TransferTag> mFailedTransferToClear;
    mutable QMutex mModelMutex;
    QTimer mMostPriorityTransferTimer;

    bool mAreAllPaused;
    bool mHasActiveTransfers;
    QSet<TransferTag> mActiveTransfers;

    bool mIgnoreMoveSignal;
    bool mInverseMoveSignal;
};

Q_DECLARE_METATYPE(QAbstractItemModel::LayoutChangeHint)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QSet<int>)

#endif // TRANSFERSMODEL_H
