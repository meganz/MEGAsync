#include "TransfersModel.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Platform.h"
#include "TransferItem.h"
#include "QMegaMessageBox.h"
#include "EventUpdater.h"
#include "SettingsDialog.h"
#include "platform/PowerOptions.h"
#include "PlatformStrings.h"

#include <QSharedData>

#include <algorithm>

using namespace mega;

static const QModelIndex DEFAULT_IDX = QModelIndex();

const int MAX_TRANSFERS = 2000;
const int CANCEL_THRESHOLD_THREAD = 100;
const int START_THRESHOLD_THREAD = 50;
const int FAILED_THRESHOLD_THREAD = 100;
const int PAUSE_RESUME_THRESHOLD_THREAD = 300;
const int CLEAR_THRESHOLD_THREAD = 300;

//LISTENER THREAD
TransferThread::TransferThread() : mMaxTransfersToProcess(MAX_TRANSFERS)
{}

TransferThread::TransfersToProcess TransferThread::processTransfers()
{
   TransfersToProcess transfers;
   if(mCacheMutex.tryLock())
   {
       int spaceForTransfers(mMaxTransfersToProcess);

       transfers.canceledTransfersByTag = extractFromCache(mTransfersToProcess.canceledTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.canceledTransfersByTag.size();

       transfers.failedTransfersByTag = extractFromCache(mTransfersToProcess.failedTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.failedTransfersByTag.size();

       transfers.startTransfersByTag = extractFromCache(mTransfersToProcess.startTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.startTransfersByTag.size();

       transfers.startSyncTransfersByTag = extractFromCache(mTransfersToProcess.startSyncTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.startSyncTransfersByTag.size();

       transfers.updateTransfersByTag = extractFromCache(mTransfersToProcess.updateTransfersByTag, spaceForTransfers);

       mCacheMutex.unlock();
   }

   return transfers;
}

void TransferThread::clear()
{
    QMutexLocker lock(&mCacheMutex);

    mTransfersToProcess.clear();
    mTransfersCount.clear();
}

QList<QExplicitlySharedDataPointer<TransferData>> TransferThread::extractFromCache(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, int spaceForTransfers)
{
    if(!dataMap.isEmpty() && spaceForTransfers > 0)
    {
        if(dataMap.size() > spaceForTransfers)
        {
            QList<QExplicitlySharedDataPointer<TransferData>> auxList;
            for(auto index = 0; index < spaceForTransfers
                && !dataMap.isEmpty(); ++index)
            {
                auto& firstItem = dataMap.first();
                if(firstItem)
                {
                    auxList.append(firstItem);
                    dataMap.take(firstItem->mTag);
                }
            }

            return auxList;
        }
        else
        {
            auto auxList = dataMap.values();
            dataMap.clear();

            return auxList;
        }
    }

    return QList<QExplicitlySharedDataPointer<TransferData>>();
}

QExplicitlySharedDataPointer<TransferData> TransferThread::createData(MegaTransfer *transfer)
{
    QExplicitlySharedDataPointer<TransferData> d (new TransferData(transfer));

    if(transfer->getState() == MegaTransfer::STATE_FAILED)
    {
        d->mFailedTransfer = std::shared_ptr<mega::MegaTransfer>(transfer->copy());
    }

    return d;
}

bool TransferThread::checkIfRepeatedAndSubstituteInStartTransfers(MegaTransfer* transfer)
{
    auto result(false);

    if(mTransfersToProcess.startTransfersByTag.contains(transfer->getTag()))
    {
        auto item = mTransfersToProcess.startTransfersByTag.value(transfer->getTag());
        if(transfer->getState() == mega::MegaTransfer::STATE_CANCELLED)
        {
            mTransfersToProcess.startTransfersByTag.remove(transfer->getTag());
        }
        else if(item->mNotificationNumber < transfer->getNotificationNumber())
        {
            mTransfersToProcess.startTransfersByTag[transfer->getTag()] = createData(transfer);
        }

        result = true;
    }

    return result;
}

bool TransferThread::checkIfRepeatedAndSubstitute(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, MegaTransfer* transfer)
{
    auto result(false);

    if(dataMap.contains(transfer->getTag()))
    {
        auto item = dataMap.value(transfer->getTag());
        if(item->mNotificationNumber < transfer->getNotificationNumber())
        {
            dataMap[transfer->getTag()] = createData(transfer);
        }

        result = true;
    }

    return result;
}

bool TransferThread::checkIfRepeatedAndRemove(QMap<int, QExplicitlySharedDataPointer<TransferData>>& dataMap, MegaTransfer* transfer)
{
    if(dataMap.contains(transfer->getTag()))
    {
        auto item = dataMap.value(transfer->getTag());
        if(item->mNotificationNumber < transfer->getNotificationNumber())
        {
            dataMap.remove(transfer->getTag());
        }
    }

    return false;
}

QExplicitlySharedDataPointer<TransferData> TransferThread::onTransferEvent(MegaTransfer *transfer)
{
    auto result = checkIfRepeatedAndSubstituteInStartTransfers(transfer);

    if(!result)
    {
        result = checkIfRepeatedAndSubstitute(mTransfersToProcess.startSyncTransfersByTag, transfer);
    }

    if(!result)
    {
        result = checkIfRepeatedAndSubstitute(mTransfersToProcess.canceledTransfersByTag, transfer);
    }

    if(!result)
    {
        result = checkIfRepeatedAndSubstitute(mTransfersToProcess.failedTransfersByTag, transfer);
    }

    if(!result)
    {
        result = checkIfRepeatedAndRemove(mTransfersToProcess.updateTransfersByTag, transfer);
    }

    if(!result)
    {
        return createData(transfer);
    }

    return QExplicitlySharedDataPointer<TransferData>();
}

void TransferThread::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        {
            QMutexLocker counterLock(&mCountersMutex);
            auto fileType = Utilities::getFileType(QString::fromStdString(transfer->getFileName()), QString());
            mTransfersCount.transfersByType[fileType]++;

            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.totalUploads++;
                mTransfersCount.pendingUploads++;
                mTransfersCount.totalUploadBytes += transfer->getTotalBytes();
                mTransfersCount.completedUploadBytes += transfer->getTransferredBytes();

                mLastTransfersCount.totalUploads++;
                mLastTransfersCount.pendingUploads++;
                mLastTransfersCount.totalUploadBytes += transfer->getTotalBytes();
                mLastTransfersCount.completedUploadBytes += transfer->getTransferredBytes();
            }
            else
            {
                mTransfersCount.totalDownloads++;
                mTransfersCount.pendingDownloads++;
                mTransfersCount.totalDownloadBytes += transfer->getTotalBytes();
                mTransfersCount.completedDownloadBytes += transfer->getTransferredBytes();

                mLastTransfersCount.totalDownloads++;
                mLastTransfersCount.pendingDownloads++;
                mLastTransfersCount.totalDownloadBytes += transfer->getTotalBytes();
                mLastTransfersCount.completedDownloadBytes += transfer->getTransferredBytes();
            }
        }

        {
            QMutexLocker cacheLock(&mCacheMutex);
            auto data = onTransferEvent(transfer);

            if(data)
            {
                if(transfer->isSyncTransfer())
                {
                    mTransfersToProcess.startSyncTransfersByTag.insert(transfer->getTag(), data);
                }
                else
                {
                    mTransfersToProcess.startTransfersByTag.insert(transfer->getTag(), data);
                }
            }
        }
    }
}

void TransferThread::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        {
            QMutexLocker counterLock(&mCountersMutex);
            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.completedUploadBytes += transfer->getDeltaSize();

                mLastTransfersCount.completedUploadBytes += transfer->getDeltaSize();
            }
            else
            {
                mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                mLastTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
            }
        }

        {
            QMutexLocker cacheLock(&mCacheMutex);
            auto data = onTransferEvent(transfer);

            if(data)
            {
                mTransfersToProcess.updateTransfersByTag.insert(transfer->getTag(), data);
            }
        }
    }
}

