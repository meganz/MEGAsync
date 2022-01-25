#include "QTransfersModel.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Platform.h"

#include <QSharedData>

#include <algorithm>

using namespace mega;

const int QTransfersModel::INIT_ROWS_PER_CHUNK;
const int QTransfersModel::MAX_TRANSFERS_INSTANT_UPDATE;


static const QVector<int> DATA_ROLE = {Qt::DisplayRole};
static const QModelIndex DEFAULT_IDX = QModelIndex();

static const TransferData::TransferStates FINISHED_STATES (
        TransferData::TRANSFER_COMPLETED
        | TransferData::TRANSFER_CANCELLED
        | TransferData::TRANSFER_FAILED);
static const TransferData::TransferStates PAUSABLE_STATES (
        TransferData::TRANSFER_QUEUED
        | TransferData::TRANSFER_ACTIVE
        | TransferData::TRANSFER_RETRYING);
static const TransferData::TransferStates CANCELABLE_STATES (
        TransferData::TRANSFER_QUEUED
        | TransferData::TRANSFER_ACTIVE
        | TransferData::TRANSFER_PAUSED
        | TransferData::TRANSFER_RETRYING);

QTransfersModel::QTransfersModel(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mPreferences (Preferences::instance()),
    mTransfers (),
    mRemainingTimes (),
    mOrder (),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mModelMutex (new QReadWriteLock(QReadWriteLock::Recursive)),
    mUpdateNotificationNumber (0),
    mModelHasTransfers (false),
    mNbTransfersPerFileType(),
    mNbFinishedPerFileType(),
    mNbTransfersPerType(),
    mNbTransfersPerState()
{
    // Init File Types
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.txt"), QString())]
            = TransferData::TYPE_TEXT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.wav"), QString())]
            = TransferData::TYPE_AUDIO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.mkv"), QString())]
            = TransferData::TYPE_VIDEO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.tar"), QString())]
            = TransferData::TYPE_ARCHIVE;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.odt"), QString())]
            = TransferData::TYPE_DOCUMENT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.png"), QString())]
            = TransferData::TYPE_IMAGE;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.bin"), QString())]
            = TransferData::TYPE_OTHER;

    // Connect to pause state change signal
    QObject::connect((MegaApplication*)qApp, &MegaApplication::pauseStateChanged,
                      this, &QTransfersModel::onPauseStateChanged, Qt::QueuedConnection);

    mAreAllPaused = mPreferences->getGlobalPaused();

    qRegisterMetaType<TransferData::FileType>("TransferData::FileType");
    qRegisterMetaType<TransferData::TransferState>("TransferData::TransferState");
    qRegisterMetaType<TransferData::TransferType>("TransferData::TransferType");

    mListener = new QTMegaTransferListener(mMegaApi, this);

    //Update transfers state for the first time
    updateTransfersCount();

    connect(&timer, &QTimer::timeout, this, &QTransfersModel::onTimerTransfers);
    timer.start(300);
    mMegaApi->addTransferListener(mListener);
}

bool QTransfersModel::hasChildren(const QModelIndex& parent) const
{
    if (parent == DEFAULT_IDX)
    {
        return mModelHasTransfers;
    }
    return false;
}

int QTransfersModel::rowCount(const QModelIndex& parent) const
{
    int rowCount (0);
    if (!mModelMutex->tryLockForRead())
    {
        mModelMutex->lockForWrite();
    }
    if (parent == DEFAULT_IDX)
    {
        rowCount = mOrder.size();
    }
    mModelMutex->unlock();
    return rowCount;
}

int QTransfersModel::columnCount(const QModelIndex& parent) const
{
    if (parent == DEFAULT_IDX)
    {
        return 1;
    }
    return 0;
}

QVariant QTransfersModel::data(const QModelIndex& index, int role) const
{
    int row (index.row());
    QVariant value;

    if (!mModelMutex->tryLockForRead())
    {
        mModelMutex->lockForWrite();
    }


    if (role == Qt::DisplayRole && row >= 0 && row < mOrder.size())
    {
        auto tag (mOrder.at(row));
        value = mTransfers.value(tag);
    }

    mModelMutex->unlock();


    return value;
}

QModelIndex QTransfersModel::parent(const QModelIndex&) const
{
    return DEFAULT_IDX;
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex& parent) const
{
    if (!mModelMutex->tryLockForRead())
    {
        mModelMutex->lockForWrite();
    }
    if (parent == DEFAULT_IDX && column == 0 && row >= 0 && row < mOrder.size())
    {
        auto tag (mOrder.at(row));
        mModelMutex->unlock();
        return createIndex(row, 0, static_cast<quintptr>(tag));
    }
    mModelMutex->unlock();
    return DEFAULT_IDX;
}

QTransfersModel::~QTransfersModel()
{
    // Wait for init to return
//    mInitFuture.cancel();
//    mInitFuture.waitForFinished();

    // Cleanup
    mModelMutex->lockForWrite();
    //qDeleteAll(mFailedTransfers);
    for (auto transfer : qAsConst(mTransfers))
    {
        auto transferItem (static_cast<TransferItem*>(transfer.data()));
        auto d (transferItem->getTransferData());
    }
    qDeleteAll(mRemainingTimes);
    mModelMutex->unlock();

    // Disconect listener
    mMegaApi->removeTransferListener(mListener);
    mListener->deleteLater();
}

void QTransfersModel::initModel()
{
    emit pauseStateChanged(mAreAllPaused);
//    mInitFuture = QtConcurrent::run([=]
//    {

//        onTimerTransfers();
//        mMegaApi->addTransferListener(mListener);
//    });
}

void QTransfersModel::onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer)
{
   mCacheStartTransfers[transfer->getTag()] =
           std::move(std::unique_ptr<mega::MegaTransfer>(transfer->copy()));
}

