#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "Utilities.h"



using namespace mega;

const int QTransfersModel2::INIT_ROWS_PER_CHUNK;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel (parent),
    mMegaApi (((MegaApplication *)qApp)->getMegaApi()),
    mPreferences (Preferences::instance()),
    mTransfers (),
    mRemainingTimes (),
    mOrder (),
    mThreadPool (ThreadPoolSingleton::getInstance()),
    mModelMutex (new QMutex(QMutex::Recursive)),
    mNotificationNumber (0),
    mModelHasTransfers (false)
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
                      this, &QTransfersModel2::onPauseStateChanged);

    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
    mAreAllPaused = mPreferences->getGlobalPaused();

    qRegisterMetaType<TransferData::FileTypes>("TransferData::FileTypes");

    mInitFuture = QtConcurrent::run(this, &QTransfersModel2::initModel);

    // Connect to transfer changes signals
    mMegaApi->addTransferListener(this);
}

bool QTransfersModel2::hasChildren(const QModelIndex& parent) const
{
    if (parent == QModelIndex())
    {
        return mModelHasTransfers;
    }

    return false;
}

int QTransfersModel2::rowCount(const QModelIndex& parent) const
{
    QMutexLocker lock (mModelMutex);
    if (parent == QModelIndex())
    {
        return mOrder.size();
    }
    return 0;
}

int QTransfersModel2::columnCount(const QModelIndex& parent) const
{
    return 1;
}

QVariant QTransfersModel2::data(const QModelIndex& index, int role) const
{
    int row (index.row());
    bool isIndexValid (index.isValid());

    QMutexLocker lock (mModelMutex);

    if (role == Qt::DisplayRole && isIndexValid && row >= 0 && row < mOrder.size())
    {
        return mTransfers[mOrder[row]];
    }

    return QVariant();
}

QModelIndex QTransfersModel2::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel2::index(int row, int column, const QModelIndex& parent) const
{
    QMutexLocker lock (mModelMutex);

    if (column == 0 && row >= 0 && row < mOrder.size())
    {
        return createIndex(row, 0, mOrder.at(row));
    }

    return QModelIndex();
}

QTransfersModel2::~QTransfersModel2()
{
    mMegaApi->removeTransferListener(this);

    mInitFuture.cancel();
    mInitFuture.waitForFinished();

    mModelMutex->lock();
    for (auto transfer : qAsConst(mTransfers))
    {
        auto transferItem (static_cast<TransferItem2*>(transfer.data()));
        auto d (transferItem->getTransferData());

        if (d && d->mPublicNode)
        {
            delete d->mPublicNode;
        }
    }
    qDeleteAll(mFailedTransfers);
    qDeleteAll(mRemainingTimes);
    mModelMutex->unlock();
    delete mModelMutex;
}

QExplicitlySharedDataPointer<TransferData> QTransfersModel2::getTransferDataByRow(int row) const
{
    QExplicitlySharedDataPointer<TransferData> transferData;
    mModelMutex->lock();

    transferData = static_cast<const TransferItem2*>(mTransfers[mOrder[row]]
                   .constData())->getTransferData();
    mModelMutex->unlock();

    return transferData;
}