void TransferThread::onTransferFinish(MegaApi*, MegaTransfer *transfer, MegaError*)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        {
            QMutexLocker counterLock(&mCountersMutex);
            auto fileType = Utilities::getFileType(QString::fromStdString(transfer->getFileName()), QString());
            if(transfer->getState() == MegaTransfer::STATE_CANCELLED || (transfer->getState() == MegaTransfer::STATE_FAILED
                                                                         && transfer->isSyncTransfer()))
            {
                mTransfersCount.transfersByType[fileType]--;

                if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
                {
                    mTransfersCount.completedUploadBytes -= transfer->getTransferredBytes();
                    mTransfersCount.totalUploadBytes -= transfer->getTotalBytes();
                    mTransfersCount.pendingUploads--;
                    mTransfersCount.totalUploads--;

                    mLastTransfersCount.completedUploadBytes -= transfer->getTransferredBytes();
                    mLastTransfersCount.totalUploadBytes -= transfer->getTotalBytes();
                    mLastTransfersCount.pendingUploads--;
                    mLastTransfersCount.totalUploads--;
                }
                else
                {
                    mTransfersCount.completedDownloadBytes -= transfer->getTransferredBytes();
                    mTransfersCount.totalDownloadBytes -= transfer->getTotalBytes();
                    mTransfersCount.pendingDownloads--;
                    mTransfersCount.totalDownloads--;

                    mLastTransfersCount.completedDownloadBytes -= transfer->getTransferredBytes();
                    mLastTransfersCount.totalDownloadBytes -= transfer->getTotalBytes();
                    mLastTransfersCount.pendingDownloads--;
                    mLastTransfersCount.totalDownloads--;
                }

                if(mTransfersCount.pendingTransfers() == 0)
                {
                    mLastTransfersCount.clear();
                }
            }
            else
            {
                mTransfersCount.transfersFinishedByType[fileType]++;
                if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
                {
                    mTransfersCount.pendingUploads--;
                    mLastTransfersCount.pendingUploads--;

                    if(transfer->getTransferredBytes() < transfer->getTotalBytes())
                    {
                        mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
                        mLastTransfersCount.completedUploadBytes += transfer->getDeltaSize();
                    }

                    if(transfer->getState() == MegaTransfer::STATE_FAILED && !transfer->isSyncTransfer())
                    {
                        mTransfersCount.failedUploads++;
                    }

                    mLastTransfersCount.completedUploadsByTag.insert(transfer->getTag());
                }
                else
                {
                    mTransfersCount.pendingDownloads--;
                    mLastTransfersCount.pendingDownloads--;

                    if(transfer->getTransferredBytes() < transfer->getTotalBytes())
                    {
                        mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                        mLastTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                    }

                    if(transfer->getState() == MegaTransfer::STATE_FAILED && !transfer->isSyncTransfer())
                    {
                        mTransfersCount.failedDownloads++;
                    }

                    mLastTransfersCount.completedDownloadsByTag.insert(transfer->getTag());
                }

                if(mTransfersCount.pendingTransfers() == 0)
                {
                    mLastTransfersCount.clear();
                }
            }
        }

        {
            QMutexLocker cacheLock(&mCacheMutex);
            auto data = onTransferEvent(transfer);
            if(data)
            {
                if(transfer->getState() == MegaTransfer::STATE_CANCELLED)
                {
                    mTransfersToProcess.canceledTransfersByTag.insert(transfer->getTag(), data);
                }
                else if(transfer->getState() == MegaTransfer::STATE_FAILED)
                {
                    mTransfersToProcess.failedTransfersByTag.insert(transfer->getTag(), data);
                }
                else
                {
                    mTransfersToProcess.updateTransfersByTag.insert(transfer->getTag(), data);
                }
            }
        }
    }

}

void TransferThread::onTransferTemporaryError(MegaApi*, MegaTransfer *transfer, MegaError *)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        {
            QMutexLocker counterLock(&mCountersMutex);
            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
                mLastTransfersCount.completedUploadBytes += transfer->getDeltaSize();
            }
            else
            {
                mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                mLastTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
            }
        }

        {
            QMutexLocker cacheLock(&mCacheMutex);
            auto data = onTransferEvent(transfer);
            if(data)
            {
                mTransfersToProcess.updateTransfersByTag.insert(transfer->getTag(), data);
                data->mTemporaryError = true;
            }
        }
    }
}

TransfersCount TransferThread::getTransfersCount()
{
    QMutexLocker lock(&mCountersMutex);
    return mTransfersCount;
}

LastTransfersCount TransferThread::getLastTransfersCount()
{
    QMutexLocker lock(&mCountersMutex);
    return mLastTransfersCount;
}

int TransfersModel::hasActiveTransfers() const
{
    return mActiveTransfers.size();
}

void TransfersModel::setActiveTransfer(TransferTag tag)
{
    auto isEmpty = mActiveTransfers.isEmpty();
    mActiveTransfers.insert(tag);
    if(isEmpty)
    {
        emit activeTransfersChanged();
    }
}

void TransfersModel::unsetActiveTransfer(TransferTag tag)
{
    auto removed = mActiveTransfers.remove(tag);
    if(removed && mActiveTransfers.isEmpty())
    {
        emit activeTransfersChanged();
    }
}

void TransfersModel::checkActiveTransfer(TransferTag tag, bool isActive)
{
   isActive ? setActiveTransfer(tag) : unsetActiveTransfer(tag);
}

void TransfersModel::uiUnblocked()
{
    showSyncCancelledWarning();
}

void TransferThread::resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset)
{
    QMutexLocker lock(&mCountersMutex);

    foreach(auto& transfer, transfersToReset)
    {
        if(mTransfersCount.totalUploads > 0)
        {
            mTransfersCount.totalUploads--;
            mTransfersCount.completedUploadBytes -= transfer->mTotalSize;
            mTransfersCount.totalUploadBytes -= transfer->mTotalSize;
            mTransfersCount.transfersByType[transfer->mFileType]--;
            mTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->isFailed() && !transfer->isSyncTransfer())
            {
                mTransfersCount.failedUploads--;
                transfer->removeFailedTransfer();
            }
        }

        if(mLastTransfersCount.completedUploadsByTag.contains(transfer->mTag))
        {
            mLastTransfersCount.completedUploadsByTag.remove(transfer->mTag);
            mLastTransfersCount.totalUploads--;
            mLastTransfersCount.completedUploadBytes -= transfer->mTotalSize;
            mLastTransfersCount.totalUploadBytes -= transfer->mTotalSize;
            mLastTransfersCount.transfersByType[transfer->mFileType]--;
            mLastTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->isFailed() && !transfer->isSyncTransfer())
            {
                mLastTransfersCount.failedUploads--;
            }
        }
    }
}