void QTransfersModel::onTransferFinish(mega::MegaApi*, mega::MegaTransfer* transfer,
                                        mega::MegaError* error)
{
    mCacheFinishedTransfers.push_back(std::move(
                std::make_pair(std::move(std::unique_ptr<mega::MegaTransfer>(transfer->copy())),
                               std::move(std::unique_ptr<mega::MegaError>(error->copy())))));
}

void QTransfersModel::onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer)
{
    auto transferTag (transfer->getTag());
    auto notificationNumber (transfer->getNotificationNumber());
    auto transferStart (mCacheStartTransfers.find(transferTag));

    // If transfer start has not been taken into account yet, replace the transfer
    // in the transfers to start list
    if (transferStart != mCacheStartTransfers.end())
    {
        // Only replace if the update is more recent
        if (notificationNumber > transferStart->second->getNotificationNumber())
        {
            transferStart->second.reset(transfer->copy());
        }
    }
    else
    {
        if (mOrder.size() > MAX_TRANSFERS_INSTANT_UPDATE)
        {
            auto existingTransfer = mCacheUpdateTransfers.find(transferTag);
            if (existingTransfer != mCacheUpdateTransfers.end())
            {
                // Only replace if the update is more recent
                if (notificationNumber > existingTransfer->second.first->getNotificationNumber())
                {
                    existingTransfer->second.first.reset(transfer->copy());
                    existingTransfer->second.second.reset(nullptr);
                }
            }
            else
            {
                mCacheUpdateTransfers[transferTag] = std::move(
                     std::make_pair(std::move(std::unique_ptr<mega::MegaTransfer>(transfer->copy())),
                                    std::move(std::unique_ptr<mega::MegaError>(nullptr))));
        }
        }
        else
        {
            if (notificationNumber > mUpdateNotificationNumber)
            {
                mUpdateNotificationNumber = notificationNumber;
                updateTransfer(transfer);
            }

            // Update stats
            updateTransfersCount();
            emit transfersDataUpdated();
        }
        mUpdatedTransfers.insert(transferTag);
    }
}

void QTransfersModel::onTransferTemporaryError(mega::MegaApi*, mega::MegaTransfer* transfer,
                                                mega::MegaError* error)
{
    auto transferTag (transfer->getTag());
    auto notificationNumber (transfer->getNotificationNumber());
    auto transferStart (mCacheStartTransfers.find(transferTag));

    // If transfer start has not been taken into account yet, replace the transfer
    // in the transfers to start list
    if (transferStart != mCacheStartTransfers.end())
    {
        // Only replace if the update is more recent
        if (notificationNumber > transferStart->second->getNotificationNumber())
        {
            transferStart->second.reset(transfer->copy());
        }
    }
    else
    {
        if (mOrder.size() > MAX_TRANSFERS_INSTANT_UPDATE)
        {
            auto existingTransfer = mCacheUpdateTransfers.find(transferTag);
            if (existingTransfer != mCacheUpdateTransfers.end())
            {
                // Only replace if the update is more recent
                if (notificationNumber > existingTransfer->second.first->getNotificationNumber())
                {
                    existingTransfer->second.first.reset(transfer->copy());
                    existingTransfer->second.second.reset(error->copy());
                }
            }
            else
            {
                mCacheUpdateTransfers[transferTag]
                        = std::move(std::make_pair(
                                        std::move(std::unique_ptr<mega::MegaTransfer>(transfer->copy())),
                                        std::move(std::unique_ptr<mega::MegaError>(error->copy()))));
            }
        }
        else
        {
            if (notificationNumber > mUpdateNotificationNumber)
            {
                mUpdateNotificationNumber = notificationNumber;
                transferTemporaryError(transfer, error);
            }

            // Update stats
            updateTransfersCount();
            emit transfersDataUpdated();
        }
        mUpdatedTransfers.insert(transferTag);
    }
}

