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

const int MAX_UPDATE_TRANSFERS = 2000;

//LISTENER THREAD
TransferThread::TransferThread()
{
}

QList<QExplicitlySharedDataPointer<TransferData>> TransferThread::processUpdates()
{
    if(mCacheUpdateTransfersByTag.size()!= 0)
    {
        QMutexLocker lock(&mCacheMutex);

        if(mCacheUpdateTransfersByTag.size() > MAX_UPDATE_TRANSFERS)
        {
            QList<QExplicitlySharedDataPointer<TransferData>> auxList;
            for(auto index = 0; index < MAX_UPDATE_TRANSFERS
                && !mCacheUpdateTransfersByTag.isEmpty(); ++index)
            {
                auto& firstItem = mCacheUpdateTransfersByTag.first();
                if(firstItem)
                {
                    auxList.append(firstItem);
                    mCacheUpdateTransfersByTag.take(firstItem->mTag);
                }
            }

            return auxList;
        }
        else
        {
            auto auxList = mCacheUpdateTransfersByTag.values();
            mCacheUpdateTransfersByTag.clear();

            return auxList;
        }
    }

    return QList<QExplicitlySharedDataPointer<TransferData>>();
}

QExplicitlySharedDataPointer<TransferData> TransferThread::createData(MegaTransfer *transfer)
{
    QExplicitlySharedDataPointer<TransferData> d (new TransferData(transfer));
    return d;
}

QExplicitlySharedDataPointer<TransferData> TransferThread::onTransferEvent(MegaTransfer *transfer)
{
    QMutexLocker lock(&mCacheMutex);

    QExplicitlySharedDataPointer<TransferData> data;

    auto found = mCacheUpdateTransfersByTag.value(transfer->getTag());

    if(found)
    {
        if(found->mNotificationNumber < transfer->getNotificationNumber())
        {
            mCacheUpdateTransfersByTag.remove(transfer->getTag());

            data = createData(transfer);
            mCacheUpdateTransfersByTag.insert(transfer->getTag(), data);
        }
    }
    else
    {
        data = createData(transfer);
        mCacheUpdateTransfersByTag.insert(transfer->getTag(), data);
    }

    return data;
}

TransfersCount TransferThread::getTransfersCount()
{
    QMutexLocker lock(&mCountersMutex);
    return mTransfersCount;
}

void TransferThread::resetCompletedUploads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset)
{
    QMutexLocker lock(&mCountersMutex);

    unsigned long long totalTransferredBytes(0);
    foreach(auto& transfer, transfersToReset)
    {
        totalTransferredBytes += transfer->mTransferredBytes != 0 ? transfer->mTransferredBytes : transfer->mTotalSize;
        mTransfersCount.transfersByType[transfer->mFileType]--;
        mTransfersCount.transfersFinishedByType[transfer->mFileType]--;
    }

    mTransfersCount.totalUploads -= transfersToReset.size();
    mTransfersCount.completedUploadBytes -= totalTransferredBytes;
    mTransfersCount.totalUploadBytes -= totalTransferredBytes;
}

void TransferThread::resetCompletedDownloads(QList<QExplicitlySharedDataPointer<TransferData>> transfersToReset)
{
    QMutexLocker lock(&mCountersMutex);

    unsigned long long totalTransferredBytes(0);
    foreach(auto& transfer, transfersToReset)
    {
        totalTransferredBytes += transfer->mTransferredBytes != 0 ? transfer->mTransferredBytes : transfer->mTotalSize;
        mTransfersCount.transfersByType[transfer->mFileType]--;
        mTransfersCount.transfersFinishedByType[transfer->mFileType]--;
    }

    mTransfersCount.totalDownloads -= transfersToReset.size();
    mTransfersCount.completedDownloadBytes -= totalTransferredBytes;
    mTransfersCount.totalDownloadBytes -= totalTransferredBytes;
}

void TransferThread::resetCompletedTransfers()
{
    QMutexLocker lock(&mCountersMutex);

    //Downloads
    mTransfersCount.totalDownloads = mTransfersCount.pendingDownloads;
    mTransfersCount.completedDownloadBytes = mTransfersCount.totalDownloadBytes - mTransfersCount.completedDownloadBytes;
    mTransfersCount.completedDownloadBytes = 0;

    //Uploads
    mTransfersCount.totalUploads = mTransfersCount.pendingUploads;
    mTransfersCount.completedUploadBytes = mTransfersCount.totalUploadBytes - mTransfersCount.completedUploadBytes;
    mTransfersCount.completedUploadBytes = 0;

    //Transfers by type
    foreach(auto& fileType, mTransfersCount.transfersByType.keys())
    {
        auto finishedTransfersByType = mTransfersCount.transfersFinishedByType.value(fileType);
        mTransfersCount.transfersByType[fileType] -= finishedTransfersByType;
        mTransfersCount.transfersFinishedByType[fileType] = 0;
    }
}