void TransferThread::resetCompletedDownloads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset)
{
    QMutexLocker lock(&mCountersMutex);

    foreach(auto& transfer, transfersToReset)
    {
        if(mTransfersCount.totalDownloads > 0)
        {
            mTransfersCount.totalDownloads--;
            mTransfersCount.completedDownloadBytes -= transfer->mTotalSize;
            mTransfersCount.totalDownloadBytes -= transfer->mTotalSize;
            mTransfersCount.transfersByType[transfer->mFileType]--;
            mTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->isFailed() && !transfer->isSyncTransfer())
            {
                mTransfersCount.failedDownloads--;
                transfer->removeFailedTransfer();
            }
        }

        if(mLastTransfersCount.completedDownloadsByTag.contains(transfer->mTag))
        {
            mLastTransfersCount.completedDownloadsByTag.remove(transfer->mTag);
            mLastTransfersCount.totalDownloads--;
            mLastTransfersCount.completedDownloadBytes -= transfer->mTotalSize;
            mLastTransfersCount.totalDownloadBytes -= transfer->mTotalSize;
            mLastTransfersCount.transfersByType[transfer->mFileType]--;
            mLastTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->isFailed() && !transfer->isSyncTransfer())
            {
                mLastTransfersCount.failedDownloads--;
            }
        }
    }
}

void TransferThread::setMaxTransfersToProcess(uint16_t max)
{
    mMaxTransfersToProcess = max;
}

///////////////// TRANSFERS MODEL //////////////////////////////////////////////

const int PROCESS_TIMER = 100;
const int RESET_AFTER_EMPTY_RECEIVES = 10;
const int MODEL_HAS_CHANGED_AFTER_EMPTY_RECEIVES = 5;

TransfersModel::TransfersModel(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mPreferences (Preferences::instance()),
    mTransfersProcessChanged(0),
    mUpdateMostPriorityTransfer(0),
    mUiBlockedCounter(0),
    mUiBlockedByCounter(0),
    mCancelledFrom(nullptr),
    mSyncsInRowsToCancel(false),
    mIgnoreMoveSignal(false),
    mInverseMoveSignal(false)
{
    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");
    qRegisterMetaType<QAbstractItemModel::LayoutChangeHint>("QAbstractItemModel::LayoutChangeHint");
    qRegisterMetaType<QVector<int>>("QVector<int>");
    qRegisterMetaType<QVector<int>>("QSet<int>");

    mAreAllPaused = mPreferences->getGlobalPaused();
    mMegaApi->pauseTransfers(mAreAllPaused);

    mTransferEventThread = new QThread();
    mTransferEventWorker = new TransferThread();
    mTransferEventWorker->moveToThread(mTransferEventThread);
    mDelegateListener = new QTMegaTransferListener(mMegaApi, mTransferEventWorker);
    mDelegateListener->moveToThread(mTransferEventThread);
    mMegaApi->addTransferListener(mDelegateListener);

    //Update transfers state for the first time
    updateTransfersCount();

    mProcessTransfersTimer.setInterval(PROCESS_TIMER);
    QObject::connect(&mProcessTransfersTimer, &QTimer::timeout, this, &TransfersModel::onProcessTransfers);
    mProcessTransfersTimer.start();

    mTransferEventThread->start();

    mMostPriorityTransferTimer.setInterval(100);
    mMostPriorityTransferTimer.setSingleShot(true);
    QObject::connect(&mMostPriorityTransferTimer, &QTimer::timeout, this, &TransfersModel::askForMostPriorityTransfer);

    connect(&mUpdateTransferWatcher, &QFutureWatcher<void>::finished, this, &TransfersModel::updateTransfersCount);
    connect(&mClearTransferWatcher, &QFutureWatcher<void>::finished, this, &TransfersModel::onClearTransfersFinished);

    connect(this, &TransfersModel::activeTransfersChanged, this, &TransfersModel::onKeepPCAwake);
}

TransfersModel::~TransfersModel()
{
    mActiveTransfers.clear();
    onKeepPCAwake();

    // Cleanup
    mTransfers.clear();

    mTransferEventThread->quit();
    mTransferEventThread->deleteLater();
    mTransferEventWorker->deleteLater();
    mMegaApi->removeTransferListener(mDelegateListener);
}

void TransfersModel::pauseModelProcessing(bool value)
{
    QMutexLocker lock(&mModelMutex);

    if(value)
    {
        mProcessTransfersTimer.stop();
    }
    else
    {
        mProcessTransfersTimer.start(PROCESS_TIMER);
    }
}

bool TransfersModel::areAllPaused() const
{
    return mAreAllPaused;
}

bool TransfersModel::hasChildren(const QModelIndex& parent) const
{
    if (parent == DEFAULT_IDX)
    {
        return !mTransfers.empty();
    }
    return false;
}

int TransfersModel::rowCount(const QModelIndex& parent) const
{
    int rowCount (0);
    if (parent == DEFAULT_IDX)
    {
        rowCount = mTransfers.size();
    }
    return rowCount;
}

int TransfersModel::columnCount(const QModelIndex& parent) const
{
    //The same number of columns as sort criterions are needed
    //However in the sort filter the column count WILL BE ALWAYS 1 (check columnCount on sort filter class)
    if (parent == DEFAULT_IDX)
    {
        return static_cast<int>(SortCriterion::LAST);
    }
    return 0;
}

QVariant TransfersModel::data(const QModelIndex& index, int role) const
{
    if (role == Qt::DisplayRole)
    {
        return QVariant::fromValue(TransferItem(getTransfer(index.row())));
    }

    return QVariant();
}

QModelIndex TransfersModel::parent(const QModelIndex&) const
{
    return DEFAULT_IDX;
}

QModelIndex TransfersModel::index(int row, int column, const QModelIndex&) const
{
    return (row < rowCount(DEFAULT_IDX)) ?  createIndex(row, column) : DEFAULT_IDX;
}

void TransfersModel::onProcessTransfers()
{
    if(mTransfersToProcess.isEmpty())
    {
        mTransfersToProcess = mTransferEventWorker->processTransfers();
    }

    if(!mTransfersToProcess.isEmpty())
    {
        mostPriorityTransferMayChanged(true);

        int containsTransfersToStart(mTransfersToProcess.startTransfersByTag.size());
        int containsSyncTransfersToStart(mTransfersToProcess.startSyncTransfersByTag.size());
        int containsTransfersToUpdate(mTransfersToProcess.updateTransfersByTag.size());
        int containsTransfersToCancel(mTransfersToProcess.canceledTransfersByTag.size());
        int containsTransfersFailed(mTransfersToProcess.failedTransfersByTag.size());

        if(containsTransfersToCancel > 0)
        {            
            cacheCancelTransfersTags();

            if(isUiBlockedModeActive() || containsTransfersToCancel > CANCEL_THRESHOLD_THREAD)
            {
                setUiBlockedMode(true);
            }
            else if(!isUiBlockedModeActive())
            {
                if(mModelMutex.tryLock())
                {
                    processCancelTransfers();
                    showSyncCancelledWarning();
                    mModelMutex.unlock();
                }
            }
        }

        if(containsTransfersFailed > 0)
        {
            if(isUiBlockedModeActive() || containsTransfersFailed > FAILED_THRESHOLD_THREAD)
            {
                setUiBlockedMode(true);

                auto future = QtConcurrent::run([this](){
                    if(mModelMutex.tryLock())
                    {
                        blockModelSignals(true);
                        processFailedTransfers();
                        blockModelSignals(false);

                        mModelMutex.unlock();
                    }

                });
                mUpdateTransferWatcher.setFuture(future);
            }
            else
            {
                if(mModelMutex.tryLock())
                {
                    processFailedTransfers();
                    mModelMutex.unlock();
                }

                processSyncFailedTransfers();
            }
        }
        else
        {
            if(containsTransfersToStart > 0 || containsSyncTransfersToStart > 0)
            {
                if(isUiBlockedModeActive() || containsTransfersToStart > START_THRESHOLD_THREAD)
                {
                    setUiBlockedMode(true);
                }

                if(mModelMutex.tryLock())
                {
                    if(isUiBlockedModeActive())
                    {
                        blockModelSignals(true);
                    }

                    processStartTransfers(mTransfersToProcess.startTransfersByTag);
                    processStartTransfers(mTransfersToProcess.startSyncTransfersByTag);

                    if(isUiBlockedModeActive())
                    {
                        blockModelSignals(false);
                    }

                    mModelMutex.unlock();
                }
            }
            else if(containsTransfersToUpdate > 0)
            {
                if(isUiBlockedByCounter())
                {
                    auto future = QtConcurrent::run([this, containsTransfersToUpdate](){
                        if(mModelMutex.tryLock())
                        {
                            blockModelSignals(true);
                            processUpdateTransfers();
                            blockModelSignals(false);

                            updateUiBlockedByCounter(containsTransfersToUpdate);
                            mModelMutex.unlock();
                        }
                    });
                    mUpdateTransferWatcher.setFuture(future);
                }
                else
                {
                    if(mModelMutex.tryLock())
                    {
                        processUpdateTransfers();
                        updateUiBlockedByCounter(containsTransfersToUpdate);
                        mModelMutex.unlock();
                    }
                }
            }
            else
            {
                if(isUiBlockedByCounter())
                {
                    setUiBlockedByCounterMode(false);
                }
            }
        }

        updateTransfersCount();

        if(isUiBlockedModeActive())
        {
            setUiBlockedMode(false);
        }

        modelHasChanged(true);
    }
    else
    {
        modelHasChanged(false);

        if(isUiBlockedModeActive())
        {
            setUiBlockedMode(false);
        }
        else if(isUiBlockedByCounter())
        {
            setUiBlockedByCounterMode(false);
        }
        else
        {
            mostPriorityTransferMayChanged(false);
        }
    }
}

void TransfersModel::processStartTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersToStart)
{
    if (!transfersToStart.isEmpty())
    {
        auto totalRows = rowCount(DEFAULT_IDX);

        // Remove repetead transfers
        QMutableListIterator<QExplicitlySharedDataPointer<TransferData>> checkIt(transfersToStart);
        while (checkIt.hasNext())
        {
            if (mTagByOrder.contains(checkIt.next()->mTag))
            {
                checkIt.remove();
            }
        }

        if(!transfersToStart.isEmpty())
        {
            auto rowsToBeInserted(static_cast<int>(transfersToStart.size()));

            beginInsertRows(DEFAULT_IDX, totalRows, totalRows + rowsToBeInserted - 1);

            for (auto it = transfersToStart.begin(); it != transfersToStart.end();)
            {
                startTransfer((*it));
                transfersToStart.erase(it++);
            }

            endInsertRows();
        }
    }
}