bool QTransfersModel::onTimerTransfers()
{
    bool updateNeeded(false);
    int nbTransfersToStart (static_cast<int>(mCacheStartTransfers.size()));
//    static std::unique_ptr<mega::MegaApiLock> megaApiLock (mMegaApi->getMegaApiLock(false));
//    megaApiLock->lockOnce();
    if (nbTransfersToStart)
    {
        mModelMutex->lockForWrite();
        auto totalRows = rowCount(DEFAULT_IDX);
        beginInsertRows(DEFAULT_IDX, totalRows, totalRows + nbTransfersToStart);

        for (auto& transfer : mCacheStartTransfers)
        {
            startTransfer(std::move(transfer.second));
            mUpdatedTransfers.insert(transfer.first);
        }
        endInsertRows();
        mModelMutex->unlock();
        mCacheStartTransfers.clear();
        updateNeeded = true;
    }

    if(!mCacheUpdateTransfers.empty())
    {
        for (const auto& transfer : mCacheUpdateTransfers)
        {
            if (transfer.second.second)
            {
                transferTemporaryError(transfer.second.first.get(),
                                       transfer.second.second.get());
            }
            else
            {
                updateTransfer(transfer.second.first.get());
            }
        }
        mCacheUpdateTransfers.clear();
        updateNeeded = true;
    }

    auto transferInfo (mCacheFinishedTransfers.begin());
    while (transferInfo != mCacheFinishedTransfers.end())
    {
        auto& transfer (transferInfo->first);
        if (mUpdatedTransfers.find(transfer->getTag()) == mUpdatedTransfers.end())
        {
            auto& error (transferInfo->second);
            finishTransfer(mMegaApi, std::move(transfer), std::move(error));
            transferInfo = mCacheFinishedTransfers.erase(transferInfo);
            updateNeeded = true;
        }
        else
        {
            ++transferInfo;
        }
    }

    if(updateNeeded)
    {
        //Update stats
        updateTransfersCount();
        emit transfersDataUpdated();
    }

    mUpdatedTransfers.clear();

//    megaApiLock->unlockOnce();
    return updateNeeded;
}

void QTransfersModel::startTransfer(std::unique_ptr<mega::MegaTransfer> transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer())
    {
        return;
    }

    mModelMutex->lockForWrite();

    auto nbRows (mOrder.size());

    insertTransfer(mMegaApi, transfer.get(), nbRows);

    auto state (static_cast<TransferData::TransferState>(1 << transfer->getState()));
    if (mAreAllPaused && (state & PAUSABLE_STATES))
    {
        mMegaApi->pauseTransfer(transfer.get(), true);
    }
    mModelMutex->unlock();

    QModelIndex idx (index(nbRows, 0, DEFAULT_IDX));
    emit dataChanged(idx, idx, DATA_ROLE);
}

void QTransfersModel::updateTransfer(mega::MegaTransfer* transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lockForWrite();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));

    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());
        QVariant& v (mTransfers[tag]);
        auto transferItem (v.value<TransferItem>());
        auto d (transferItem.getTransferData());

        auto transferredBytes (static_cast<unsigned long long>(transfer->getTransferredBytes()));
        TransferData::TransferState state (
                    static_cast<TransferData::TransferState>(1 << transfer->getState()));
        auto prevState = d->mState;
        d->mState = (TransferData::TransferState)(1 << transfer->getState());
        auto priority (transfer->getPriority());
        auto prevPriority (d->mPriority);

        if (prevState != state
                || priority != prevPriority
                || transferredBytes >= d->mTransferredBytes)
        {
            auto speed (static_cast<unsigned long long>(transfer->getSpeed()));
            auto totalBytes (static_cast<unsigned long long>(transfer->getTotalBytes()));
            auto rem (mRemainingTimes[transfer->getTag()]);
            auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes - transferredBytes));
            int errorCode (MegaError::API_OK);
            auto errorValue (0LL);
            auto megaError (transfer->getLastErrorExtended());
            if (megaError)
            {
                errorCode = megaError->getErrorCode();
                errorValue = megaError->getValue();
            }

            transferItem.updateValuesTransferUpdated(remSecs.count(), errorCode, errorValue,
                                                     static_cast<unsigned long long>(transfer->getMeanSpeed()),
                                                     speed, priority,
                                                     state, transferredBytes);
            if(transferItem.getTransferData()->mFilename.isEmpty())
            {
                auto a = 10;
            }

            v = QVariant::fromValue(transferItem);

            // Keep statistics up to date
            if (prevState != state)
            {
                mNbTransfersPerState[prevState]--;
                mNbTransfersPerState[state]++;
            }

            QModelIndex idx (index(row, 0, DEFAULT_IDX));
            emit dataChanged(idx, idx, DATA_ROLE);
        }
    }

    mModelMutex->unlock();
}