void QTransfersModel2::initModel()
{
    mega::MegaApiLock* megaApiLock (mMegaApi->getMegaApiLock(true));

    const auto transfers (mMegaApi->getTransfers());
    const auto transferData (mMegaApi->getTransferData());
    mNotificationNumber = transferData->getNotificationNumber();

    // First, list all the transfers to add
    QList<TransferTag> transfersToAdd;

    for (auto i (0); i < transfers->size(); ++i)
    {
        mega::MegaTransfer* mt (transfers->get(i));
        auto type (mt->getType());
        auto priority (mt->getPriority());
        if (!mt->isStreamingTransfer()
                && !mt->isFolderTransfer()
                && priority)
        {
            // Sort transfers
            if (transfersToAdd.isEmpty())
            {
                transfersToAdd.push_back(i);
            }
            else
            {
                auto other(transfers->get(transfersToAdd.first()));
                if (type == other->getType() && priority < other->getPriority())
                {
                    transfersToAdd.push_front(i);
                }
                else
                {
                    // Start from the back
                    auto otherTag (transfersToAdd.rbegin());
                    bool found (false);
                    int insertAt (transfersToAdd.size());
                    while (!found && otherTag != transfersToAdd.rend())
                    {
                        other = transfers->get(*otherTag);
                        if (type == other->getType()
                                && priority > other->getPriority())
                        {
                            found = true;
                        }
                        else
                        {
                            otherTag++;
                            insertAt--;
                        }
                    }
                    transfersToAdd.insert(insertAt, i);
                }
            }
        }
    }
    // Then load them in the model by chunks
    const auto nbRows (transfersToAdd.size());
    if (nbRows > 0)
    {
        auto remainingRows (nbRows);

        // Load in chunks for responsiveness
        auto nbChunks (nbRows / INIT_ROWS_PER_CHUNK);

        // Add 1 chunk more if needed
        if ((nbChunks * INIT_ROWS_PER_CHUNK) < nbRows)
        {
            nbChunks++;
        }

        for (auto chunk (0); chunk < nbChunks; ++chunk)
        {
            auto first (nbRows - remainingRows);
            auto nbRowsInChunk (std::min(remainingRows, INIT_ROWS_PER_CHUNK));

            mModelMutex->lock();
            // Use the actual number of items to update the rows, in case
            // transfers were added/removed between chunks.
            auto nbRowsInModel (mOrder.size());
            beginInsertRows(QModelIndex(), nbRowsInModel,
                            nbRowsInModel + nbRowsInChunk - 1);

            // Insert transfers
            for (auto row (first); row < first + nbRowsInChunk; ++row)
            {
                insertTransfer(mMegaApi, transfers->get(transfersToAdd[row]), row);
            }

            mModelMutex->unlock();

            endInsertRows();
            remainingRows -= INIT_ROWS_PER_CHUNK;
        }
    }
    delete transferData;
    delete transfers;

    megaApiLock->unlockOnce();
    delete megaApiLock;
}

void QTransfersModel2::onTransferStart(mega::MegaApi* api, mega::MegaTransfer* transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    mModelMutex->lock();
    mNotificationNumber = transfer->getNotificationNumber();

    auto nbRows (mOrder.size());
    auto insertAt (0);
    // Find place
    if (nbRows > 0)
    {
        bool found (false);
        auto priority (transfer->getPriority());
        auto type (transfer->getType());

        // Find first of same type if any.
        if (mNbTransfersPerType[type] > 0)
        {
            auto tagOther (mOrder.begin());
            auto dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                          .constData())->getTransferData();
            while (type != dOther->mType && insertAt < (nbRows - 1))
            {
                insertAt++;
                tagOther++;
                dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                         .constData())->getTransferData();
            }
            // Check if we have found the spot
            if (type == dOther->mType && priority < dOther->mPriority)
            {
                found = true;
            }
        }
        // Put at the end.
        else
        {
            insertAt = nbRows;
            found = true;
        }

        // Search from the end.
        if (!found)
        {
            insertAt = nbRows;
            auto tagOther (mOrder.rbegin());
            while (!found && tagOther != mOrder.rend())
            {
                auto dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                              .constData())->getTransferData();
                if (type == dOther->mType && priority >= dOther->mPriority)
                {
                    found = true;
                }
                else
                {
                    tagOther++;
                    insertAt--;
                }
            }
        }
    }
    beginInsertRows(QModelIndex(), insertAt, insertAt);
    insertTransfer(api, transfer, insertAt);
    endInsertRows();

    mModelMutex->unlock();
}

void QTransfersModel2::onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer,
                                        mega::MegaError* error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lock();
    mNotificationNumber = transfer->getNotificationNumber();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());

        //auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
       // auto transferItem (qvariant_cast<TransferItem2>(mTransfers[tag]));
        QVariant& v (mTransfers[tag]);
        auto transferItem (v.value<TransferItem2>());

        auto d (transferItem.getTransferData());



        auto state (transfer->getState());
        auto prevState (d->mState);
        int  errorCode (MegaError::API_OK);
        auto errorValue (0LL);
        if (error)
        {
            errorCode = error->getErrorCode();
            errorValue = error->getValue();
        }
        auto transferredBytes (transfer->getTransferredBytes());
        auto meanSpead (transfer->getMeanSpeed());
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
                                                    transfer->getNodeHandle(),
                                                    transfer->getPublicMegaNode());

        v = QVariant::fromValue(transferItem);
        emit dataChanged(index(row, 0), index(row, 0));

        if (state == MegaTransfer::STATE_FAILED)
        {
            mFailedTransfers[tag] = transfer->copy();
        }

        // Keep statistics up to date
        if (prevState != state)
        {
            mNbTransfersPerState[prevState]--;
            mNbTransfersPerState[state]++;
            mNbTransfersPerType[d->mType]--;
        }

        auto rem (mRemainingTimes.take(tag));
        if (rem)
        {
            delete rem;
        }
    }
    else
    {
        onTransferStart(api, transfer);
    }
    mModelMutex->unlock();
}