void TransfersModel::startTransfer(QExplicitlySharedDataPointer<TransferData> transfer)
{
    addTransfer(transfer);

    auto state (transfer->getState());

    if (mAreAllPaused && (state & TransferData::PAUSABLE_STATES_MASK))
    {
        mMegaApi->pauseTransferByTag(transfer->mTag, true);
        transfer->setState(TransferData::TRANSFER_PAUSED);

        //Otherwise when filtering there will be wrong result
        transfer->setPreviousState(TransferData::TRANSFER_NONE);
    }
}

void TransfersModel::updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer, int row)
{
    checkActiveTransfer(transfer->mTag, transfer->isActive());

    mTransfers[row] = transfer;
}

void TransfersModel::processUpdateTransfers()
{
    for (auto it = mTransfersToProcess.updateTransfersByTag.begin(); it != mTransfersToProcess.updateTransfersByTag.end();)
    {   
        auto row(getRowByTransferTag((*it)->mTag));
        auto d  = getTransfer(row);
        if(d && !d->ignoreUpdate((*it)->getState()))
        {
            (*it)->setPreviousState(d->getState());
            updateTransfer((*it), row);
            sendDataChanged(row);
            (*it)->resetStateHasChanged();
        }

        mTransfersToProcess.updateTransfersByTag.erase(it++);
    }
}

void TransfersModel::processFailedTransfers()
{
    for (auto it = mTransfersToProcess.failedTransfersByTag.begin(); it != mTransfersToProcess.failedTransfersByTag.end();)
    {
        TransferTag tag ((*it)->mTag);

        auto row(getRowByTransferTag((*it)->mTag));
        auto d  = getTransfer(row);
        if(d)
        {
            (*it)->setPreviousState(d->getState());
            updateTransfer((*it), row);

            if(d->isSyncTransfer())
            {
                mFailedTransferToClear.append(tag);
            }
            else
            {
                sendDataChanged(row);
            }

            (*it)->resetStateHasChanged();
        }

        mTransfersToProcess.failedTransfersByTag.erase(it++);
    }
}

void TransfersModel::cacheCancelTransfersTags()
{
    for (auto it = mTransfersToProcess.canceledTransfersByTag.begin(); it != mTransfersToProcess.canceledTransfersByTag.end();)
    {
        mRowsToCancel.append((*it)->mTag);

        mTransfersToProcess.canceledTransfersByTag.erase(it++);
    }
}

void TransfersModel::processCancelTransfers()
{
    if(mRowsToCancel.size() > 0)
    {
        QModelIndexList indexesToCancel;

        foreach(auto tag, mRowsToCancel)
        {
            auto row = getRowByTransferTag(tag);
            if(row >= 0)
            {
                indexesToCancel.append(index(row,0, DEFAULT_IDX));
            }

            auto transfer = getTransferByTag(tag);
            checkActiveTransfer(tag, false);
        }

        mRowsToCancel.clear();

        removeRows(indexesToCancel);
    }
}

void TransfersModel::processSyncFailedTransfers()
{
    if(mFailedTransferToClear.size() > 0)
    {
        QModelIndexList indexesToClear;

        foreach(auto tag, mFailedTransferToClear)
        {
            auto row = getRowByTransferTag(tag);
            if(row >= 0)
            {
                indexesToClear.append(index(row,0, DEFAULT_IDX));
            }
        }

        mFailedTransferToClear.clear();

        removeRows(indexesToClear);
    }
}

void TransfersModel::getLinks(QList<int>& rows)
{
    if (!rows.isEmpty())
    {
        QList<MegaHandle> exportList;
        QStringList linkList;

        QMutexLocker lock(&mModelMutex);

        for (auto row : rows)
        {
            auto d (getTransfer(row));

            MegaNode *node (nullptr);

            if (d->getState() == TransferData::TRANSFER_FAILED)
            {
                auto transfer = mMegaApi->getTransferByTag(d->mTag);
                if(transfer)
                {
                    node = transfer->getPublicMegaNode();
                }
            }
            else if(d->mNodeHandle)
            {
                node = ((MegaApplication*)qApp)->getMegaApi()->getNodeByHandle(d->mNodeHandle);
            }

            if (!node || !node->isPublic())
            {
                exportList.push_back(d->mNodeHandle);
            }
            else if (node)
            {
                char *handle = node->getBase64Handle();
                char *key = node->getBase64Key();
                if (handle && key)
                {
                    QString link = Preferences::BASE_URL + QString::fromUtf8("/#!%1!%2")
                            .arg(QString::fromUtf8(handle), QString::fromUtf8(key));
                    linkList.push_back(link);
                }
                delete [] key;
                delete [] handle;
                delete node;
            }
        }
        if (exportList.size() || linkList.size())
        {
            qobject_cast<MegaApplication*>(qApp)->exportNodes(exportList, linkList);
        }
    }
}

