#include "TransfersModel.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Platform.h"
#include "TransferItem.h"
#include "QMegaMessageBox.h"

#include <QSharedData>

#include <algorithm>

using namespace mega;

static const QModelIndex DEFAULT_IDX = QModelIndex();

const int MAX_TRANSFERS = 2000;
const int CANCEL_THRESHOLD_THREAD = 100;

//LISTENER THREAD
TransferThread::TransferThread()
{

}

TransferThread::TransfersToProcess TransferThread::processTransfers()
{
   TransfersToProcess transfers;
   if(mCacheMutex.tryLock())
   {
       int spaceForTransfers(MAX_TRANSFERS);

       transfers.canceledTransfersByTag = extractFromCache(mTransfersToProcess.canceledTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.canceledTransfersByTag.size();

       transfers.failedTransfersByTag = extractFromCache(mTransfersToProcess.failedTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.failedTransfersByTag.size();

       transfers.startTransfersByTag = extractFromCache(mTransfersToProcess.startTransfersByTag, spaceForTransfers);
       spaceForTransfers -= transfers.startTransfersByTag.size();

       transfers.updateTransfersByTag = extractFromCache(mTransfersToProcess.updateTransfersByTag, spaceForTransfers);

       mCacheMutex.unlock();
   }

   return transfers;
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
    auto result = checkIfRepeatedAndSubstitute(mTransfersToProcess.startTransfersByTag, transfer);

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
        QMutexLocker counterLock(&mCountersMutex);
        auto fileType = Utilities::getFileType(QString::fromStdString(transfer->getFileName()), QString());
        mTransfersCount.transfersByType[fileType]++;

        if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            mTransfersCount.totalUploads++;
            mTransfersCount.pendingUploads++;
            mTransfersCount.totalUploadBytes += transfer->getTotalBytes();
            mTransfersCount.completedUploadBytes += transfer->getTransferredBytes();
        }
        else
        {
            mTransfersCount.totalDownloads++;
            mTransfersCount.pendingDownloads++;
            mTransfersCount.totalDownloadBytes += transfer->getTotalBytes();
            mTransfersCount.completedDownloadBytes += transfer->getTransferredBytes();
        }

        QMutexLocker cacheLock(&mCacheMutex);

        auto data = onTransferEvent(transfer);

        if(data)
        {
            mTransfersToProcess.startTransfersByTag.insert(transfer->getTag(), data);
        }
    }
}

void TransferThread::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        QMutexLocker counterLock(&mCountersMutex);
        if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
        }
        else
        {
            mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
        }

        QMutexLocker cacheLock(&mCacheMutex);
        auto data = onTransferEvent(transfer);

        if(data)
        {
            mTransfersToProcess.updateTransfersByTag.insert(transfer->getTag(), data);
        }
    }
}

void TransferThread::onTransferFinish(MegaApi*, MegaTransfer *transfer, MegaError*)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        QMutexLocker counterLock(&mCountersMutex);
        auto fileType = Utilities::getFileType(QString::fromStdString(transfer->getFileName()), QString());
        if(transfer->getState() == MegaTransfer::STATE_CANCELLED)
        {
            mTransfersCount.transfersByType[fileType]--;

            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.completedUploadBytes -= transfer->getTransferredBytes();
                mTransfersCount.totalUploadBytes -= transfer->getTotalBytes();
                mTransfersCount.pendingUploads--;
                mTransfersCount.totalUploads--;
            }
            else
            {
                mTransfersCount.completedDownloadBytes -= transfer->getTransferredBytes();
                mTransfersCount.totalDownloadBytes -= transfer->getTotalBytes();
                mTransfersCount.pendingDownloads--;
                mTransfersCount.totalDownloads--;
            }
        }
        else
        {
            mTransfersCount.transfersFinishedByType[fileType]++;
            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
                mTransfersCount.pendingUploads--;

                if(transfer->getState() == MegaTransfer::STATE_FAILED)
                {
                    mTransfersCount.failedUploads++;
                }
            }
            else
            {
                mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                mTransfersCount.pendingDownloads--;

                if(transfer->getState() == MegaTransfer::STATE_FAILED)
                {
                    mTransfersCount.failedDownloads++;
                }
            }
        }

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

