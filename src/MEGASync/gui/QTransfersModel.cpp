#include "QTransfersModel.h"
#include "MegaApplication.h"
#include "Utilities.h"
#include "Platform.h"
#include "TransferItem.h"

#include <QSharedData>

#include <algorithm>

using namespace mega;

static const QVector<int> DATA_ROLE = {Qt::DisplayRole};
static const QModelIndex DEFAULT_IDX = QModelIndex();

QHash<QString, TransferData::FileType> QTransfersModel::mFileTypes;

const int MAX_UPDATE_TRANSFERS = 5000;

//LISTENER THREAD
TransferThread::TransferThread() :mCacheMutex (new QReadWriteLock(QReadWriteLock::Recursive))
{
}

std::list<MegaTransfer *> TransferThread::processUpdates()
{
    QWriteLocker lock(mCacheMutex);

    if(mCacheUpdateTransfers.size()!= 0)
    {
        if(mCacheUpdateTransfers.size() > MAX_UPDATE_TRANSFERS)
        {
            std::list<mega::MegaTransfer*> auxList;
            auto lastIt = mCacheUpdateTransfers.begin();
            std::advance(lastIt, MAX_UPDATE_TRANSFERS);
            auxList.splice(auxList.begin(), mCacheUpdateTransfers, mCacheUpdateTransfers.begin(), lastIt);

            return auxList;
        }
        else
        {
            auto auxList = mCacheUpdateTransfers;
            mCacheUpdateTransfers.clear();

            return auxList;
        }
    }

    return std::list<MegaTransfer*>();
}

void TransferThread::onTransferEvent(MegaTransfer *transfer)
{
    if (!transfer->isStreamingTransfer()
            && !transfer->isFolderTransfer())
    {
        QWriteLocker lock(mCacheMutex);
        auto found = std::find_if(mCacheUpdateTransfers.begin(), mCacheUpdateTransfers.end(),[&transfer](MegaTransfer* transferToCheck){
            return transfer->getTag() == transferToCheck->getTag();
        });

        if(found != mCacheUpdateTransfers.end())
        {
            if((*found)->getNotificationNumber() < transfer->getNotificationNumber())
            {
                delete (*found);
                (*found) = transfer->copy();
            }
        }
        else
        {
            mCacheUpdateTransfers.push_back(transfer->copy());
        }
    }
}

void TransferThread::onTransferStart(MegaApi *, MegaTransfer *transfer)
{
    onTransferEvent(transfer);
}

void TransferThread::onTransferUpdate(MegaApi *, MegaTransfer *transfer)
{
    onTransferEvent(transfer);
}

void TransferThread::onTransferFinish(MegaApi*, MegaTransfer *transfer, MegaError *)
{
    onTransferEvent(transfer);
}

void TransferThread::onTransferTemporaryError(MegaApi*, MegaTransfer *transfer, MegaError *)
{
    onTransferEvent(transfer);
}