void TransfersModel::openInMEGA(QList<int> &rows)
{
    if (!rows.isEmpty())
    {
        QMutexLocker lock(&mModelMutex);

        for (auto row : rows)
        {
            auto d (getTransfer(row));

            MegaNode *node (nullptr);

            if (d->getState() == TransferData::TRANSFER_FAILED)
            {
                auto transfer = mMegaApi->getTransferByTag(d->mTag);
                if(transfer)
                {
                    node = transfer->getPublicMegaNode();
                }
            }
            else if(d->mNodeHandle)
            {
                node = ((MegaApplication*)qApp)->getMegaApi()->getNodeByHandle(d->mNodeHandle);
            }

            if (node)
            {
                char *handle = node->getBase64Handle();
                char *key = node->getBase64Key();
                if (handle && key)
                {
                    QString url = QString::fromUtf8("mega://#fm/") + QString::fromUtf8(handle);
                    QtConcurrent::run(QDesktopServices::openUrl, QUrl(url));
                }
                delete [] key;
                delete [] handle;
                delete node;
            }
        }
    }
}

void TransfersModel::openFolderByIndex(const QModelIndex& index)
{
    QMutexLocker lock(&mModelMutex);

    const auto transferItem (
                qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
    auto d (transferItem.getTransferData());
    auto path = d->path();
    if (d && !path.isEmpty())
    {
        QFileInfo fileInfo(path);
        if(fileInfo.exists())
        {
            QtConcurrent::run([this, path]
            {
                emit showInFolderFinished(Platform::showInFolder(path));
            });
        }
        else
        {
            emit showInFolderFinished(false);
        }
    }
}

void TransfersModel::retryTransferByIndex(const QModelIndex& index)
{
    mModelMutex.lock();

    const auto transferItem (
                qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
    auto d (transferItem.getTransferData());

    if(d && d->mFailedTransfer)
    {
        auto failedTransferCopy = d->mFailedTransfer->copy();
        mModelMutex.unlock();

        QModelIndexList indexToRemove;
        indexToRemove.append(index);
        clearFailedTransfers(indexToRemove);

        QtConcurrent::run([failedTransferCopy, this](){
            mMegaApi->retryTransfer(failedTransferCopy);
            delete failedTransferCopy;
        });

    }
    else
    {
        mModelMutex.unlock();
    }
}

void TransfersModel::retryTransfers(QModelIndexList indexes)
{
    if(indexes.size() > FAILED_THRESHOLD_THREAD)
    {
        setUiBlockedMode(true);
    }

    std::sort(indexes.begin(), indexes.end(),[](QModelIndex index1, QModelIndex index2){
        return index1.row() > index2.row();
    });

    //Try to add more threads to speed up
    auto threadsToUse(1);

    auto availableThreads = QThreadPool::globalInstance()->maxThreadCount() - QThreadPool::globalInstance()->activeThreadCount();
    if(availableThreads > 2)
    {
        threadsToUse = availableThreads / 2;
    }

    QList<mega::MegaTransfer*> transfersToRetry;
    auto indexPerThread = (indexes.size() >= threadsToUse) ? (indexes.size() / threadsToUse) : indexes.size();

    mModelMutex.lock();

    auto counter = 0;
    foreach(auto index, indexes)
    {
        const auto transferItem (
                    qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
        auto d (transferItem.getTransferData());

        if(d && d->mFailedTransfer)
        {
            transfersToRetry.append(d->mFailedTransfer->copy());
        }

        if(counter % indexPerThread == 0)
        {
            QtConcurrent::run([transfersToRetry, this](){
                foreach(auto& failedTransferCopy, transfersToRetry)
                {
                    mMegaApi->retryTransfer(failedTransferCopy);
                    delete failedTransferCopy;
                }
            });

            transfersToRetry.clear();
        }
        counter++;
    }

    mModelMutex.unlock();

    QtConcurrent::run([transfersToRetry, this](){
        foreach(auto& failedTransferCopy, transfersToRetry)
        {
            mMegaApi->retryTransfer(failedTransferCopy);
            delete failedTransferCopy;
        }
    });

    clearFailedTransfers(indexes);
}

void TransfersModel::openFolderByTag(TransferTag tag)
{
    auto indexToOpen = index(getRowByTransferTag(tag), 0);
    if(indexToOpen.isValid())
    {
        openFolderByIndex(indexToOpen);
    }
}

long long TransfersModel::failedTransfers()
{
    return mTransfersCount.totalFailedTransfers();
}

void TransfersModel::cancelAllTransfers(QWidget* canceledFrom)
{
    auto count = rowCount(DEFAULT_IDX);

    mModelMutex.lock();
    for (auto row = 0; row < count;++row)
    {
        auto d (getTransfer(row));

        // Clear (remove rows of) finished transfers
        if (d && d->isSyncTransfer())
        {
            if(!mSyncsInRowsToCancel)
            {
                mSyncsInRowsToCancel = true;
                mCancelledFrom = canceledFrom;
                break;
            }
        }
    }
    mModelMutex.unlock();

    //Cancel little by little??? CAnceling everythin blocks the SDK
    mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}

void TransfersModel::cancelAndClearTransfers(const QModelIndexList& indexes, QWidget* canceledFrom)
{
    if(indexes.isEmpty())
    {
        return;
    }

    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

    QList<TransferTag> toCancel;

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    mModelMutex.lock();
    for (auto index : indexes)
    {
        auto d (getTransfer(index.row()));

        // Clear (remove rows of) finished transfers
        if (d)
        {
            if(!d->isCancelable())
            {
                if(d->isFinished())
                {
                    if(d->isFailed())
                    {
                        classifyUploadOrDownloadFailedTransfers(uploadToClear, downloadToClear,index);
                    }
                    else
                    {
                        classifyUploadOrDownloadCompletedTransfers(uploadToClear, downloadToClear,index);
                    }
                }
                else
                {
                    if(d->isSyncTransfer())
                    {
                        mSyncsInRowsToCancel = true;
                        mCancelledFrom = canceledFrom;
                    }
                }
            }
            else
            {
                toCancel.append(d->mTag);
            }
        }
    }
    mModelMutex.unlock();

    if(!uploadToClear.isEmpty() || !downloadToClear.isEmpty())
    {
        clearTransfers(uploadToClear, downloadToClear);
    }

    if(!toCancel.isEmpty())
    {
        auto counter(0);
        // Now cancel transfers
        for (auto item : toCancel)
        {
            mMegaApi->cancelTransferByTag(item);

            //This is done to avoid GUI freezes
            if(++counter == 100)
            {
                counter = 0;
                MegaSyncApp->processEvents();
            }
        }
    }
}

void TransfersModel::showSyncCancelledWarning()
{
    if(syncsInRowsToCancel())
    {
        QPointer<QMessageBox> removeSync = new QMessageBox(QMessageBox::Warning, QLatin1Literal("MEGAsync"),
                                                          PlatformStrings::cancelSyncsWarning(),
                                                           QMessageBox::No | QMessageBox::Yes, mCancelledFrom);
        removeSync->setButtonText(QMessageBox::No, tr("Dismiss"));
        removeSync->setButtonText(QMessageBox::Yes, PlatformStrings::openSettings());

        auto result = removeSync->exec();

        if(result == QMessageBox::Yes)
        {
            MegaSyncApp->openSettings(SettingsDialog::SYNCS_TAB);
        }

        resetSyncInRowsToCancel();
    }
}

bool TransfersModel::syncsInRowsToCancel() const
{
    return mSyncsInRowsToCancel;
}

void TransfersModel::resetSyncInRowsToCancel()
{
    mSyncsInRowsToCancel = false;
    mCancelledFrom = nullptr;
}

void TransfersModel::classifyUploadOrDownloadCompletedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& uploads,
                                                       QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& downloads,
                                                       const QModelIndex& index)
{
    auto d (getTransfer(index.row()));

    // Clear (remove rows of) finished transfers
    if (d && d->isCompleted())
    {
        if(d->isUpload())
        {
            uploads.insert(index, d);
        }
        else
        {
            downloads.insert(index, d);
        }
    }
}

void TransfersModel::classifyUploadOrDownloadFailedTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& uploads,
                                                       QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& downloads,
                                                             const QModelIndex& index)
{
    auto d (getTransfer(index.row()));

    // Clear (remove rows of) finished transfers
    if (d && d->isFailed())
    {
        if(d->isUpload())
        {
            uploads.insert(index, d);
        }
        else
        {
            downloads.insert(index, d);
        }
    }
}

void TransfersModel::clearAllTransfers()
{
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

    auto totalRows(rowCount(DEFAULT_IDX));

    EventUpdater updater(totalRows, 500);

    for (auto row = 0; row < totalRows; ++row)
    {
        auto indexToCheck = index(row, 0);
        classifyUploadOrDownloadCompletedTransfers(uploadToClear, downloadToClear,indexToCheck);

        updater.update(row);
    }

    clearTransfers(uploadToClear, downloadToClear);
}

void TransfersModel::clearTransfers(const QModelIndexList& indexes)
{   
    if(indexes.isEmpty())
    {
        return;
    }

    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

    for (auto indexToCheck : indexes)
    {
        classifyUploadOrDownloadCompletedTransfers(uploadToClear, downloadToClear,indexToCheck);
    }

    clearTransfers(uploadToClear, downloadToClear);
}

void TransfersModel::clearFailedTransfers(const QModelIndexList &indexes)
{
    if(indexes.isEmpty())
    {
        return;
    }

    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

    for (auto indexToCheck : indexes)
    {
        classifyUploadOrDownloadFailedTransfers(uploadToClear, downloadToClear,indexToCheck);
    }

    clearTransfers(uploadToClear, downloadToClear);
}

void TransfersModel::clearTransfers(const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > uploads,
                                    const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > downloads)
{
    if(!uploads.isEmpty() || !downloads.isEmpty())
    {
        auto totalTransfersToClear(uploads.size() + downloads.size());
        if(totalTransfersToClear > CLEAR_THRESHOLD_THREAD)
        {
            auto future = QtConcurrent::run([this, uploads, downloads]()
            {
                emit blockUi();

                blockModelSignals(true);
                performClearTransfers(uploads, downloads);
                blockModelSignals(false);
            });
            mClearTransferWatcher.setFuture(future);
        }
        else
        {
            performClearTransfers(uploads, downloads);
            updateTransfersCount();

            //The clear transfer is the only action which does not receive a SDK request
            emit transfersProcessChanged();
        }
    }
}

void TransfersModel::onClearTransfersFinished()
{
    updateTransfersCount();

    //The clear transfer is the only action which does not receive a SDK request
    emit transfersProcessChanged();
    emit unblockUiAndFilter();
}

void TransfersModel::onKeepPCAwake()
{
    PowerOptions options;
    options.keepAwake(hasActiveTransfers());
}

void TransfersModel::performClearTransfers(const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > uploads,
                                           const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > downloads)
{
    QModelIndexList itemsToRemove;

    if(!uploads.isEmpty())
    {
        mTransferEventWorker->resetCompletedUploads(uploads.values());

        itemsToRemove.append(uploads.keys());
    }

    if(!downloads.isEmpty())
    {
        mTransferEventWorker->resetCompletedDownloads(downloads.values());

        itemsToRemove.append(downloads.keys());
    }

    //About to remove transfers, be careful with the other threads
    mModelMutex.lock();
    removeRows(itemsToRemove);
    mModelMutex.unlock();

    //The clear transfer is the only action which does not receive a SDK request
    emit transfersProcessChanged();
}

void TransfersModel::pauseTransfers(const QModelIndexList& indexes, bool pauseState)
{
    if(!indexes.isEmpty())
    {
        setUiBlockedModeByCounter(indexes.size());

        if(indexes.size() > PAUSE_RESUME_THRESHOLD_THREAD)
        {
            QtConcurrent::run([this, indexes, pauseState]()
            {
                blockModelSignals(true);
                performPauseResumeVisibleTransfers(indexes, pauseState, false);
                blockModelSignals(false);

                emit pauseStateChanged(mAreAllPaused);
            });
        }
        else
        {
            performPauseResumeVisibleTransfers(indexes, pauseState, true);
        }
    }
}

int TransfersModel::performPauseResumeVisibleTransfers(const QModelIndexList& indexes, bool pauseState, bool useEventUpdater)
{
    EventUpdater updater(indexes.size(), 1000);
    auto tagsUpdated(0);

    QMutexLocker lock(&mModelMutex);

    for (auto index : indexes)
    {
        auto d = getTransfer(index.row());
        if((pauseState && d->getState() & TransferData::PAUSABLE_STATES_MASK)
                || (!pauseState && d->getState() & TransferData::TRANSFER_PAUSED))
        {
            pauseResumeTransferByTag(d->mTag, pauseState);
        }

        tagsUpdated++;

        if(useEventUpdater)
        {
            updater.update(tagsUpdated);
        }
    }

    return tagsUpdated;
}

void TransfersModel::blockModelSignals(bool state)
{
    blockSignals(state);
}

void TransfersModel::pauseResumeAllTransfers(bool state)
{
    mAreAllPaused = state;
    auto activeTransfers = rowCount();

    //Provisional active transfers, just used to know if UI will be blocked
    //The final count can be +- 30 transfers
    if(activeTransfers > PAUSE_RESUME_THRESHOLD_THREAD)
    {
        QtConcurrent::run([this, activeTransfers]()
        {
            blockModelSignals(true);
            auto tagsUpdated = performPauseResumeAllTransfers(activeTransfers, false);
            blockModelSignals(false);

            setUiBlockedModeByCounter(tagsUpdated);
            emit pauseStateChanged(mAreAllPaused);
        });
    }
    else
    {
        auto tagsUpdated = performPauseResumeAllTransfers(activeTransfers, true);
        setUiBlockedModeByCounter(tagsUpdated);

        emit pauseStateChanged(mAreAllPaused);
    }
}

int TransfersModel::performPauseResumeAllTransfers(int activeTransfers, bool useEventUpdater)
{
    auto tagsUpdated(0);

    QMutexLocker lock(&mModelMutex);

    if (mAreAllPaused)
    {
        mMegaApi->pauseTransfers(mAreAllPaused);

        EventUpdater updater(activeTransfers, 200);
        std::for_each(mTransfers.crbegin(), mTransfers.crend(), [this, &tagsUpdated, updater, useEventUpdater](QExplicitlySharedDataPointer<TransferData> item)
                      mutable {

            if(item->getState() & TransferData::PAUSABLE_STATES_MASK)
            {
                pauseResumeTransferByTag(item->mTag, mAreAllPaused);
                tagsUpdated++;
            }

            if(useEventUpdater)
            {
                updater.update(tagsUpdated);
            }
        });
    }
    else
    {
        EventUpdater updater(activeTransfers, 200);
        std::for_each(mTransfers.cbegin(), mTransfers.cend(), [this, &tagsUpdated, updater, useEventUpdater](QExplicitlySharedDataPointer<TransferData> item)
                      mutable {

            if(item->getState() & TransferData::TRANSFER_PAUSED)
            {
                pauseResumeTransferByTag(item->mTag, mAreAllPaused);
                tagsUpdated++;
            }

            if(useEventUpdater)
            {
                updater.update(tagsUpdated);
            }
        });

        mMegaApi->pauseTransfers(mAreAllPaused);

    }

    return tagsUpdated;
}

void TransfersModel::pauseResumeTransferByTag(TransferTag tag, bool pauseState)
{
    auto row(getRowByTransferTag(tag));
    auto d  = getTransfer(row);

    if(d)
    {
        if(!pauseState && mAreAllPaused)
        {
            mMegaApi->pauseTransfers(pauseState);
            mAreAllPaused = false;
            emit pauseStateChangedByTransferResume();
        }

        if(pauseState)
        {
            if(d->getState() & TransferData::PAUSABLE_STATES_MASK)
            {
                d->setPauseResume(true);
            }
        }
        else
        {
            d->setPauseResume(false);
        }

        sendDataChanged(row);
        d->resetStateHasChanged();
        mMegaApi->pauseTransferByTag(d->mTag, pauseState);
    }
}

void TransfersModel::pauseResumeTransferByIndex(const QModelIndex &index, bool pauseState)
{
    QMutexLocker lock(&mModelMutex);

    const auto transferItem (
                qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));

    pauseResumeTransferByTag(transferItem.getTransferData()->mTag, pauseState);
}