void TransferThread::onTransferTemporaryError(MegaApi*, MegaTransfer *transfer, MegaError *)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        QMutexLocker counterLock(&mCountersMutex);
        if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
        }
        else
        {
            mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
        }

        QMutexLocker cacheLock(&mCacheMutex);
        auto data = onTransferEvent(transfer);
        if(data)
        {
            mTransfersToProcess.updateTransfersByTag.insert(transfer->getTag(), data);
            data->mTemporaryError = true;
        }
    }
}

TransfersCount TransferThread::getTransfersCount()
{
    QMutexLocker lock(&mCountersMutex);
    return mTransfersCount;
}

void TransferThread::resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset)
{
    QMutexLocker lock(&mCountersMutex);

    foreach(auto& transfer, transfersToReset)
    {
        if(mTransfersCount.totalUploads > 0)
        {
            mTransfersCount.totalUploads--;
            mTransfersCount.completedUploadBytes -= transfer->isCompleted() ? transfer->mTotalSize : transfer->mTransferredBytes;
            mTransfersCount.totalUploadBytes -= transfer->mTotalSize;
            mTransfersCount.transfersByType[transfer->mFileType]--;
            mTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->hasFailed())
            {
                mTransfersCount.failedUploads--;
                transfer->removeFailedTransfer();
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
            mTransfersCount.completedDownloadBytes -= transfer->isCompleted() ? transfer->mTotalSize : transfer->mTransferredBytes;
            mTransfersCount.totalDownloadBytes -= transfer->mTotalSize;
            mTransfersCount.transfersByType[transfer->mFileType]--;
            mTransfersCount.transfersFinishedByType[transfer->mFileType]--;

            if(transfer->hasFailed())
            {
                mTransfersCount.failedDownloads--;
                transfer->removeFailedTransfer();
            }
        }
    }
}

///////////////// TRANSFERS MODEL //////////////////////////////////////////////

const int PROCESS_TIMER = 100;
const int RESET_AFTER_EMPTY_RECEIVES = 10;
const unsigned long long ACTIVE_PRIORITY_OFFSET = 100000000000000;
const unsigned long long COMPLETED_PRIORITY_OFFSET = 200000000000000;