void QTransfersModel::finishTransfer(MegaApi*, std::unique_ptr<mega::MegaTransfer> transfer,
                                      std::unique_ptr<mega::MegaError> error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lockForWrite();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());

        QVariant& v (mTransfers[tag]);
        auto transferItem (v.value<TransferItem>());

        auto d (transferItem.getTransferData());
        TransferData::TransferState state (
                    static_cast<TransferData::TransferState>(1 << transfer->getState()));
        auto prevState (d->mState);
        int  errorCode (MegaError::API_OK);
        auto errorValue (0LL);
        if (error)
        {
            errorCode = error->getErrorCode();
            errorValue = error->getValue();
        }
        auto transferredBytes (static_cast<unsigned long long>(transfer->getTransferredBytes()));
        auto meanSpead (static_cast<unsigned long long>(transfer->getMeanSpeed()));
        if (meanSpead == 0)
        {
            meanSpead = transferredBytes;
        }
        // Time is in decisecs.
        auto  finishTime (QDateTime::currentSecsSinceEpoch());
        finishTime += (transfer->getUpdateTime() - transfer->getStartTime()) / 10;

        transferItem.updateValuesTransferFinished( finishTime, errorCode, errorValue,
                                                    meanSpead, state, transferredBytes,
                                                    transfer->getParentHandle(),
                                                    transfer->getNodeHandle());
        if(transferItem.getTransferData()->mFilename.isEmpty())
        {
            auto a = 10;
        }

        v = QVariant::fromValue(transferItem);

        if (state == TransferData::TRANSFER_FAILED)
        {
            mFailedTransfers[tag] = std::move(transfer);
        }

        // Keep statistics up to date
        if (prevState != state)
        {
            mNbTransfersPerState[prevState]--;
            mNbTransfersPerState[state]++;

            auto type ((d->mType & TransferData::TRANSFER_UPLOAD) ?
                           TransferData::TRANSFER_UPLOAD
                         : TransferData::TRANSFER_DOWNLOAD);
            mNbTransfersPerType[type]--;
            mNbFinishedPerFileType[d->mFileType]++;
        }

        auto rem (mRemainingTimes.take(tag));
        if (rem)
        {
            delete rem;
        }

        QModelIndex idx (index(row, 0, DEFAULT_IDX));
        emit dataChanged(idx, idx, DATA_ROLE);
    }
    mModelMutex->unlock();
}

void QTransfersModel::transferTemporaryError(mega::MegaTransfer *transfer, mega::MegaError *error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lockForWrite();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());

        auto transferItem (static_cast<TransferItem*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        auto speed (static_cast<unsigned long long>(transfer->getSpeed()));
        auto totalBytes (static_cast<unsigned long long>(transfer->getTotalBytes()));
        auto transferredBytes (static_cast<unsigned long long>(transfer->getTransferredBytes()));
        auto rem (mRemainingTimes[transfer->getTag()]);
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes - transferredBytes));

        int errorCode (MegaError::API_OK);
        auto errorValue (0LL);
        if (error)
        {
            errorCode = error->getErrorCode();
            errorValue = error->getValue();
        }

        TransferData::TransferState state (
                    static_cast<TransferData::TransferState>(1 << transfer->getState()));
        auto prevState = d->mState;

        transferItem->updateValuesTransferUpdated(remSecs.count(), errorCode, errorValue,
                                                  static_cast<unsigned long long>(transfer->getMeanSpeed()),
                                                  speed,
                                                  transfer->getPriority(), state,
                                                  transferredBytes);

        QModelIndex idx (index(row, 0, DEFAULT_IDX));
        emit dataChanged(idx, idx, DATA_ROLE);

        // Keep statistics up to date
        if (prevState != state)
        {
            mNbTransfersPerState[prevState]--;
            mNbTransfersPerState[state]++;
        }
    }
    mModelMutex->unlock();
}