QTransfersModel::QTransfersModel(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (MegaSyncApp->getMegaApi()),
    mPreferences (Preferences::instance()),
    mTransfers (),
    mOrder (),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mModelMutex (new QReadWriteLock(QReadWriteLock::Recursive)),
    mUpdateNotificationNumber (0),
    mNbTransfersPerFileType(),
    mNbFinishedPerFileType()
{
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

    qRegisterMetaType<QList<QPersistentModelIndex>>("QList<QPersistentModelIndex>");

    // Connect to pause state change signal
    QObject::connect((MegaApplication*)qApp, &MegaApplication::pauseStateChanged,
                      this, &QTransfersModel::onPauseStateChanged, Qt::QueuedConnection);

    mAreAllPaused = mPreferences->getGlobalPaused();

    mTransferEventThread = new QThread();
    mTransferEventWorker = new TransferThread();
    mTransferEventWorker->moveToThread(mTransferEventThread);
    mMegaApi->addTransferListener(mTransferEventWorker);

    //Update transfers state for the first time
    updateTransfersCount();

    mTimer.setInterval(200);
    QObject::connect(&mTimer, &QTimer::timeout, this, &QTransfersModel::onProcessTransfers);
    mTimer.start();

    mTransferEventThread->start();

}


QTransfersModel::~QTransfersModel()
{
    // Cleanup
    mTransfers.clear();

    // Disconect listener
    mMegaApi->removeTransferListener(mTransferEventWorker);
    mTransferEventThread->quit();
    mTransferEventThread->deleteLater();
    mTransferEventWorker->deleteLater();
}

void QTransfersModel::pauseModelProcessing(bool value)
{
    if(value)
    {
        mTimer.stop();
    }
    else
    {
        mTimer.start();
    }
}

bool QTransfersModel::hasChildren(const QModelIndex& parent) const
{
    if (parent == DEFAULT_IDX)
    {
        return !mOrder.empty();
    }
    return false;
}

int QTransfersModel::rowCount(const QModelIndex& parent) const
{
    int rowCount (0);
    if (parent == DEFAULT_IDX)
    {
        rowCount = mOrder.size();
    }
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

    if (role == Qt::DisplayRole && row >= 0 && row < mOrder.size())
    {
        auto tag (mOrder.at(row));
        value = QVariant::fromValue(TransferItem(mTransfers[tag]));
    }

    return value;
}

QModelIndex QTransfersModel::parent(const QModelIndex&) const
{
    return DEFAULT_IDX;
}

QModelIndex QTransfersModel::index(int row, int column, const QModelIndex& parent) const
{
    if (parent == DEFAULT_IDX && column == 0 && row >= 0 && row < mOrder.size())
    {
        auto tag (mOrder.at(row));
        return createIndex(row, 0, static_cast<quintptr>(tag));
    }
    return DEFAULT_IDX;
}

void QTransfersModel::initModel()
{
    emit pauseStateChanged(mAreAllPaused);
}

void QTransfersModel::onProcessTransfers()
{
    mRowsToUpdate.clear();

    auto updateList = mTransferEventWorker->processUpdates();

    std::list<mega::MegaTransfer*> newTransferList;
    std::list<mega::MegaTransfer*> transferToUpdateList;
    std::list<mega::MegaTransfer*> transferToCancelList;
    for (auto it = updateList.begin(); it != updateList.end(); ++it)
    {
        auto state (static_cast<TransferData::TransferState>(1 << (*it)->getState()));
        auto row = mTagByOrder.value((*it)->getTag(),-1);

        if(row >= 0)
        {
            if(state == TransferData::TransferState::TRANSFER_CANCELLED)
            {
                transferToCancelList.push_back((*it));
            }
            else
            {
                transferToUpdateList.push_back((*it));
            }
        }
        else
        {
            if(state == TransferData::TransferState::TRANSFER_CANCELLED)
            {
                continue;
            }
            else
            {
                newTransferList.push_back((*it));
            }
        }
    }

    if(transferToCancelList.size() != 0
            || newTransferList.size() != 0
            || transferToUpdateList.size() != 0)
    {
        processCancelTransfers(transferToCancelList);
        processStartTransfers(newTransferList);
        processUpdateTransfers(transferToUpdateList);

        if(!mRowsToUpdate.isEmpty())
        {
            foreach(auto& row, mRowsToUpdate)
            {
                QModelIndex topLeft (index(row, 0, DEFAULT_IDX));
                emit dataChanged(topLeft, topLeft);
            }

        }

        updateTransfersCount();


        newTransferList.clear();
        transferToUpdateList.clear();
        transferToCancelList.clear();
        updateList.clear();
    }
}

void QTransfersModel::processStartTransfers(std::list<MegaTransfer *> transferList)
{
    if (transferList.size() != 0)
    {
        auto totalRows = rowCount(DEFAULT_IDX);
        auto rowsToBeInserted(static_cast<int>(transferList.size()));

        beginInsertRows(DEFAULT_IDX, totalRows, totalRows + rowsToBeInserted);

        auto counter(totalRows);
        for (auto it = transferList.begin(); it != transferList.end();)
        {
           startTransfer((*it));
           delete (*it);

           if(mRowsToUpdate.contains(counter))
           {
               mRowsToUpdate.append(counter);
           }
           counter++;
           it++;
        }

        endInsertRows();

        mRowsToUpdate.append(totalRows);
        mRowsToUpdate.append(totalRows + rowsToBeInserted - 1);
    }
}

void QTransfersModel::startTransfer(mega::MegaTransfer* transfer)
{
    insertTransfer(transfer);

    auto state (static_cast<TransferData::TransferState>(1 << transfer->getState()));
    auto tag = transfer->getTag();

    mThreadPool->push([=]
    {
        if (mAreAllPaused && (state & TransferData::PAUSABLE_STATES_MASK))
        {
            mMegaApi->pauseTransferByTag(tag, true);
        }
    });
}

void QTransfersModel::insertTransfer(mega::MegaTransfer* transfer)
{
    auto tag (transfer->getTag());

    QExplicitlySharedDataPointer<TransferData> d (new TransferData(
                         transfer));
    d->update(transfer);

    mOrder.append(tag);
    mTagByOrder.insert(tag, mOrder.size() - 1);
    mTransfers[tag] = d;
    mNbTransfersPerFileType[d->mFileType]++;
}

void QTransfersModel::processUpdateTransfers(std::list<MegaTransfer *> transferList)
{
  for (auto it = transferList.begin(); it != transferList.end();)
  {
      auto row = updateTransfer((*it));
      if(row >= 0 && !mRowsToUpdate.contains(row))
      {
          mRowsToUpdate.append(row);
      }

      delete (*it);
      it++;
  }
}

void QTransfersModel::processCancelTransfers(std::list<MegaTransfer *> transferList)
{
    if(transferList.size() > 0)
    {
        QModelIndexList indexesToCancel;

        for (auto it = transferList.begin(); it != transferList.end();)
        {
            auto row = mTagByOrder.value((*it)->getTag());
            indexesToCancel.append(index(row,0, DEFAULT_IDX));

            delete (*it);
            it++;
        }

        qSort(indexesToCancel.begin(), indexesToCancel.end(), qGreater<QModelIndex>());

        cancelClearTransfers(indexesToCancel, false);
    }
}

int QTransfersModel::updateTransfer(mega::MegaTransfer* transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer())
    {
        return - 1;
    }

    TransferTag tag (transfer->getTag());

    auto d  = mTransfers.value(tag);
    if(d)
    {
        auto prevState (d->mState);
        d->update(transfer);

        if(d->mState & TransferData::FINISHED_STATES_MASK)
        {
            // Keep statistics up to date
            if (prevState != d->mState)
            {
                mNbFinishedPerFileType[d->mFileType]++;
            }
        }
    }

    return mTagByOrder.value(tag);
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

        for (auto row : rows)
        {
            auto tag = mOrder.at(row);
            auto d = mTransfers[tag];
            tags.push_back(d->mTag);
        }

        for (auto tag : tags)
        {
            auto d (mTransfers[tag]);

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

void QTransfersModel::openFolderByIndex(const QModelIndex& index)
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

void QTransfersModel::cancelClearTransfers(const QModelIndexList& indexes, bool clearAll)
{
    QMap<int, TransferTag> tags;
    QVector<TransferTag> rows;
    QVector<TransferTag> toCancel;
    QVector<TransferTag> uploadToClear;
    QVector<TransferTag> downloadToClear;

    // Get rows from the indexes.
    for (auto index : indexes)
    {
        auto row (index.row());
        rows.push_back(row);
        tags[row] = static_cast<TransferTag>(index.internalId());
    }

    // Reverse sort to keep indexes valid after deletion
    std::sort(rows.rbegin(), rows.rend());

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (mOrder.size() - 1);
    for (auto item : rows)
    {
        auto tag (tags[item]);
        auto d (mTransfers[tag]);

        // Clear (remove rows of) finished transfers
        if (d)
        {
            if (d->mState & TransferData::CANCELABLE_STATES_MASK)
            {
                toCancel.push_back(d->mTag);
            }
            else if(!clearAll && d->mState & TransferData::FINISHED_STATES_MASK)
            {
                if(d->mType & TransferData::TransferType::TRANSFER_UPLOAD)
                {
                    uploadToClear.push_back(d->mTag);
                }
                else
                {
                    downloadToClear.push_back(d->mTag);
                }
            }

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
    }
    // Flush pooled rows (start at row + 1).
    // This happens when the last item processed is in a finished state.
    if (count > 0)
    {
        removeRows(row + 1, count, DEFAULT_IDX);
    }

    mThreadPool->push([=]
    {
        // Now cancel transfers.
        for (auto item : toCancel)
        {
            mMegaApi->cancelTransferByTag(item);
        }
    });

    if(clearAll)
    {
        mMegaApi->resetCompletedDownloads();
        mMegaApi->resetCompletedUploads();
    }
    else
    {
        for (auto item : uploadToClear)
        {
            mMegaApi->removeCompletedUpload(item);
        }

        for (auto item : downloadToClear)
        {
            mMegaApi->removeCompletedDownload(item);
        }
    }

    //Update stats
    updateTransfersCount();
}

void QTransfersModel::pauseTransfers(const QModelIndexList& indexes, bool pauseState)
{
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
    {
        // First lock the sdk to avoid new callbacks
       std::unique_ptr<mega::MegaApiLock> megaApiLock (mMegaApi->getMegaApiLock(true));

       QList<TransferTag> orderCopy;
       orderCopy = mOrder;
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
    mThreadPool->push([=]
    {
        auto d (mTransfers[tag]);
        auto state (d->mState);

        if ((!pauseState && (state == TransferData::TRANSFER_PAUSED))
                || (pauseState && (state & TransferData::PAUSABLE_STATES_MASK)))
        {
           mMegaApi->pauseTransferByTag(d->mTag, pauseState);
        }
    });
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
    if (lock)
    {
        mModelMutex->lockForRead();
    }
    else
    {
        mModelMutex->unlock();
    }
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

    mTransfersCount.completedDownloads = mMegaApi->getCompletedDownloads();
    mTransfersCount.completedUploads = mMegaApi->getCompletedUploads();

    mTransfersCount.completedDownloadBytes = mMegaApi->getTotalDownloadedBytes();
    mTransfersCount.leftDownloadBytes = mMegaApi->getTotalDownloadBytes();

    mTransfersCount.completedUploadBytes = mMegaApi->getTotalUploadedBytes();
    mTransfersCount.leftUploadBytes = mMegaApi->getTotalUploadBytes();

    mTransfersCount.currentDownload = mTransfersCount.totalDownloads - mTransfersCount.remainingDownloads + 1;
    mTransfersCount.currentUpload = mTransfersCount.totalUploads - mTransfersCount.remainingUploads + 1;

    emit transfersDataUpdated();
}

const TransfersCount &QTransfersModel::getTransfersCount()
{
    return mTransfersCount;
}

void QTransfersModel::resetCompletedTransfersCount()
{
    mMegaApi->resetCompletedDownloads();
    mMegaApi->resetCompletedUploads();

    updateTransfersCount();
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
    mThreadPool->push([=]
    {
        auto transfer = mMegaApi->getTransferByTag(tag);

        if (transfer)
        {
            mMegaApi->retryTransfer(transfer);
            //auto row = mTagByOrder.value(tag);
            //removeRows(row, 1, DEFAULT_IDX);
        }
    });
}

bool QTransfersModel::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent == DEFAULT_IDX && count > 0 && row >= 0)
    {
        QWriteLocker lock(mModelMutex);

        beginRemoveRows(DEFAULT_IDX, row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            TransferTag tag = mOrder[row];
            auto d = mTransfers.value(tag);

            // Keep statistics updated
            auto fileType(d->mFileType);

            mNbTransfersPerFileType[fileType]--;
            mNbFinishedPerFileType[fileType]--;

            mOrder.removeAt(row);
            mTransfers.remove(tag);
        }
        endRemoveRows();

        mTagByOrder.clear();
        //Recalculate rest of items
        for(int row = 0; row < rowCount(DEFAULT_IDX); ++row)
        {
            auto idx = index(row,0, DEFAULT_IDX);
            auto tag = quintptr(idx.internalPointer());
            mTagByOrder.insert(tag, row);
        }
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
            auto d (mTransfers[tag]);
            if (destinationChild == 0)
            {
                mMegaApi->moveTransferToFirstByTag(d->mTag);
            }
            else if (destinationChild == rowCount)
            {
                mMegaApi->moveTransferToLastByTag(d->mTag);
            }
            else
            {
                // Get target
                auto target (mOrder.at(destinationChild));

                mMegaApi->moveTransferBeforeByTag(d->mTag, target);
            }
        }

        return true;
    }
    return false;
}

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

    if (destRow >= 0 && destRow <= mOrder.size() && action == Qt::MoveAction)
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
