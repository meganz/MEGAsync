#ifndef TRANSFERSMODEL_H
#define TRANSFERSMODEL_H

#include "QTMegaTransferListener.h"
#include "TransferItem.h"
#include "TransferMetaData.h"
#include "TransferRemainingTime.h"
#include "control/Preferences.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QLinkedList>
#include <QtConcurrent/QtConcurrent>
#include <QFutureWatcher>
#include <QReadWriteLock>

#include <set>
#include <memory>

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
        QList<QExplicitlySharedDataPointer<TransferData>> failedFolderTransfersByTag;
        QList<QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;

        bool isEmpty(){return updateTransfersByTag.isEmpty()
                              && startTransfersByTag.isEmpty()
                              && startSyncTransfersByTag.isEmpty()
                              && canceledTransfersByTag.isEmpty()
                              && failedFolderTransfersByTag.isEmpty()
                              && failedTransfersByTag.isEmpty();}

        void clear(){
            updateTransfersByTag.clear();
            startTransfersByTag.clear();
            startSyncTransfersByTag.clear();
            canceledTransfersByTag.clear();
            failedFolderTransfersByTag.clear();
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
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError*e);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi*,mega::MegaTransfer* transfer,mega::MegaError*);

private:
    bool isRetried(mega::MegaTransfer* transfer);
    bool isRetriedFolder(mega::MegaTransfer* transfer);
    bool isCompletedFromFolderRetry(mega::MegaTransfer* transfer);
    bool isIgnored(mega::MegaTransfer* transfer, bool removeCache = false);
    void updateFailedTransfer(QExplicitlySharedDataPointer<TransferData> data, mega::MegaTransfer* transfer,
                              mega::MegaError* e);

    QExplicitlySharedDataPointer<TransferData> createData(mega::MegaTransfer* transfer, mega::MegaError *e);
    QExplicitlySharedDataPointer<TransferData> onTransferEvent(mega::MegaTransfer* transfer, mega::MegaError *e);
    QList<QExplicitlySharedDataPointer<TransferData>> extractFromCache(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, int spaceForTransfers);
    QExplicitlySharedDataPointer<TransferData> checkIfRepeatedAndRemove(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, mega::MegaTransfer *transfer);
    QExplicitlySharedDataPointer<TransferData> checkIfRepeatedAndSubstitute(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, mega::MegaTransfer *transfer);
    QExplicitlySharedDataPointer<TransferData> checkIfRepeatedAndSubstituteInStartTransfers(QMap<int, QExplicitlySharedDataPointer<TransferData> > &dataMap, mega::MegaTransfer *transfer);

    struct cacheTransfers
    {
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> updateTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> startTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> startSyncTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> canceledTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> failedFolderTransfersByTag;
        QMap<TransferTag, QExplicitlySharedDataPointer<TransferData>> failedTransfersByTag;

        void clear()
        {
            updateTransfersByTag.clear();
            startTransfersByTag.clear();
            startSyncTransfersByTag.clear();
            canceledTransfersByTag.clear();
            failedFolderTransfersByTag.clear();
            failedTransfersByTag.clear();
        }
    };

    cacheTransfers mTransfersToProcess;
    QMutex mCacheMutex;
    QMutex mCountersMutex;
    TransfersCount mTransfersCount;
    LastTransfersCount mLastTransfersCount;
    std::atomic<int16_t> mMaxTransfersToProcess;

    QList<int> mRetriedFolder;
    QList<int> mIgnoredFiles;
};

class TransfersModel : public QAbstractItemModel
{
    Q_OBJECT

public:
    explicit TransfersModel(QObject* parent = 0);
    ~TransfersModel();