void QTransfersModel2::onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lock();
    mNotificationNumber = transfer->getNotificationNumber();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));

    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());
        //        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        QVariant& v (mTransfers[tag]);
        auto transferItem (v.value<TransferItem2>());
        auto d (transferItem.getTransferData());

        auto transferredBytes (transfer->getTransferredBytes());
        auto state (transfer->getState());
        auto prevState = d->mState;
        auto priority (transfer->getPriority());
        auto prevPriority (d->mPriority);

        if (state != prevState
                || priority != prevPriority
                || transferredBytes >= d->mTransferredBytes)
        {
            auto speed (transfer->getSpeed());
            auto totalBytes (transfer->getTotalBytes());
            auto rem (mRemainingTimes[transfer->getTag()]);
            auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes - transferredBytes));

            transferItem.updateValuesTransferUpdated(remSecs.count(), 0, 0,
                                                      transfer->getMeanSpeed(), speed, priority,
                                                      state, transferredBytes,
                                                      transfer->getPublicMegaNode());
            v = QVariant::fromValue(transferItem);
            // Re-order if priority changed
            bool sameRow (true);
            if (priority != prevPriority)
            {
                bool found (false);
                auto type (d->mType);
                auto newRow (row);
                int destRowToPassToBeginMove (0);

                int lastRow (mOrder.size() - 1);

                // Search after if priority is higher
                if (priority > prevPriority && row < lastRow)
                {
                    // Test first if the transfer was sent to bottom
                    newRow = lastRow;
                    auto tagOther (mOrder.rbegin());
                    auto dOther (static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                 .constData())->getTransferData());
                    // Find last of same type
                    while (type != dOther->mType && newRow > row)
                    {
                        newRow--;
                        tagOther++;
                        dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                 .constData())->getTransferData();
                    }

                    // Check if found
                    if (type == dOther->mType && priority >= dOther->mPriority)
                    {
                        found = true;
                    }
                    // If not found, search from next row
                    else
                    {
                        newRow = row + 1;
                        auto tagOther (mOrder.begin() + (row + 1));
                        while (!found && newRow <= lastRow)
                        {
                            dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                     .constData())->getTransferData();
                            if (type == dOther->mType && priority < dOther->mPriority)
                            {
                                found = true;
                                newRow--;
                            }
                            else
                            {
                                newRow++;
                                tagOther++;
                            }
                        }
                    }
                    if (found)
                    {
                        destRowToPassToBeginMove = newRow + 1;
                    }
                }
                // Search before if priority is lower
                else if (priority < prevPriority && row > 0)
                {
                    // Test first if the transfer was sent to top
                    newRow = 0;
                    auto tagOther (mOrder.begin());
                    auto dOther (static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                 .constData())->getTransferData());
                    // Find the first of the same type.
                    while (type != dOther->mType && newRow < row)
                    {
                        newRow++;
                        tagOther++;
                        dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                 .constData())->getTransferData();
                    }
                    // Check if found (can be itself)
                    if (type == dOther->mType && priority <= dOther->mPriority)
                    {
                        found = true;
                    }
                    // If not found, search from previous row
                    else
                    {
                        newRow = row - 1;
                        auto tagOther (mOrder.begin() + (row - 1));
                        found = false;
                        while (!found && newRow >= 0)
                        {
                            dOther = static_cast<const TransferItem2*>(mTransfers[*tagOther]
                                     .constData())->getTransferData();
                            if (type == dOther->mType && priority > dOther->mPriority)
                            {
                                found = true;
                                newRow++;
                            }
                            else
                            {
                                newRow--;
                                tagOther--;
                            }
                        }
                    }
                    if (found)
                    {
                        destRowToPassToBeginMove = newRow;
                    }
                }

                if (newRow != row)
                {
                    if (beginMoveRows(QModelIndex(), row, row,
                                      QModelIndex(), destRowToPassToBeginMove))
                    {
                        mOrder.move(row, newRow);
//                        mOrder.erase(rowIt);
//                        mOrder.insert(mOrder.cbegin() + newRow, tag);
                        endMoveRows();
                        sameRow = false;
                    }
                }
            }

            if (sameRow)
            {
                emit dataChanged(index(row, 0), index(row, 0));
            }

            // Keep statistics up to date
            if (prevState != state)
            {
                mNbTransfersPerState[prevState]--;
                mNbTransfersPerState[state]++;

                if (transfer->isFinished())
                {
                    mNbTransfersPerType[d->mType]--;
                    mNbTransfersPerFileType[d->mFileType]--;
                }
            }
        }
    }
    else
    {
        onTransferStart(api, transfer);
    }
    mModelMutex->unlock();
}