void TransfersModel::lockModelMutex(bool lock)
{
    if (lock)
    {
        mModelMutex.lock();
    }
    else
    {
        mModelMutex.unlock();
    }
}

long long TransfersModel::getNumberOfTransfersForFileType(Utilities::FileType fileType) const
{
    return mTransfersCount.transfersByType.value(fileType);
}

long long TransfersModel::getNumberOfFinishedForFileType(Utilities::FileType fileType) const
{
    return mTransfersCount.transfersFinishedByType.value(fileType);
}

TransfersCount TransfersModel::getTransfersCount()
{
    return mTransfersCount;
}

TransfersCount TransfersModel::getLastTransfersCount()
{
    return mLastTransfersCount;
}

void TransfersModel::updateTransfersCount()
{    
    mTransfersCount = mTransferEventWorker->getTransfersCount();
    mLastTransfersCount = mTransferEventWorker->getLastTransfersCount();

    emit transfersCountUpdated();
}

void TransfersModel::removeRows(QModelIndexList& indexesToRemove)
{
    if(indexesToRemove.isEmpty())
    {
        return;
    }

    std::sort(indexesToRemove.begin(), indexesToRemove.end(),[](QModelIndex check1, QModelIndex check2){
        return check1.row() > check2.row();
    });

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    //We add 1 to row, as the row will be reduced by one as soon as the loop starts
    int row (indexesToRemove.last().row() + 1);
    for (auto index : indexesToRemove)
    {
        row--;

        // Init row with row of first tag
        if (count == 0)
        {
            row = index.row();
        }

        // If rows are non-contiguous, flush and start from item
        if (row != index.row())
        {
            removeRows(row + 1, count, DEFAULT_IDX);

            count = 0;
            row = index.row();
        }

        // We have at least one row
        count++;
    }
    // Flush pooled rows (start at row).
    // This happens when the last item processed is in a finished state.
    if (count > 0 && row >= 0)
    {
        removeRows(row, count, DEFAULT_IDX);
    }
}