void TransferThread::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        onTransferEvent(transfer);

        QMutexLocker lock(&mCountersMutex);
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
    }
}

void TransferThread::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        onTransferEvent(transfer);

        QMutexLocker lock(&mCountersMutex);
        if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
        }
        else
        {
            mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
        }
    }
}

void TransferThread::onTransferFinish(MegaApi*, MegaTransfer *transfer, MegaError* error)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        auto data = onTransferEvent(transfer);

        QMutexLocker lock(&mCountersMutex);
        auto fileType = Utilities::getFileType(QString::fromStdString(transfer->getFileName()), QString());
        if(transfer->getState() == MegaTransfer::STATE_CANCELLED)
        {
            mTransfersCount.transfersByType[fileType]--;

            if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
            {
                mTransfersCount.completedUploadBytes -= transfer->getTransferredBytes();
                mTransfersCount.totalUploadBytes -= transfer->getDeltaSize();
                mTransfersCount.pendingUploads--;
                mTransfersCount.totalUploads--;
            }
            else
            {
                mTransfersCount.completedDownloadBytes -= transfer->getTransferredBytes();
                mTransfersCount.totalDownloadBytes -= transfer->getDeltaSize();
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
            }
            else
            {
                mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
                mTransfersCount.pendingDownloads--;
            }
        }

        if(error)
        {
            data->mFailedTransfer = std::shared_ptr<mega::MegaTransfer>(transfer->copy());
        }
    }
}

void TransferThread::onTransferTemporaryError(MegaApi*, MegaTransfer *transfer, MegaError *)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        auto data = onTransferEvent(transfer);
        data->mTemporaryError = true;

        QMutexLocker lock(&mCountersMutex);
        if(transfer->getType() == MegaTransfer::TYPE_UPLOAD)
        {
            mTransfersCount.completedUploadBytes += transfer->getDeltaSize();
        }
        else
        {
            mTransfersCount.completedDownloadBytes += transfer->getDeltaSize();
        }
    }
}

const int PROCESS_TIMER = 100;