void QTransfersModel2::onTransferTemporaryError(mega::MegaApi *api,mega::MegaTransfer *transfer,
                                                mega::MegaError* error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    TransferTag tag (transfer->getTag());

    mModelMutex->lock();
    mNotificationNumber = transfer->getNotificationNumber();

    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
    if (rowIt != mOrder.cend())
    {
        auto row (rowIt - mOrder.cbegin());

        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        auto speed (transfer->getSpeed());
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes (transfer->getTransferredBytes());
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

        auto state (transfer->getState());
        auto prevState (d->mState);

        transferItem->updateValuesTransferUpdated(remSecs.count(), errorCode, errorValue,
                                                  transfer->getMeanSpeed(), speed,
                                                  transfer->getPriority(), state, transferredBytes,
                                                  transfer->getPublicMegaNode());
        emit dataChanged(index(row, 0), index(row, 0));

        // Keep statistics up to date
        if (prevState != state)
        {
            mNbTransfersPerState[prevState]--;
            mNbTransfersPerState[state]++;

            if (transfer->isFinished())
            {
                mNbTransfersPerType[d->mType]--;
                mNbTransfersPerFileType[d->mFileType]--;
            }
        }
    }
    else
    {
        onTransferStart(api, transfer);
    }
    mModelMutex->unlock();
}

bool QTransfersModel2::areDlPaused()
{
    return mAreDlPaused;
}

bool QTransfersModel2::areUlPaused()
{
    return mAreUlPaused;
}