bool QTransfersModel::areAllPaused()
{
    return mAreAllPaused;
}

void QTransfersModel::getLinks(QList<int>& rows)
{
    if (!rows.isEmpty())
    {
        QList<MegaHandle> exportList;
        QStringList linkList;
        QList<TransferTag> tags;

        mModelMutex->lockForRead();
        for (auto row : rows)
        {
            tags.push_back(mOrder.at(row));
        }
        mModelMutex->unlock();

        for (auto tag : tags)
        {
            auto transferItem (static_cast<const TransferItem*>(mTransfers[tag].constData()));
            const auto d (transferItem->getTransferData());

            MegaNode *node (nullptr);

            if (d->mState == TransferData::TRANSFER_FAILED)
            {
                node = mFailedTransfers[tag]->getPublicMegaNode();
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

void QTransfersModel::openFolderByIndex(const QModelIndex& index)
{
    QtConcurrent::run([=]
    {
        const auto transferItem (
                    qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
        auto d (transferItem.getTransferData());
        if (d && !d->mPath.isEmpty())
        {
            QString localPath = d->mPath;
            #ifdef WIN32
            if (localPath.startsWith(QString::fromAscii("\\\\?\\")))
            {
                localPath = localPath.mid(4);
            }
            #endif
            Platform::showInFolder(localPath);
        }
    });
}

void QTransfersModel::cancelClearTransfers(const QModelIndexList& indexes, bool cancel, bool clear)
{
    QMap<int, TransferTag> tags;
    QVector<int> rows;
    QVector<int> toCancel;
    QVector<int>& rowsToCancel (clear? toCancel : rows);

    mModelMutex->lockForWrite();

    // Get rows from the indexes.
    for (auto index : indexes)
    {
        auto row (index.row());
        rows.push_back(row);
        tags[row] = static_cast<TransferTag>(index.internalId());
    }

    if (clear)
    {
        // Reverse sort to keep indexes valid after deletion
        std::sort(rows.rbegin(), rows.rend());

        // First clear finished transfers (remove rows), then cancel the others.
        // This way, there is no risk of messing up the rows order with cancel requests.
        int count (0);
        int row (mOrder.size() - 1);
        for (auto item : rows)
        {
            auto tag (tags[item]);
            const auto d (static_cast<const TransferItem*>(mTransfers[tag]
                                                            .constData())->getTransferData());

            // Clear (remove rows of) finished transfers
            if (d && d->mState & FINISHED_STATES)
            {
                // Init row with row of first tag
                if (count == 0)
                {
                    row = item;
                }

                // If rows are non-contiguous, flush and start from item
                if (row != item)
                {
                    removeRows(row + 1, count, DEFAULT_IDX);
                    count = 0;
                    row = item;
                }

                // We have at least one row
                count++;
                row--;
            }
            else
            {
                // Flush pooled rows (start at row+1)
                // Not strictly necessary, but allows to process rows sooner
                if (count > 0)
                {
                    removeRows(row + 1, count, DEFAULT_IDX);
                    count = 0;
                }
                // Queue transfer to be canceled (if needed)
                if (cancel && d->mState & CANCELABLE_STATES)
                {
                    toCancel.push_back(item);
                }
            }
        }
        // Flush pooled rows (start at row + 1).
        // This happens when the last item processed is in a finished state.
        if (count > 0)
        {
            removeRows(row + 1, count, DEFAULT_IDX);
        }
    }
    mModelMutex->unlock();

    // Now cancel transfers.
    if (cancel)
    {
        QReadLocker lock (mModelMutex);
        for (auto item : rowsToCancel)
        {
            // If we cleared before, all transfers are cancelable
            if (clear)
            {
                mMegaApi->cancelTransferByTag(tags[item]);
            }
            // If not, check before canceling
            else
            {
                auto tag (tags[item]);
                const auto d (static_cast<const TransferItem*>(mTransfers[tag]
                                                                .constData())->getTransferData());
                if (d->mState & CANCELABLE_STATES)
                {
                    d->mMegaApi->cancelTransferByTag(tag);
                }
            }
        }
    }

    //Update stats
    resetTransfersCount();
}

void QTransfersModel::pauseTransfers(const QModelIndexList& indexes, bool pauseState)
{
    QReadLocker lock (mModelMutex);

    for (auto index : indexes)
    {
        TransferTag tag (static_cast<TransferTag>(index.internalId()));
        pauseResumeTransferByTag(tag, pauseState);
    }

    if (!pauseState && mAreAllPaused)
    {
        mAreAllPaused = false;
        mMegaApi->pauseTransfers(false);
        emit pauseStateChanged(false);
    }
}

void QTransfersModel::pauseResumeAllTransfers()
{
    bool newPauseState (!mAreAllPaused);
    mAreAllPaused = newPauseState;

    mThreadPool->push([=]
    //QtConcurrent::run([=]
    {
        // First lock the sdk to avoid new callbacks
       std::unique_ptr<mega::MegaApiLock> megaApiLock (mMegaApi->getMegaApiLock(true));
       // Process remaining events
        //qApp->processEvents();

        QList<TransferTag> orderCopy;
        mModelMutex->lockForRead();
        orderCopy = mOrder;
        mModelMutex->unlock();
        megaApiLock->unlockOnce();

        if (newPauseState)
        {
            mMegaApi->pauseTransfers(newPauseState);
            std::for_each(orderCopy.crbegin(), orderCopy.crend(), [this, newPauseState](TransferTag tag)
            {
                pauseResumeTransferByTag(tag, newPauseState);
            });
        }
        else
        {
            std::for_each(orderCopy.cbegin(), orderCopy.cend(), [this, newPauseState](TransferTag tag)
            {
                pauseResumeTransferByTag(tag, newPauseState);
            });
            mMegaApi->pauseTransfers(newPauseState);
        }
    });
    emit pauseStateChanged(mAreAllPaused);
}

void QTransfersModel::pauseResumeTransferByTag(TransferTag tag, bool pauseState)
{
    const auto d (static_cast<const TransferItem*>(mTransfers[tag]
                                                    .constData())->getTransferData());

    auto state (d->mState);

    if ((!pauseState && (state == TransferData::TRANSFER_PAUSED))
            || (pauseState && (state & PAUSABLE_STATES)))
    {
        d->mMegaApi->pauseTransferByTag(d->mTag, pauseState);
    }
}

void QTransfersModel::cancelClearAllTransfers()
{
    mThreadPool->push([=]
    {
       mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
       mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
    });
}

void QTransfersModel::lockModelMutex(bool lock)
{
//    static std::unique_ptr<mega::MegaApiLock> megaApiLock (mMegaApi->getMegaApiLock(false));

    if (lock)
    {
//        megaApiLock->lockOnce();
        mModelMutex->lockForRead();
    }
    else
    {
        mModelMutex->unlock();
//        megaApiLock->unlockOnce();
    }
}

long long QTransfersModel::getNumberOfTransfersForState(TransferData::TransferState state) const
{
    return mNbTransfersPerState[state];
}

long long QTransfersModel::getNumberOfTransfersForType(TransferData::TransferType type) const
{
    return mNbTransfersPerType[type];
}

long long QTransfersModel::getNumberOfTransfersForFileType(TransferData::FileType fileType) const
{
    return mNbTransfersPerFileType[fileType];
}

long long QTransfersModel::getNumberOfFinishedForFileType(TransferData::FileType fileType) const
{
    return mNbFinishedPerFileType[fileType];
}

void QTransfersModel::updateTransfersCount()
{
    mTransfersCount.remainingUploads = mMegaApi->getNumPendingUploads();
    mTransfersCount.remainingDownloads = mMegaApi->getNumPendingDownloads();
    mTransfersCount.totalUploads = mMegaApi->getTotalUploads();
    mTransfersCount.totalDownloads = mMegaApi->getTotalDownloads();

    mTransfersCount.completedDownloadBytes = mMegaApi->getTotalDownloadedBytes();
    mTransfersCount.leftDownloadBytes = mMegaApi->getTotalDownloadBytes();

    mTransfersCount.completedUploadBytes = mMegaApi->getTotalUploadedBytes();
    mTransfersCount.leftUploadBytes = mMegaApi->getTotalUploadBytes();

    mTransfersCount.currentDownload = mTransfersCount.totalDownloads - mTransfersCount.remainingDownloads + 1;
    mTransfersCount.currentUpload = mTransfersCount.totalUploads - mTransfersCount.remainingUploads + 1;
}

const TransfersCount &QTransfersModel::getTransfersCount()
{
    return mTransfersCount;
}

void QTransfersModel::resetTransfersCount()
{
    updateTransfersCount();

    if(mTransfersCount.remainingDownloads == 0)
    {
        mMegaApi->resetTotalDownloads();
    }

    if(mTransfersCount.remainingUploads == 0)
    {
        mMegaApi->resetTotalUploads();
    }

    mTransfersCount = TransfersCount();

    emit transfersDataUpdated();
}

void QTransfersModel::onPauseStateChanged()
{
    bool newPauseState (mPreferences->getGlobalPaused());
    if (newPauseState != mAreAllPaused)
    {
        pauseResumeAllTransfers();
    }
}

void QTransfersModel::onRetryTransfer(TransferTag tag)
{
    QReadLocker lock (mModelMutex);
    auto transferIt (mFailedTransfers.find(tag));
    if (transferIt != mFailedTransfers.end())
    {
        const auto& transfer (transferIt->second);
        if (transfer)
        {
            mMegaApi->retryTransfer(transfer.get());
            auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
            auto row (rowIt - mOrder.cbegin());
            lock.unlock();
            removeRows(row, 1, DEFAULT_IDX);
        }
    }
}

bool QTransfersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent == DEFAULT_IDX && count > 0 && row >= 0)
    {
        mModelMutex->lockForWrite();

        beginRemoveRows(DEFAULT_IDX, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            TransferTag tag (mOrder[row]);
            auto transferItem (qvariant_cast<TransferItem>(mTransfers.take(tag)));
            auto d (transferItem.getTransferData());

            // Keep statistics updated
            auto state(d->mState);
            auto fileType(d->mFileType);

            mNbTransfersPerState[state]--;
            mNbTransfersPerFileType[fileType]--;
            mNbFinishedPerFileType[fileType]--;

            if (!(state & FINISHED_STATES))
            {
                auto type ((d->mType & TransferData::TRANSFER_UPLOAD) ?
                               TransferData::TRANSFER_UPLOAD
                             : TransferData::TRANSFER_DOWNLOAD);
                mNbTransfersPerType[type]--;
            }

            if (state == TransferData::TRANSFER_FAILED)
            {
                mFailedTransfers.erase(tag);
            }

            auto rem (mRemainingTimes.take(tag));
            if (rem)
            {
                delete rem;
            }

            mOrder.erase(mOrder.begin() + row);
        }
        endRemoveRows();

        if (mOrder.empty())
        {
            mModelHasTransfers = false;
        }
        mModelMutex->unlock();
        return true;
    }
    else
    {
        return false;
    }
}

bool QTransfersModel::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
                                const QModelIndex &destinationParent, int destinationChild)
{
    int lastRow (sourceRow + count - 1);

    if (sourceParent == destinationParent
            && (destinationChild < sourceRow || destinationChild > lastRow))
    {
        // To keep order, do from first to last if destination is before first,
        // and from last to first if destination is after last.
        bool ascending (destinationChild < sourceRow ? false : true);

        QList<TransferTag> tagsToMove;

        mModelMutex->lockForRead();
        auto rowCount (mOrder.size());

        for (auto row (sourceRow); row <= lastRow; ++row)
        {
            if (ascending)
            {
                tagsToMove.push_back(mOrder.at(row));
            }
            else
            {
                tagsToMove.push_front(mOrder.at(row));
            }
        }       

        for (auto tag : tagsToMove)
        {
            const auto d (static_cast<const TransferItem*>(mTransfers[tag].constData())
                          ->getTransferData());
            if (destinationChild == 0)
            {
                d->mMegaApi->moveTransferToFirstByTag(d->mTag);
            }
            else if (destinationChild == rowCount)
            {
                d->mMegaApi->moveTransferToLastByTag(d->mTag);
            }
            else
            {
                // Get target
                auto target (mOrder.at(destinationChild));
                d->mMegaApi->moveTransferBeforeByTag(d->mTag, target);
            }
        }
        mModelMutex->unlock();
        return true;
    }
    return false;
}

//Qt::ItemFlags QTransfersModel::flags(const QModelIndex& index) const
//{
//    Qt::ItemFlags flags (QAbstractItemModel::flags(index));
//    if (index.isValid())
//    {
//        mModelMutex->lock();
//        TransferTag tag (static_cast<TransferTag>(index.internalId()));
//        const auto d (static_cast<const TransferItem*>(mTransfers[tag]
//                          .constData())->getTransferData());
//        if (d)
//        {
//            const auto state (d->mState);

//            if ((state == MegaTransfer::STATE_QUEUED
//                 || state == MegaTransfer::STATE_ACTIVE
//                 || state == MegaTransfer::STATE_PAUSED
//                 || state == MegaTransfer::STATE_RETRYING))
//            {
//                flags |= Qt::ItemIsDragEnabled;
//            }
//        }
//        mModelMutex->unlock();
//    }
//    else
//    {
//        flags |= Qt::ItemIsDropEnabled;
//    }

//    return flags;
//}

Qt::ItemFlags QTransfersModel::flags(const QModelIndex& index) const
{
    if (index.isValid())
    {
        return QAbstractItemModel::flags(index) | Qt::ItemIsDragEnabled;
    }
    return QAbstractItemModel::flags(index) | Qt::ItemIsDropEnabled;
}

Qt::DropActions QTransfersModel::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* QTransfersModel::mimeData(const QModelIndexList& indexes) const
{
    QByteArray byteArray;
    QDataStream stream (&byteArray, QIODevice::WriteOnly);
    QList<TransferTag> tags;

    for (auto index : indexes)
    {
        tags.push_back(static_cast<TransferTag>(index.internalId()));
    }

    stream << tags;

    QMimeData* data = new QMimeData();
    data->setData(QString::fromUtf8("application/x-qabstractitemmodeldatalist"), byteArray);

    return data;
}

bool QTransfersModel::dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                    int column, const QModelIndex& parent)
{
    Q_UNUSED(column)
    QByteArray byteArray (data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist")));
    QDataStream stream (&byteArray, QIODevice::ReadOnly);
    QList<TransferTag> tags;
    stream >> tags;

    mModelMutex->lockForRead();

    if (destRow >= 0 && destRow <= mOrder.size() && action == Qt::MoveAction)
    {
        QList<int> rows;
        for (auto tag : qAsConst(tags))
        {
            auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
            rows.push_back(rowIt - mOrder.cbegin());
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

    mModelMutex->unlock();

    // Return false to avoid row deletion...dirty!
    return false;
}

void QTransfersModel::insertTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer, int row, bool signal)
{
    auto tag (transfer->getTag());
    QWriteLocker lock (mModelMutex);
//    if (mTransfers.find(tag) == mTransfers.end())
//    {
        auto state (static_cast<TransferData::TransferState>(1 << transfer->getState()));
        auto type (static_cast<TransferData::TransferType>(1 << transfer->getType()));
        auto fileName (QString::fromUtf8(transfer->getFileName()));
        auto path (QString::fromUtf8(transfer->getPath()));
        auto pixmapName (Utilities::getExtensionPixmapName(fileName, QString()));
        auto fileType (mFileTypes.contains(pixmapName) ?
                           mFileTypes[pixmapName]
                           : TransferData::FileType::TYPE_OTHER);
        auto speed (static_cast<unsigned long long>(api->getCurrentSpeed(transfer->getType())));
        auto totalBytes (static_cast<unsigned long long>(transfer->getTotalBytes()));
        auto transferredBytes (static_cast<unsigned long long>(transfer->getTransferredBytes()));
        auto remBytes (totalBytes - transferredBytes);
        auto rem (new TransferRemainingTime(speed, remBytes));
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, remBytes));
        auto priority (transfer->getPriority());
        int errorCode (MegaError::API_OK);
        auto errorValue (0LL);
        auto megaError (transfer->getLastErrorExtended());
        if (megaError)
        {
            errorCode = megaError->getErrorCode();
            errorValue = megaError->getValue();
        }

        TransferData::TransferTypes typePlus (type);
        if (transfer->isSyncTransfer())
        {
            typePlus |= TransferData::TRANSFER_SYNC;
        }

        QExplicitlySharedDataPointer<TransferData> d (new TransferData(
                             typePlus,
                             errorCode, state, tag, errorValue, 0,
                             remSecs.count(),
                             totalBytes, priority, speed,
                             static_cast<unsigned long long>(transfer->getMeanSpeed()),
                             transferredBytes, fileType,
                             transfer->getParentHandle(), transfer->getNodeHandle(), api,
                             fileName, path));



        mTransfers[tag] = QVariant::fromValue(TransferItem(d));
        mRemainingTimes[tag] = rem;

        if(fileName.isEmpty())
        {
            auto a = 10;
        }

        mOrder.insert(mOrder.begin() + row, tag);

        // Update statistics
        mNbTransfersPerState[state]++;
        mNbTransfersPerFileType[fileType]++;
        mNbTransfersPerType[type]++;
        if (mOrder.size() == 1)
        {
            mModelHasTransfers = true;
        }
//    }
}