TransfersModel::TransfersModel(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mPreferences (Preferences::instance()),
    mTransfersCancelling(false)
{
    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");

    mAreAllPaused = mPreferences->getGlobalPaused();

    mTransferEventThread = new QThread();
    mTransferEventWorker = new TransferThread();
    delegateListener = new QTMegaTransferListener(mMegaApi, mTransferEventWorker);
    mTransferEventWorker->moveToThread(mTransferEventThread);
    delegateListener->moveToThread(mTransferEventThread);
    mMegaApi->addTransferListener(delegateListener);

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
    mMegaApi->removeTransferListener(mTransferEventWorker);
    mTransferEventThread->quit();
    mTransferEventThread->deleteLater();
    mTransferEventWorker->deleteLater();
}

void TransfersModel::pauseModelProcessing(bool value)
{
    if(value)
    {
        mTimer.stop();
    }
    else
    {
        mTimer.start(PROCESS_TIMER);
    }
}

bool TransfersModel::areAllPaused()
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

QModelIndex TransfersModel::index(int row, int, const QModelIndex&) const
{
    return (row < rowCount(DEFAULT_IDX)) ?  createIndex(row, 0) : DEFAULT_IDX;
}

void TransfersModel::initModel()
{
    emit pauseStateChanged(mAreAllPaused);
}

void TransfersModel::onProcessTransfers()
{
    if(mTransfersToStart.size() == 0
            && mTransfersToCancel.size() == 0
            && mTransfersToUpdate.size() == 0)
    {
        if(mModelMutex.tryLock())
        {
            auto transfersToProcess = mTransferEventWorker->processUpdates();

            auto it = transfersToProcess.begin();
            while (it != transfersToProcess.end())
            {
                auto state ((*it)->mState);
                int row = mTagByOrder.value((*it)->mTag, -1);

                if(row >= 0)
                {
                    if(state == TransferData::TransferState::TRANSFER_CANCELLED)
                    {
                        mTransfersToCancel.push_back((*it));
                    }
                    else
                    {
                        mTransfersToUpdate.push_back((*it));
                    }
                }
                else
                {
                    mTransfersToStart.push_back((*it));
                }

                it++;
            }

            mModelMutex.unlock();


        }
    }

    //It is done before processing the transfers as the process itself clears the list of canceled transfers
    if(mTransfersCancelling && mTransfersToCancel.size() == 0)
    {
        mTransfersCancelling = false;
        emit transfersCanceled();
    }

    if(mTransfersToCancel.size() != 0
            || mTransfersToUpdate.size() != 0
            || mTransfersToStart.size() != 0)
    {
        if(mTransfersToCancel.size() != 0)
        {
            QtConcurrent::run([this](){
                if(!mTransfersCancelling)
                {
                    emit transfersAboutToBeCanceled();
                    mTransfersCancelling = true;
                }

                if(mModelMutex.tryLock())
                {
                    processCancelTransfers();
                    sendDataChanged();

                    updateTransfersCount();

                    mModelMutex.unlock();
                }
            });
        }

        bool transfersCountNeedsUpdate(false);

        //Do not add new transfers while items are being cancelled
        if(mModelMutex.tryLock())
        {
            processStartTransfers();
            transfersCountNeedsUpdate = true;

            mModelMutex.unlock();
        }

        if(mModelMutex.tryLock())
        {
            processUpdateTransfers();

            auto rowsToUpdate(mRowsToUpdate.size());
            sendDataChanged();

            transfersCountNeedsUpdate = true;

            mModelMutex.unlock();
        }

        if(transfersCountNeedsUpdate)
        {
            updateTransfersCount();
        }
    }
}

void TransfersModel::processStartTransfers()
{
    if (mTransfersToStart.size() != 0)
    {
        auto totalRows = rowCount(DEFAULT_IDX);
        auto rowsToBeInserted(static_cast<int>(mTransfersToStart.size()));

        beginInsertRows(DEFAULT_IDX, totalRows, totalRows + rowsToBeInserted - 1);

        for (auto it = mTransfersToStart.begin(); it != mTransfersToStart.end();)
        {
            startTransfer((*it));
            mTransfersToStart.erase(it++);
        }

        endInsertRows();
    }
}

void TransfersModel::startTransfer(QExplicitlySharedDataPointer<TransferData> transfer)
{
    mTransfers.append(transfer);
    mTagByOrder.insert(transfer->mTag, rowCount(DEFAULT_IDX) - 1);

    auto state (transfer->mState);
    auto tag = transfer->mTag;

    if (mAreAllPaused && (state & TransferData::PAUSABLE_STATES_MASK))
    {
        mMegaApi->pauseTransferByTag(tag, true);
    }
}

void TransfersModel::processUpdateTransfers()
{
    for (auto it = mTransfersToUpdate.begin(); it != mTransfersToUpdate.end();)
    {
        auto row = updateTransfer((*it));
        if(row >= 0 && !mRowsToUpdate.contains(row))
        {
            mRowsToUpdate.append(row);
        }

        mTransfersToUpdate.erase(it++);
    }
}

int TransfersModel::updateTransfer(QExplicitlySharedDataPointer<TransferData> transfer)
{
    TransferTag tag (transfer->mTag);

    auto row = mTagByOrder.value(tag);
    auto d  = getTransfer(row);
    if(d)
    {
        mTransfers[row] = transfer;
    }

    return row;
}

void TransfersModel::processCancelTransfers()
{
    if(mTransfersToCancel.size() > 0)
    {
        QModelIndexList indexesToCancel;

        for (auto it = mTransfersToCancel.begin(); it != mTransfersToCancel.end();)
        {
            auto row = mTagByOrder.value((*it)->mTag);
            indexesToCancel.append(index(row,0, DEFAULT_IDX));

            mTransfersToCancel.erase(it++);
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
    return mTransferEventWorker->getTransfersCount();
}

void TransfersModel::resetCompletedTransfersCount()
{
    mTransferEventWorker->resetCompletedTransfers();

    updateTransfersCount();
}

void TransfersModel::cancelTransfers(const QModelIndexList& indexes)
{
    if(indexes.isEmpty())
    {
        clearTransfers(QModelIndexList());

        mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
        mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
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
            QMegaMessageBox::warning(nullptr, QString::fromUtf8("MEGAsync"),
                                         tr("Transfers cannot be cancelled or cleared"),
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
    if (d)
    {
        if(d->isFinished())
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
    for (auto index : indexes)
    {
        TransferTag tag (getTransfer(index.row())->mTag);
        pauseResumeTransferByTag(tag, pauseState);
    }
}

void TransfersModel::pauseResumeAllTransfers(bool state)
{
    mAreAllPaused = state;

    int counterToRefreshUI(0);

    if (mAreAllPaused)
    {
        mMegaApi->pauseTransfers(mAreAllPaused);
        std::for_each(mTransfers.crbegin(), mTransfers.crend(), [this, counterToRefreshUI](QExplicitlySharedDataPointer<TransferData> item)
        mutable {

            if(item->mState & TransferData::PAUSABLE_STATES_MASK)
            {
                item->mState = TransferData::TRANSFER_PAUSED;
            }

            pauseResumeTransferByTag(item->mTag, mAreAllPaused);

            if(counterToRefreshUI % 1000 == 0)
            {
                qApp->processEvents();
            }

            counterToRefreshUI++;

            mRowsToUpdate.append(mTagByOrder.value(item->mTag));
        });
    }
    else
    {
        std::for_each(mTransfers.cbegin(), mTransfers.cend(), [this, counterToRefreshUI](QExplicitlySharedDataPointer<TransferData> item)
        mutable {

            if(item->mState & TransferData::TRANSFER_PAUSED)
            {
                item->mState = TransferData::TRANSFER_QUEUED;
            }

            pauseResumeTransferByTag(item->mTag, mAreAllPaused);

            if(counterToRefreshUI % 1000 == 0)
            {
                qApp->processEvents();
            }

            counterToRefreshUI++;

            mRowsToUpdate.append(mTagByOrder.value(item->mTag));
        });
        mMegaApi->pauseTransfers(mAreAllPaused);
    }

    emit pauseStateChanged(mAreAllPaused);
}

void TransfersModel::cancelScanning()
{

}

void TransfersModel::pauseResumeTransferByTag(TransferTag tag, bool pauseState)
{
    auto row = mTagByOrder.value(tag);
    auto d  = getTransfer(row);

    if(!pauseState && mAreAllPaused)
    {
        mMegaApi->pauseTransfers(pauseState);
        mAreAllPaused = false;
        emit pauseStateChangedByTransferResume();
    }

    mMegaApi->pauseTransferByTag(d->mTag, pauseState);
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
    return mTransferEventWorker->getTransfersCount().transfersByType.value(fileType);
}

long long TransfersModel::getNumberOfFinishedForFileType(Utilities::FileType fileType) const
{
    return mTransferEventWorker->getTransfersCount().transfersFinishedByType.value(fileType);
}

void TransfersModel::updateTransfersCount()
{
    emit transfersCountUpdated();
}

void TransfersModel::removeRows(QModelIndexList& indexesToRemove)
{
    std::sort(indexesToRemove.begin(), indexesToRemove.end(),[](QModelIndex check1, QModelIndex check2){
        return check1.row() > check2.row();
    });

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (indexesToRemove.last().row());
    for (auto index : indexesToRemove)
    {
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
        row--;
    }
    // Flush pooled rows (start at row + 1).
    // This happens when the last item processed is in a finished state.
    if (count > 0)
    {
        removeRows(row + 1, count, DEFAULT_IDX);
    }
}

QExplicitlySharedDataPointer<TransferData> TransfersModel::getTransfer(int row) const
{
    return mTransfers.at(row);
}

void TransfersModel::removeTransfer(int row)
{
    mTransfers.removeAt(row);
}

void TransfersModel::sendDataChanged()
{
    foreach(auto& row, mRowsToUpdate)
    {
        QModelIndex topLeft (index(row, 0, DEFAULT_IDX));
        emit dataChanged(topLeft, topLeft);
    }
    mRowsToUpdate.clear();
}

void TransfersModel::onPauseStateChanged()
{
    bool newPauseState (mPreferences->getGlobalPaused());
    if (newPauseState != mAreAllPaused)
    {
        pauseResumeAllTransfers(!mAreAllPaused);
    }
}

void TransfersModel::onRetryTransfer(TransferTag tag)
{
    QtConcurrent::run([this, tag](){
        auto transfer = mMegaApi->getTransferByTag(tag);

        if (transfer)
        {
            mMegaApi->retryTransfer(transfer);
        }
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

        mTagByOrder.clear();
        //Recalculate rest of items
        for(int row = 0; row < rowCount(DEFAULT_IDX); ++row)
        {
            auto item = getTransfer(row);
            mTagByOrder.insert(item->mTag, row);
        }
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