void QTransfersModel2::getLinks(QList<int>& rows)
{
    if (!rows.isEmpty())
    {
        QList<MegaHandle> exportList;
        QStringList linkList;
        QList<TransferTag> tags;

        mModelMutex->lock();
        for (auto row : rows)
        {
            tags.push_back(mOrder.at(row));
        }
        mModelMutex->unlock();

        for (auto tag : tags)
        {
            auto transferItem (static_cast<const TransferItem2*>(mTransfers[tag].constData()));
            const auto d (transferItem->getTransferData());

            MegaNode *node (nullptr);

            if (d->mState == MegaTransfer::STATE_FAILED)
            {
                node = mFailedTransfers[tag]->getPublicMegaNode();
            }
            else
            {
                node = d->mPublicNode;
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

void QTransfersModel2::cancelClearTransfers(QModelIndexList& indexes)
{
    QMap<int, TransferTag> tags;
    QList<int> rows;

    mModelMutex->lock();

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
        const auto d (static_cast<const TransferItem2*>(mTransfers[tag]
                                                        .constData())->getTransferData());
        if (d)
        {
            // Clear (remove rows of) finished transfers
            if ((d->mState == MegaTransfer::STATE_COMPLETED
                 || d->mState == MegaTransfer::STATE_CANCELLED
                 || d->mState == MegaTransfer::STATE_FAILED))
            {
                // Init row with row of first tag
                if (count == 0)
                {
                    row = mOrder.lastIndexOf(tag, row);
//                    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
//                    row = rowIt - mOrder.cbegin();
                }

                // Flush pooled rows (start at row+1), init row with tag's
                if (row == -1 || mOrder.at(row) != tag)
                {
                    removeRows(row + 1, count, QModelIndex());
                    count = 0;
                    row = mOrder.lastIndexOf(tag, row);
//                    auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
//                    row = rowIt - mOrder.cbegin();
                }
                count++;
                row--;
                rows.removeOne(item);
            }
            // Do not cancel/clear completing transfers.
            else if (d->mState == MegaTransfer::STATE_COMPLETING)
            {
                rows.removeOne(item);
            }
        }
    }

    // Flush pooled rows (start at row).
    if (count > 0)
    {
        removeRows(row + 1, count, QModelIndex());
    }

    mModelMutex->unlock();

    // All remaining tags, if any, are cancelable transfers. Cancel them.
    for (auto item : rows)
    {
        mMegaApi->cancelTransferByTag(tags[item]);
    }
}

void QTransfersModel2::pauseTransfers(QModelIndexList& indexes, bool pauseState)
{
    mModelMutex->lock();
    for (auto index : indexes)
    {
        TransferTag tag (static_cast<TransferTag>(index.internalId()));
        const auto d (static_cast<const TransferItem2*>(mTransfers[tag]
                                                        .constData())->getTransferData());
        if (d)
        {
            if ((!pauseState && (d->mState == MegaTransfer::STATE_PAUSED))
                    || (pauseState && (d->mState == MegaTransfer::STATE_ACTIVE
                                       || d->mState == MegaTransfer::STATE_QUEUED
                                       || d->mState == MegaTransfer::STATE_RETRYING)))
            {
                d->mMegaApi->pauseTransferByTag(d->mTag, pauseState);
            }
        }
    }
    mModelMutex->unlock();
}

void QTransfersModel2::pauseResumeAllTransfers()
{
    mMegaApi->pauseTransfers(!mAreAllPaused);
}

void QTransfersModel2::pauseResumeDownloads()
{
    auto pauseState (!mAreDlPaused);
    mMegaApi->pauseTransfers(pauseState, MegaTransfer::TYPE_DOWNLOAD);
    mMegaApi->pauseTransfers(pauseState, MegaTransfer::TYPE_LOCAL_HTTP_DOWNLOAD);
    mMegaApi->pauseTransfers(pauseState, MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD);
}

void QTransfersModel2::pauseResumeUploads()
{
    mMegaApi->pauseTransfers(!mAreUlPaused, MegaTransfer::TYPE_UPLOAD);
}

void QTransfersModel2::cancelAllTransfers()
{
    mMegaApi->cancelTransfers(MegaTransfer::TYPE_UPLOAD);
    mMegaApi->cancelTransfers(MegaTransfer::TYPE_DOWNLOAD);
}

long long QTransfersModel2::getNumberOfTransfersForState(int state)
{
    return mNbTransfersPerState[state];
}

long long QTransfersModel2::getNumberOfTransfersForType(int type)
{
    return mNbTransfersPerType[type];
}

long long QTransfersModel2::getNumberOfTransfersForFileType(TransferData::FileTypes fileType)
{
    return mNbTransfersPerFileType[fileType];
}

void QTransfersModel2::onPauseStateChanged()
{
    mAreDlPaused  = mPreferences->getDownloadsPaused();
    mAreUlPaused  = mPreferences->getUploadsPaused();
    mAreAllPaused = mPreferences->getGlobalPaused();

    emit dataChanged(index(0, 0), index(mOrder.size() - 1, 0), {Qt::DisplayRole});
}

void QTransfersModel2::onRetryTransfer(TransferTag tag)
{
    auto transfer (mFailedTransfers[tag]);
    if (transfer)
    {
        const auto transferItem (static_cast<const TransferItem2*>(mTransfers[tag].constData()));
        transferItem->getTransferData()->mMegaApi->retryTransfer(transfer);

        auto rowIt (std::find(mOrder.cbegin(), mOrder.cend(), tag));
        removeRows(rowIt - mOrder.cbegin(), 1, QModelIndex());
    }
}

bool QTransfersModel2::removeRows(int row, int count, const QModelIndex& parent)
{
    if (parent == QModelIndex() && count > 0 && row >= 0)
    {
        mModelMutex->lock();
        beginRemoveRows(QModelIndex(), row, row + count - 1);

        for (auto i (0); i < count; ++i)
        {
            TransferTag tag (mOrder[row]);
            auto transferItem (qvariant_cast<TransferItem2>(mTransfers.take(tag)));
            auto d (transferItem.getTransferData());

            // Keep statistics updated
            auto state(d->mState);

            mNbTransfersPerState[state]--;
            mNbTransfersPerFileType[d->mFileType]--;

            if (!(state == MegaTransfer::STATE_COMPLETED
                    || state == MegaTransfer::STATE_CANCELLED
                    || state == MegaTransfer::STATE_FAILED))
            {
                mNbTransfersPerType[d->mType]--;
            }

            if (state == MegaTransfer::STATE_FAILED)
            {
                auto transfer (mFailedTransfers.take(tag));
                if (transfer)
                {
                    delete transfer;
                }
            }
            auto rem (mRemainingTimes.take(tag));
            if (rem)
            {
                delete rem;
            }
            if (d->mPublicNode)
            {
                delete d->mPublicNode;
            }

            mOrder.erase(mOrder.begin() + row);
        }
        endRemoveRows();

        if (mOrder.empty())
        {
            mModelHasTransfers = false;
            emit transfersInModelChanged(false);
        }
        mModelMutex->unlock();
        return true;
    }
    else
    {
        return false;
    }
}

bool QTransfersModel2::moveRows(const QModelIndex &sourceParent, int sourceRow, int count,
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

        mModelMutex->lock();
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
            const auto d (static_cast<const TransferItem2*>(mTransfers[tag].constData())
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

Qt::ItemFlags QTransfersModel2::flags(const QModelIndex &index) const
{
    Qt::ItemFlags flags (QAbstractItemModel::flags(index));
    if (index.isValid())
    {
        mModelMutex->lock();
//        auto d (static_cast<TransferData*>(index.internalPointer()));
        //        auto d (static_cast<TransferData*>(index.internalPointer()));
        TransferTag tag (static_cast<TransferTag>(index.internalId()));
        const auto d (static_cast<const TransferItem2*>(mTransfers[tag]
                          .constData())->getTransferData());
        if (d)
        {
            const auto state (d->mState);

            if ((state == MegaTransfer::STATE_QUEUED
                 || state == MegaTransfer::STATE_ACTIVE
                 || state == MegaTransfer::STATE_PAUSED
                 || state == MegaTransfer::STATE_RETRYING))
            {
                flags |= Qt::ItemIsDragEnabled;
            }
        }
        mModelMutex->unlock();
    }
    else
    {
        flags |= Qt::ItemIsDropEnabled;
    }

    return flags;
}

Qt::DropActions QTransfersModel2::supportedDropActions() const
{
    return Qt::MoveAction;
}

QMimeData* QTransfersModel2::mimeData(const QModelIndexList& indexes) const
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

bool QTransfersModel2::dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                    int column, const QModelIndex& parent)
{
    QByteArray byteArray (data->data(QString::fromUtf8("application/x-qabstractitemmodeldatalist")));
    QDataStream stream (&byteArray, QIODevice::ReadOnly);
    QList<TransferTag> tags;
    stream >> tags;

    mModelMutex->lock();

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

void QTransfersModel2::insertTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer, int row)
{
    auto tag (transfer->getTag());
    if (mTransfers.find(tag) == mTransfers.end())
    {
        auto state (transfer->getState());
        auto type (transfer->getType());
        auto fileName (QString::fromUtf8(transfer->getFileName()));
        auto path (QString::fromUtf8(transfer->getPath()));
        auto fileType (mFileTypes[Utilities::getExtensionPixmapName(fileName, QString())]);
        auto speed (api->getCurrentSpeed(type));
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes (transfer->getTransferredBytes());
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

        TransferData dataRow(type, errorCode, state, tag, errorValue, 0, remSecs.count(),
                             totalBytes, priority, speed, transfer->getMeanSpeed(),
                             transferredBytes, transfer->getPublicMegaNode(), fileType,
                             transfer->getParentHandle(), transfer->getNodeHandle(), api,
                             fileName, path);

         mTransfers[tag] = QVariant::fromValue(TransferItem2(dataRow));
         mRemainingTimes[tag] = rem;

         mOrder.insert(mOrder.begin() + row, tag);

         // Update statistics
         mNbTransfersPerState[state]++;
         mNbTransfersPerFileType[fileType]++;
         mNbTransfersPerType[type]++;

         if (mOrder.size() == 1)
         {
             mModelHasTransfers = true;
             emit transfersInModelChanged(true);
         }
    }
}