    virtual Qt::ItemFlags flags(const QModelIndex& index) const override;
    virtual Qt::DropActions supportedDropActions() const override;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const override;
    bool dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                                int column, const QModelIndex& parent) override;
    bool hasChildren(const QModelIndex& parent) const override;
    int rowCount(const QModelIndex& parent = QModelIndex()) const override;
    int columnCount(const QModelIndex& parent = QModelIndex()) const override;
    QVariant data(const QModelIndex& index, int role) const override;
    QModelIndex parent(const QModelIndex& index) const override;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const override;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex()) override;
    bool moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                  const QModelIndex &destinationParent, int destinationChild) override;

    void ignoreMoveRowsSignal(bool state);
    void inverseMoveRowsSignal(bool state);
    bool moveTransferPriority(const QModelIndex& sourceParent, const QList<int>& rows,
                  const QModelIndex& destinationParent, int destinationChild);

    void resetModel();

    void getLinks(const QList<int>& rows);
    void openInMEGA(const QList<int>& rows);
    std::unique_ptr<mega::MegaNode> getNodeToOpenByRow(int row);
    std::unique_ptr<mega::MegaNode> getParentNodeToOpenByRow(int row);
    QFileInfo getFileInfoByIndex(const QModelIndex &index);
    void openFolderByIndex(const QModelIndex& index);
    void openFoldersByIndexes(const QModelIndexList& indexes);
    void openFolderByTag(TransferTag tag);

    void retryTransferByIndex(const QModelIndex& index);
    void retryTransfers(QModelIndexList indexes, unsigned long long suggestedUploadAppData = 0, unsigned long long suggestedDownloadAppData = 0);
    void retryTransfersByAppDataId(const std::shared_ptr<TransferMetaData> &data);

    void cancelAndClearTransfers(const QModelIndexList& indexes, QWidget *canceledFrom);
    void cancelAllTransfers(QWidget *canceledFrom);
    void clearAllTransfers();
    void clearTransfers(const QModelIndexList& indexes);
    void clearFailedTransfers(const QModelIndexList& indexes);
    void clearTransfers(const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>>& uploads,
                        const QMap<QModelIndex,QExplicitlySharedDataPointer<TransferData>>& downloads);
    void performClearTransfers(const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads);
    void classifyUploadOrDownloadCompletedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void classifyUploadOrDownloadFailedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &uploads,
                        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > &downloads,
                                           const QModelIndex &index);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);
    void pauseResumeTransferByIndex(const QModelIndex& index, bool pauseState);
    void globalPauseStateChanged(bool state);
    void setGlobalPause(bool state);

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

    const QExplicitlySharedDataPointer<const TransferData> getTransferByTag(int tag) const;
    QExplicitlySharedDataPointer<TransferData> getTransferByTag(int tag);

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
    void onUpdateTransfersFinished();
    void onAskForMostPriorityTransfersFinished();
    void onKeepPCAwake();

private:
    void removeRows(QModelIndexList &indexesToRemove);
    QExplicitlySharedDataPointer<TransferData> getTransfer(int row) const;
    void addTransfer(QExplicitlySharedDataPointer<TransferData>);
    void removeTransfer(int row);
    void sendDataChanged(int row);
    void restoreTagsByRow();

    void retryTransfers(const QMultiMap<unsigned long long, std::shared_ptr<mega::MegaTransfer>>& transfersToRetry);

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

    void openFolder(const QFileInfo& info);

    void updateMetaDataBeforeRetryingTransfers(std::shared_ptr<mega::MegaTransfer> transfer);

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
    QHash<int,QExplicitlySharedDataPointer<TransferData>> mFailedFoldersByTag;
    QHash<mega::MegaHandle,QPersistentModelIndex> mCompletedTransfersByTag;

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
    mutable QReadWriteLock  mDataMutex;
    QTimer mMostPriorityTransferTimer;

    bool mAreAllPaused;
    bool mHasActiveTransfers;
    QSet<TransferTag> mActiveTransfers;

    bool mIgnoreMoveSignal;
    bool mInverseMoveSignal;

    QSet<int> mRetriedFolderTags;
};

Q_DECLARE_METATYPE(QAbstractItemModel::LayoutChangeHint)
Q_DECLARE_METATYPE(QList<QPersistentModelIndex>)
Q_DECLARE_METATYPE(QVector<int>)
Q_DECLARE_METATYPE(QSet<int>)

#endif // TRANSFERSMODEL_H