QExplicitlySharedDataPointer<TransferData> TransfersModel::getTransfer(int row) const
{
    if(row >= 0 && mTransfers.size() > row)
    {
        auto transfer = mTransfers.at(row);
        return transfer;
    }

    return QExplicitlySharedDataPointer<TransferData>();
}

void TransfersModel::addTransfer(QExplicitlySharedDataPointer<TransferData> transfer)
{
    mTransfers.append(transfer);
    mTagByOrder.insert(transfer->mTag, QPersistentModelIndex(index(rowCount(DEFAULT_IDX) - 1,0)));
}

QExplicitlySharedDataPointer<TransferData> TransfersModel::getTransferByTag(int tag) const
{
    return getTransfer(getRowByTransferTag(tag));
}

int TransfersModel::getRowByTransferTag(int tag) const
{
    return mTagByOrder.value(tag).row();
}

void TransfersModel::removeTransfer(int row)
{
    auto transfer = mTransfers.takeAt(row);
    mTagByOrder.remove(transfer->mTag);
}

void TransfersModel::sendDataChangedByTag(int tag)
{
    sendDataChanged(getRowByTransferTag(tag));
}

void TransfersModel::sendDataChanged(int row)
{
    if(!signalsBlocked())
    {
        QModelIndex indexChanged (index(row, 0, DEFAULT_IDX));
        emit dataChanged(indexChanged, indexChanged);
    }
}

bool TransfersModel::isUiBlockedModeActive() const
{
    return mUiBlockedCounter > 0;
}

void TransfersModel::setUiBlockedMode(bool state)
{
    if(state)
    {
        emit blockUi();

        mUiBlockedCounter = RESET_AFTER_EMPTY_RECEIVES;
    }
    else if(!state && mUiBlockedCounter != 0)
    {
        mUiBlockedCounter--;

        if(mUiBlockedCounter == 0)
        {
            updateTransfersCount();

            if(!mRowsToCancel.isEmpty() || !mFailedTransferToClear.isEmpty())
            {
                auto task = QtConcurrent::run([this]()
                {
                    if(mModelMutex.tryLock())
                    {
                        blockModelSignals(true);
                        processCancelTransfers();
                        processSyncFailedTransfers();
                        blockModelSignals(false);

                        mModelMutex.unlock();
                    }

                    emit unblockUiAndFilter();
                });
                mUpdateTransferWatcher.setFuture(task);
            }
            else
            {
                emit unblockUiAndFilter();
            }
        }
    }
}

void TransfersModel::setUiBlockedModeByCounter(uint32_t transferCount)
{
    if(transferCount > 0 && transferCount > PAUSE_RESUME_THRESHOLD_THREAD)
    {
        emit blockUi();
        setUiBlockedByCounterMode(true);
        mUiBlockedByCounter = transferCount;
        mTransferEventWorker->setMaxTransfersToProcess(20000);
    }
    else if(transferCount == 0)
    {
        emit unblockUi();
    }
}