TransfersModel::TransfersModel(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mPreferences (Preferences::instance()),
    mCancelingMode(0),
    mFailingMode(0),
    mModelReset(false)
{
    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");

    mAreAllPaused = mPreferences->getGlobalPaused();
    mMegaApi->pauseTransfers(mAreAllPaused);

    mTransferEventThread = new QThread();
    mTransferEventWorker = new TransferThread();
    mDelegateListener = new QTMegaTransferListener(mMegaApi, mTransferEventWorker);
    mTransferEventWorker->moveToThread(mTransferEventThread);
    mDelegateListener->moveToThread(mTransferEventThread);
    mMegaApi->addTransferListener(mDelegateListener);

    //Update transfers state for the first time
    updateTransfersCount();

    mTimer.setInterval(PROCESS_TIMER);
    QObject::connect(&mTimer, &QTimer::timeout, this, &TransfersModel::onProcessTransfers);
    mTimer.start();

    mTransferEventThread->start();
}

TransfersModel::~TransfersModel()
{
    // Cleanup
    mTransfers.clear();

    // Disconect listener
    mMegaApi->removeTransferListener(mDelegateListener);
    mTransferEventThread->quit();
    mTransferEventThread->deleteLater();
    mTransferEventWorker->deleteLater();
}

void TransfersModel::pauseModelProcessing(bool value)
{
    QMutexLocker lock(&mModelMutex);

    if(value)
    {
        mTimer.stop();
    }
    else
    {
        mTimer.start(PROCESS_TIMER);
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
        int containsTransfersToStart(mTransfersToProcess.startTransfersByTag.size());
        int containsTransfersToUpdate(mTransfersToProcess.updateTransfersByTag.size());
        int containsTransfersToCancel(mTransfersToProcess.canceledTransfersByTag.size());
        int containsTransfersFailed(mTransfersToProcess.failedTransfersByTag.size());

        if(containsTransfersToCancel > 0)
        {
            if(containsTransfersToCancel > CANCEL_THRESHOLD_THREAD)
            {
                setCancelingMode(true);

                QtConcurrent::run([this](){
                    if(mModelMutex.tryLock())
                    {
                        blockSignals(true);
                        processCancelTransfers();
                        blockSignals(false);
                        updateTransfersCount();

                        mModelMutex.unlock();
                    }
                });
            }
            else
            {
                if(mModelMutex.tryLock())
                {
                    processCancelTransfers();
                    updateTransfersCount();
                    mModelMutex.unlock();
                }
            }
        }
        else if(isCancelingModeActive())
        {
            setCancelingMode(false);
        }

        if(containsTransfersFailed > 0)
        {
            setFailingMode(true);

            QtConcurrent::run([this](){
                if(mModelMutex.tryLock())
                {
                    blockSignals(true);
                    processFailedTransfers();
                    blockSignals(false);

                    updateTransfersCount();

                    mModelMutex.unlock();
                }
            });
        }
        else
        {
            if(isFailingModeActive())
            {
                setFailingMode(false);
            }

            bool transfersCountNeedsUpdate(false);

            if(containsTransfersToStart > 0)
            {
                //Do not add new transfers while items are being cancelled
                if(mModelMutex.tryLock())
                {
                    processStartTransfers(mTransfersToProcess.startTransfersByTag);
                    transfersCountNeedsUpdate = true;

                    mModelMutex.unlock();
                }
            }

            if(containsTransfersToUpdate > 0)
            {
                if(mModelMutex.tryLock())
                {
                    processUpdateTransfers();
                    transfersCountNeedsUpdate = true;

                    mModelMutex.unlock();
                }
            }


            if(transfersCountNeedsUpdate)
            {
                updateTransfersCount();
            }
        }
    }
    else
    {
        if(isCancelingModeActive())
        {
            setCancelingMode(false);
        }

        if(isFailingModeActive())
        {
            setFailingMode(false);
        }
    }
}

void TransfersModel::processStartTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersToStart)
{
    if (!transfersToStart.isEmpty())
    {
        auto totalRows = rowCount(DEFAULT_IDX);
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

void TransfersModel::startTransfer(QExplicitlySharedDataPointer<TransferData> transfer)
{
    mTransfers.append(transfer);
    mTagByOrder.insert(transfer->mTag, rowCount(DEFAULT_IDX) - 1);

    auto state (transfer->mState);

    if (mAreAllPaused && (state & TransferData::PAUSABLE_STATES_MASK))
    {
        mMegaApi->pauseTransferByTag(transfer->mTag, true);
        transfer->mState = TransferData::TRANSFER_PAUSED;
    }

    updateTransferPriority(transfer);
}

void TransfersModel::processActiveTransfers(QList<QExplicitlySharedDataPointer<TransferData>>& transfersToActive)
{
    if (!transfersToActive.isEmpty())
    {
        auto rowsToBeInserted(static_cast<int>(transfersToActive.size()));

        beginInsertRows(DEFAULT_IDX, 0, rowsToBeInserted - 1);

        for (auto it = transfersToActive.begin(); it != transfersToActive.end();)
        {
            mTransfers.prepend((*it));
            (*it)->mPriority -= ACTIVE_PRIORITY_OFFSET;

            transfersToActive.erase(it++);
        }

        updateTagsByOrder();

        endInsertRows();
    }
}

void TransfersModel::processUpdateTransfers()
{
    QList<int> rowsToUpdate;
    QList<QExplicitlySharedDataPointer<TransferData>> transfersFinished;
    QModelIndexList rowsToRemove;

    for (auto it = mTransfersToProcess.updateTransfersByTag.begin(); it != mTransfersToProcess.updateTransfersByTag.end();)
    {   
        TransferTag tag ((*it)->mTag);

        auto row = mTagByOrder.value(tag);
        auto d  = getTransfer(row);

        if(d && d->hasChanged(*it))
        {
            if((!isCancelingModeActive() && !isFailingModeActive())
                    && ((*it)->isFinished()) || (*it)->isProcessing())
            {
                if(d->mState != (*it)->mState)
                {
                    transfersFinished.append((*it));
                    rowsToRemove.append(index(row,0));
                }
                else
                {
                    updateTransferPriority((*it));
                    mTransfers[row] = (*it);
                    rowsToUpdate.append(row);
                }
            }
            else
            {
                mTransfers[row] = (*it);
                rowsToUpdate.append(row);
            }
        }

        mTransfersToProcess.updateTransfersByTag.erase(it++);
    }

    foreach(auto& row, rowsToUpdate)
    {
        sendDataChanged(row);
    }

    if(!transfersFinished.isEmpty())
    {
        removeRows(rowsToRemove);

        processStartTransfers(transfersFinished);
    }
}

void TransfersModel::processFailedTransfers()
{
    QList<int> rowsToUpdate;
    QList<QExplicitlySharedDataPointer<TransferData>> transfersFinished;
    QModelIndexList rowsFinished;

    for (auto it = mTransfersToProcess.failedTransfersByTag.begin(); it != mTransfersToProcess.failedTransfersByTag.end();)
    {
        TransferTag tag ((*it)->mTag);

        auto row = mTagByOrder.value(tag);

        transfersFinished.append((*it));
        rowsFinished.append(index(row,0));
        rowsToUpdate.append(row);
        auto d  = getTransfer(row);
        if(d)
        {
            mTransfers[row] = (*it);
        }

        mTransfersToProcess.failedTransfersByTag.erase(it++);
    }

    if(!transfersFinished.isEmpty())
    {
        removeRows(rowsFinished);

        processStartTransfers(transfersFinished);
    }
}

void TransfersModel::processCancelTransfers()
{
    if(mTransfersToProcess.canceledTransfersByTag.size() > 0)
    {
        QModelIndexList indexesToCancel;

        for (auto it = mTransfersToProcess.canceledTransfersByTag.begin(); it != mTransfersToProcess.canceledTransfersByTag.end();)
        {
            auto row = mTagByOrder.value((*it)->mTag,-1);
            if(row >= 0)
            {
                indexesToCancel.append(index(row,0, DEFAULT_IDX));
            }

            mTransfersToProcess.canceledTransfersByTag.erase(it++);
        }

        removeRows(indexesToCancel);
    }
}

void TransfersModel::getLinks(QList<int>& rows)
{
    if (!rows.isEmpty())
    {
        QList<MegaHandle> exportList;
        QStringList linkList;

        for (auto row : rows)
        {
            auto d (getTransfer(row));

            MegaNode *node (nullptr);

            if (d->mState == TransferData::TRANSFER_FAILED)
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
        for (auto row : rows)
        {
            auto d (getTransfer(row));

            MegaNode *node (nullptr);

            if (d->mState == TransferData::TRANSFER_FAILED)
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
    QtConcurrent::run([=]
    {
        const auto transferItem (
                    qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
        auto d (transferItem.getTransferData());
        auto path = d->path();
        if (d && !path.isEmpty())
        {
            Platform::showInFolder(path);
        }
    });
}

void TransfersModel::retryTransferByIndex(const QModelIndex& index)
{
    const auto transferItem (
                qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
    auto d (transferItem.getTransferData());

    if(d && d->mFailedTransfer)
    {
        auto failedTransferCopy = d->mFailedTransfer->copy();

        QtConcurrent::run([&failedTransferCopy, this](){
            mMegaApi->retryTransfer(failedTransferCopy);
        });

        QModelIndexList indexToRemove;
        indexToRemove.append(index);
        clearTransfers(indexToRemove);
    }
}

void TransfersModel::retryTransfers(QModelIndexList indexes)
{
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

    QtConcurrent::run([transfersToRetry, this](){
        foreach(auto& failedTransferCopy, transfersToRetry)
        {
            mMegaApi->retryTransfer(failedTransferCopy);
            delete failedTransferCopy;
        }
    });

    clearTransfers(indexes);
}

void TransfersModel::setResetMode()
{
    mModelReset = true;
}

void TransfersModel::openFolderByTag(TransferTag tag)
{
    auto row = mTagByOrder.value(tag);
    auto indexToOpen = index(row, 0);
    if(indexToOpen.isValid())
    {
        openFolderByIndex(indexToOpen);
    }
}

TransfersCount TransfersModel::getTransfersCount()
{
    return mTransfersCount;
}

bool TransfersModel::hasFailedTransfers()
{
    return mTransfersCount.failedDownloads != 0 || mTransfersCount.failedUploads != 0;
}

void TransfersModel::cancelTransfers(const QModelIndexList& indexes, QWidget* canceledFrom)
{
    if(indexes.isEmpty())
    {
        bool someTransfersAreNotCancelable(false);

        auto count = rowCount(DEFAULT_IDX);
        for (auto row = 0; row < count;++row)
        {
            auto d (getTransfer(row));

            // Clear (remove rows of) finished transfers
            if (d)
            {
                if (!d->isCancelable())
                {
                    someTransfersAreNotCancelable = true;
                    break;
                }
            }
        }

        mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);

        if(someTransfersAreNotCancelable)
        {
            QMegaMessageBox::warning(canceledFrom, QString::fromUtf8("MEGAsync"),
                                     tr("Some Transfers cannot be cancelled or cleared. "),
                                     QMessageBox::Ok);
        }
    }
    else
    {
        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
        QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

        QList<TransferTag> toCancel;

        // First clear finished transfers (remove rows), then cancel the others.
        // This way, there is no risk of messing up the rows order with cancel requests.
        for (auto index : indexes)
        {
            auto d (getTransfer(index.row()));

            // Clear (remove rows of) finished transfers
            if (d)
            {
                if (d->isCancelable())
                {
                    toCancel.append(d->mTag);
                }
                else if(d->isFinished())
                {
                    classifyUploadOrDownloadTransfers(uploadToClear, downloadToClear,index);
                }
            }
        }

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

        if(!uploadToClear.isEmpty() && !downloadToClear.isEmpty() && !toCancel.isEmpty())
        {
            QMegaMessageBox::warning(canceledFrom, QString::fromUtf8("MEGAsync"),
                                     tr("Transfer(s) cannot be cancelled or cleared", "", uploadToClear.size() + downloadToClear.size() + toCancel.size()),
                                     QMessageBox::Ok);
        }
    }

    updateTransfersCount();
}

void TransfersModel::classifyUploadOrDownloadTransfers(QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& uploads,
                                                       QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>>& downloads,
                                                       const QModelIndex& index)
{
    auto d (getTransfer(index.row()));

    // Clear (remove rows of) finished transfers
    if (d && d->isFinished())
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

void TransfersModel::clearTransfers(const QModelIndexList& indexes)
{   
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> uploadToClear;
    QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData>> downloadToClear;

    if(indexes.isEmpty())
    {
        for (auto row = 0; row < rowCount(DEFAULT_IDX); ++row)
        {
            auto indexToCheck = index(row, 0);
            classifyUploadOrDownloadTransfers(uploadToClear, downloadToClear,indexToCheck);
        }
    }
    else
    {
        for (auto indexToCheck : indexes)
        {
            classifyUploadOrDownloadTransfers(uploadToClear, downloadToClear,indexToCheck);
        }
    }

    clearTransfers(uploadToClear, downloadToClear);

    updateTransfersCount();
}

void TransfersModel::clearTransfers(const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > uploads,
                                    const QMap<QModelIndex, QExplicitlySharedDataPointer<TransferData> > downloads)
{
    if(!uploads.isEmpty() || !downloads.isEmpty())
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

        removeRows(itemsToRemove);
    }
}

void TransfersModel::pauseTransfers(const QModelIndexList& indexes, bool pauseState)
{
    if(!indexes.isEmpty())
    {
        TransferTag followingTransfer(0);

        for (auto index : indexes)
        {
            auto d = getTransfer(index.row());
            if((pauseState && d->mState & TransferData::PAUSABLE_STATES_MASK)
                    || (!pauseState && d->mState & TransferData::TRANSFER_PAUSED))
            {
                pauseResumeTransferByTag(d->mTag, pauseState);

                if(pauseState && followingTransfer == 0)
                {
                    sendDataChanged(index.row());
                    followingTransfer = d->mTag;
                }
            }
        }

    }
}

void TransfersModel::pauseResumeAllTransfers(bool state)
{
    QMutexLocker lock(&mModelMutex);

    mAreAllPaused = state;

    auto tagsUpdated(0);

    if (mAreAllPaused)
    {
        mMegaApi->pauseTransfers(mAreAllPaused);
        TransferTag followingTag(0);
        unsigned long long followingTagPriority(0);

        std::for_each(mTransfers.crbegin(), mTransfers.crend(), [this, &tagsUpdated, &followingTag, &followingTagPriority](QExplicitlySharedDataPointer<TransferData> item)
                      mutable {

            if(item->mState & TransferData::PAUSABLE_STATES_MASK)
            {
                pauseResumeTransferByTag(item->mTag, mAreAllPaused);
                if(followingTag == 0 || followingTagPriority > item->mPriority)
                {
                    followingTag = item->mTag;
                    followingTagPriority = item->mPriority;
                }
                tagsUpdated++;

                if(tagsUpdated % 1000 == 0)
                {
                    tagsUpdated = 0;

                    qApp->processEvents();
                }
            }
        });

        if(followingTag > 0)
        {
            sendDataChanged(mTagByOrder.value(followingTag));
        }
    }
    else
    {
        std::for_each(mTransfers.cbegin(), mTransfers.cend(), [this, &tagsUpdated](QExplicitlySharedDataPointer<TransferData> item)
                      mutable {

            if(item->mState & TransferData::TRANSFER_PAUSED)
            {
                pauseResumeTransferByTag(item->mTag, mAreAllPaused);

                tagsUpdated++;

                if(tagsUpdated % 1000 == 0)
                {
                    tagsUpdated = 0;

                    qApp->processEvents();
                }

            }
        });
        mMegaApi->pauseTransfers(mAreAllPaused);
    }


    emit pauseStateChanged(mAreAllPaused);
}

void TransfersModel::pauseResumeTransferByTag(TransferTag tag, bool pauseState)
{
    auto row = mTagByOrder.value(tag);
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
            bool wasProcessing (d->isProcessing());
            d->mState = TransferData::TRANSFER_PAUSED;

            if(wasProcessing)
            {
                d->mPriority += ACTIVE_PRIORITY_OFFSET;
                sendDataChanged(row);
            }
        }
        else
        {
            d->mState = TransferData::TRANSFER_QUEUED;
        }

        mMegaApi->pauseTransferByTag(d->mTag, pauseState);
    }
}

void TransfersModel::pauseResumeTransferByIndex(const QModelIndex &index, bool pauseState)
{
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

void TransfersModel::updateTransfersCount()
{
    auto count = mTransferEventWorker->getTransfersCount();

    if(isCancelingModeActive())
    {
        if(mTransfersCount.totalUploads < count.totalUploads
                || mTransfersCount.totalDownloads < count.totalDownloads)
        {
            return;
        }

    }

    mTransfersCount = count;

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

    updateTagsByOrder();
}

QExplicitlySharedDataPointer<TransferData> TransfersModel::getTransfer(int row) const
{
    if(mTransfers.size() > row)
    {
        auto transfer = mTransfers.at(row);
        return transfer;

    }

    return QExplicitlySharedDataPointer<TransferData>();
}

void TransfersModel::removeTransfer(int row)
{
    mTransfers.removeAt(row);
}

void TransfersModel::sendDataChanged(int row)
{
    QModelIndex indexChanged (index(row, 0, DEFAULT_IDX));
    emit dataChanged(indexChanged, indexChanged);
}

bool TransfersModel::isFailingModeActive() const
{
    return mFailingMode > 0;
}

void TransfersModel::setFailingMode(bool state)
{
    if(state)
    {
        if(mFailingMode == 0)
        {
            emit blockUi();
        }

        mFailingMode = RESET_AFTER_EMPTY_RECEIVES;
    }
    else if(!state && mFailingMode != 0)
    {
        mFailingMode--;

        if(mFailingMode == 0 && !isCancelingModeActive())
        {
            emit unblockUi();
        }
    }
}

bool TransfersModel::isCancelingModeActive() const
{
    return mCancelingMode > 0;
}

void TransfersModel::setCancelingMode(bool state)
{
    if(state)
    {
        if(mCancelingMode == 0)
        {
            emit blockUi();
        }

        mCancelingMode = RESET_AFTER_EMPTY_RECEIVES;
    }
    else if(!state && mCancelingMode != 0)
    {
        mCancelingMode--;
        if(mCancelingMode == 0 && !isFailingModeActive())
        {
            if(mModelReset)
            {
                mModelReset = false;
                clearTransfers(QModelIndexList());
            }

            emit unblockUi();
        }
    }
}

void TransfersModel::updateTagsByOrder()
{
    mTagByOrder.clear();

    //Recalculate rest of items
    for(int row = 0; row < rowCount(DEFAULT_IDX); ++row)
    {
        auto item = getTransfer(row);
        mTagByOrder.insert(item->mTag, row);
    }
}

void TransfersModel::updateTransferPriority(QExplicitlySharedDataPointer<TransferData> transfer)
{
    //The larger, the less priority, used to place the transfer on the bottom
    if(transfer->isFinished())
    {
        transfer->mPriority += COMPLETED_PRIORITY_OFFSET;
    }
    //The lower, the more priority, used to place the transfer on the top
    else if(transfer->isProcessing())
    {
        transfer->mPriority -= ACTIVE_PRIORITY_OFFSET;
    }
}

void TransfersModel::onPauseStateChanged()
{
    bool newPauseState (mPreferences->getGlobalPaused());
    if (newPauseState != mAreAllPaused)
    {
        pauseResumeAllTransfers(!mAreAllPaused);
    }
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

bool TransfersModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                              const QModelIndex &destinationParent, int destinationChild)
{
    //TODO MOVE TO TOP THE SECOND ITEM
    int lastRow (sourceRow + count - 1);

    if (sourceParent == destinationParent
            && (destinationChild < sourceRow || destinationChild > lastRow))
    {
        // To keep order, do from first to last if destination is before first,
        // and from last to first if destination is after last.
        bool ascending (destinationChild < sourceRow ? false : true);

        QList<TransferTag> tagsToMove;

        auto rows (rowCount(DEFAULT_IDX));

        for (auto row (sourceRow); row <= lastRow; ++row)
        {
            if (ascending)
            {
                tagsToMove.push_back(getTransfer(row)->mTag);
            }
            else
            {
                tagsToMove.push_front(getTransfer(row)->mTag);
            }
        }

        for (auto tag : tagsToMove)
        {
            auto row = mTagByOrder.value(tag);
            auto d  = getTransfer(row);
            if(destinationChild < 0)
            {
                mMegaApi->moveTransferToFirstByTag(d->mTag);
            }
            else if (destinationChild == rows)
            {
                mMegaApi->moveTransferToLastByTag(d->mTag);
            }
            else
            {
                // Get target
                auto target (getTransfer(destinationChild));

                mMegaApi->moveTransferBeforeByTag(d->mTag, target->mTag);
            }
        }

        return true;
    }
    return false;
}

Qt::ItemFlags TransfersModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
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

    for (auto index : indexes)
    {
        auto transfer = mTransfers.at(index.row());
        tags.push_back(static_cast<TransferTag>(transfer->mTag));
    }

    stream << tags;

    QMimeData* data = new QMimeData();
    data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), byteArray);

    return data;
}

bool TransfersModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                  int column, const QModelIndex& parent)
{
    Q_UNUSED(column)
    QByteArray byteArray (data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist")));
    QDataStream stream (&byteArray, QIODevice::ReadOnly);
    QList<TransferTag> tags;
    stream >> tags;

    if (destRow >= 0 && destRow <= rowCount(DEFAULT_IDX) && action == Qt::MoveAction)
    {
        QList<int> rows;
        for (auto tag : qAsConst(tags))
        {
            rows.push_back(mTagByOrder.value(tag));
        }

        if (destRow == 0)
        {
            std::sort(rows.rbegin(), rows.rend());
        }
        else
        {
            std::sort(rows.begin(), rows.end());
        }

        for (auto row : qAsConst(rows))
        {
            moveRows(parent, row, 1, parent, destRow);
        }
    }

    // Return false to avoid row deletion...dirty!
    return false;
}