void TransfersModel::updateUiBlockedByCounter(int updates)
{
    if(updates > 0 && mUiBlockedByCounter > 0)
    {
        if(mUiBlockedByCounter >= updates)
        {
            mUiBlockedByCounter -= updates;
        }
        else
        {
            mUiBlockedByCounter = 0;
        }

        if(mUiBlockedByCounter == 0)
        {
            mTransferEventWorker->setMaxTransfersToProcess(MAX_TRANSFERS);
            emit unblockUiAndFilter();
        }
    }
}

bool TransfersModel::isUiBlockedByCounter() const
{
    return mUiBlockedByCounter > 0;
}

void TransfersModel::setUiBlockedByCounterMode(bool state)
{
    if(state)
    {
        mUiBlockedByCounterSafety = RESET_AFTER_EMPTY_RECEIVES;
    }
    else if(!state && mUiBlockedByCounterSafety != 0)
    {
        mUiBlockedByCounterSafety--;

        if(mUiBlockedByCounterSafety == 0 && mUiBlockedByCounter > 0)
        {
            updateUiBlockedByCounter(mUiBlockedByCounter);
        }
    }
}

void TransfersModel::modelHasChanged(bool state)
{
    if(state)
    {
        if(mTransfersProcessChanged == 0)
        {
            emit transfersProcessChanged();
        }

        mTransfersProcessChanged = MODEL_HAS_CHANGED_AFTER_EMPTY_RECEIVES;
    }
    else
    {
        if(mTransfersProcessChanged > 0)
        {
            mTransfersProcessChanged--;

            if(mTransfersProcessChanged == 0)
            {
                emit transfersProcessChanged();
            }
        }
    }
}

void TransfersModel::mostPriorityTransferMayChanged(bool state)
{
    if(!state && mUpdateMostPriorityTransfer)
    {
        mUpdateMostPriorityTransfer--;
        if(mUpdateMostPriorityTransfer == 0)
        {
            mMostPriorityTransferTimer.stop();
            mMostPriorityTransferTimer.start();
        }
    }
    else if(state)
    {
        mUpdateMostPriorityTransfer = RESET_AFTER_EMPTY_RECEIVES;
        mMostPriorityTransferTimer.stop();
    }
}

void TransfersModel::askForMostPriorityTransfer()
{
    QtConcurrent::run([this]()
    {
        std::unique_ptr<MegaTransfer> nextUTransfer(MegaSyncApp->getMegaApi()->getFirstTransfer(MegaTransfer::TYPE_UPLOAD));
        auto UTag = nextUTransfer ? nextUTransfer->getTag() : -1;
        std::unique_ptr<MegaTransfer> nextDTransfer(MegaSyncApp->getMegaApi()->getFirstTransfer(MegaTransfer::TYPE_DOWNLOAD));
        auto DTag = nextDTransfer ? nextDTransfer->getTag() : -1;

        emit mostPriorityTransferUpdate(UTag, DTag);

    });
}

bool TransfersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent == DEFAULT_IDX && count > 0 && row >= 0)
    {
        beginRemoveRows(DEFAULT_IDX, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            removeTransfer(row);
        }

        endRemoveRows();

        return true;
    }
    else
    {
        return false;
    }
}

void TransfersModel::ignoreMoveRowsSignal(bool state)
{
    mIgnoreMoveSignal = state;
}

void TransfersModel::inverseMoveRowsSignal(bool state)
{
    mInverseMoveSignal = state;
}

bool TransfersModel::moveTransferPriority(const QModelIndex &sourceParent, const QList<int>& rows,
                              const QModelIndex &destinationParent, int destinationChild)
{
    bool result(false);

    if(!rows.isEmpty())
    {
        foreach(auto sourceRow, rows)
        {
            //TODO MOVE TO TOP THE SECOND ITEM
            int lastRow (sourceRow);

            if (sourceParent == destinationParent
                    && (destinationChild < sourceRow || destinationChild > lastRow))
            {
                // To keep order, do from first to last if destination is before first,
                // and from last to first if destination is after last.
                bool ascending (destinationChild < sourceRow ? false : true);

                QList<TransferTag> tagsToMove;

                QMutexLocker lock(&mModelMutex);

                for (auto row (sourceRow); row <= lastRow; ++row)
                {
                    auto tag(getTransfer(row)->mTag);

                    if(tag)
                    {
                        if (ascending)
                        {
                            tagsToMove.push_back(tag);
                        }
                        else
                        {
                            tagsToMove.push_front(tag);
                        }
                    }
                }

                for (auto tag : tagsToMove)
                {
                    if(destinationChild == -1)
                    {
                        mMegaApi->moveTransferToFirstByTag(tag);
                    }
                    else if (destinationChild == -2)
                    {
                        mMegaApi->moveTransferToLastByTag(tag);
                    }
                    else
                    {
                        // Get target
                        auto target (getTransfer(destinationChild));

                        if(target)
                        {
                            mMegaApi->moveTransferBeforeByTag(tag, target->mTag);

                            if(!mIgnoreMoveSignal && mInverseMoveSignal)
                            {
                                emit rowsAboutToBeMoved(target->mTag);
                            }
                        }
                    }

                    if(!mIgnoreMoveSignal && !mInverseMoveSignal)
                    {  
                        emit rowsAboutToBeMoved(tag);
                    }
                }

                result = true;
            }
        }
    }

    return result;
}

void TransfersModel::resetModel()
{
    QMutexLocker lock(&mModelMutex);

    beginResetModel();

    mTransfersCount.clear();
    mTransfers.clear();
    mTransferEventWorker->clear();
    mTransfersToProcess.clear();
    mTransfersProcessChanged = 0;
    mUpdateMostPriorityTransfer = 0;
    mUiBlockedCounter = 0;
    mTagByOrder.clear();

    endResetModel();
}

Qt::ItemFlags TransfersModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
    {
        auto indexData = getTransfer(index.row());
        if(indexData)
        {
            if(indexData->isFinished())
            {
                return QAbstractItemModel::flags(index);
            }
            else
            {
                return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
            }
        }
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
}

Qt::DropActions TransfersModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* TransfersModel::mimeData(const QModelIndexList& indexes) const
{
    QByteArray byteArray;
    QDataStream stream (&byteArray, QIODevice::WriteOnly);
    QList<TransferTag> tags;

    mModelMutex.lock();

    for (auto index : indexes)
    {
        auto transfer = getTransfer(index.row());
        tags.push_back(static_cast<TransferTag>(transfer->mTag));
    }

    mModelMutex.unlock();

    stream << tags;

    QMimeData* data = new QMimeData();
    data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), byteArray);

    emit internalMoveStarted();

    connect(data, &QMimeData::destroyed, this, [this](){
        emit internalMoveFinished();
    });

    return data;
}

bool TransfersModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                  int column, const QModelIndex& parent)
{
    Q_UNUSED(column)

    if (destRow >= 0 && destRow <= rowCount(DEFAULT_IDX) && action == Qt::MoveAction)
    {
        QList<int> rows = getDragAndDropRows(data);

        moveTransferPriority(parent, rows, parent, destRow);
    }

    // Return false to avoid row deletion...dirty!
    return false;
}

QList<int> TransfersModel::getDragAndDropRows(const QMimeData *data)
{
    QByteArray byteArray (data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist")));
    QDataStream stream (&byteArray, QIODevice::ReadOnly);
    QList<TransferTag> tags;
    stream >> tags;

    QList<int> rows;
    for (auto tag : qAsConst(tags))
    {
        rows.push_back(getRowByTransferTag(tag));
    }

    return rows;
}
