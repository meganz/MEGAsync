#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "Utilities.h"

using namespace mega;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mPreferences(Preferences::instance()),
    mTransfers(QMap<TransferTag,QVariant>()),
    mRemainingTimes(QMap<TransferTag, TransferRemainingTime*>()),
    mOrder(QList<TransferTag>()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mModelMutex(QMutex::Recursive),
    mNotificationNumber(0)
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
    QObject::connect((MegaApplication *)qApp, &MegaApplication::pauseStateChanged,
                      this, &QTransfersModel2::onPauseStateChanged);

    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
    mAreAllPaused = mPreferences->getGlobalPaused();

    qRegisterMetaType<TransferData::FileTypes>("TransferData::FileTypes");

    // Connect to transfer changes signals
    mMegaApi->addTransferListener(this);

    initModel();
}

int QTransfersModel2::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return mTransfers.size();
}

int QTransfersModel2::columnCount(const QModelIndex &parent) const
{
    return 1;
}

QVariant QTransfersModel2::data(const QModelIndex &index, int role) const
{
    if (!index.isValid() || (index.row() < 0 || mTransfers.count() <= index.row()))
    {
        return QVariant();
    }

    if (role == Qt::DisplayRole)
    {
        return mTransfers[mOrder[index.row()]];
    }
    return QVariant();
}

QModelIndex QTransfersModel2::parent(const QModelIndex &) const
{
    return QModelIndex();
}

QModelIndex QTransfersModel2::index(int row, int column, const QModelIndex &parent) const
{
    QModelIndex index;

    if (!hasIndex(row, column, parent))
    {
        index = QModelIndex();
    }
    else
    {
        auto tag (mOrder.at(row));
        index = createIndex(row, column, tag);
    }

    return index;
}

QTransfersModel2::~QTransfersModel2()
{
    mMegaApi->removeTransferListener(this);

    mModelMutex.lock();
    for (auto transfer : qAsConst(mTransfers))
    {
        auto transferItem (static_cast<TransferItem2*>(transfer.data()));
        auto d (transferItem->getTransferData());

        if (d->mPublicNode)
        {
            delete d->mPublicNode;
        }
    }

    qDeleteAll(mFailedTransfers);
    qDeleteAll(mRemainingTimes);
    mModelMutex.unlock();
}

void QTransfersModel2::initModel()
{
    QTransfersModel2* transferModel = this;
    mThreadPool->push([this, transferModel]()
    {//thread pool function
        if (transferModel)
        {
            Utilities::queueFunctionInAppThread([this, transferModel]()
            {//queued function
                if (transferModel) //Check if this is not deleted
                {
                    auto transfers (mMegaApi->getTransfers());
                    auto transferData(mMegaApi->getTransferData());
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
                                    auto otherTag = transfersToAdd.rbegin();
                                    bool found (false);
                                    int insertAt(transfersToAdd.size());
                                    while (!found && otherTag != transfersToAdd.rend())
                                    {
                                        other = transfers->get(*otherTag);
                                        if (type == other->getType() && priority > other->getPriority())
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

                    // Then load them in the model by chunks of 50
                    const auto nbRows(transfersToAdd.size());
                    if (nbRows > 0)
                    {
                        auto remainingRows(nbRows);

                        // Load in chunks for responsiveness
                        constexpr int rowsPerChunk (50);
                        auto nbChunks (nbRows / rowsPerChunk);

                        // Add 1 chunk more if needed
                        if ((nbChunks * rowsPerChunk) < nbRows)
                        {
                            nbChunks++;
                        }

                        for (auto chunk(0); chunk < nbChunks; ++chunk)
                        {
                            auto first (nbRows - remainingRows);
                            auto nbRowsInChunk (std::min(remainingRows, rowsPerChunk));

                            mModelMutex.lock();
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

                            endInsertRows();
                            mModelMutex.unlock();
                            remainingRows -= rowsPerChunk;
                        }
                    }
                    delete transfers;
                }
            });//end of queued function
        }
    });// end of thread pool function
}

void QTransfersModel2::onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }
    mNotificationNumber = transfer->getNotificationNumber();

    mModelMutex.lock();

    auto row (mOrder.size());
    // Find place
    if (row > 0)
    {
        auto priority (transfer->getPriority());
        auto other (qvariant_cast<TransferItem2>(mTransfers[mOrder.first()]).getTransferData());
        if (priority < other->mPriority)
        {
            row = 0;
        }
        else
        {
            auto otherTag (mOrder.rbegin());
            other = qvariant_cast<TransferItem2>(mTransfers[*otherTag]).getTransferData();
            while (priority < other->mPriority && otherTag != mOrder.rend())
            {
                otherTag++;
                other = qvariant_cast<TransferItem2>(mTransfers[*otherTag]).getTransferData();
                row--;
            }
        }
    }
    beginInsertRows(QModelIndex(), row, row);
    insertTransfer(api, transfer, row);
    endInsertRows();

    mModelMutex.unlock();
}

void QTransfersModel2::onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }
    mNotificationNumber = transfer->getNotificationNumber();

    TransferTag tag (transfer->getTag());

    mModelMutex.lock();
    auto row (mOrder.indexOf(tag));
    if (row >= 0)
    {
        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        auto state (transfer->getState());
        auto prevState (d->mState);
        int  errorCode (MegaError::API_OK);
        auto errorValue (0LL);
        if (error)
        {
            errorCode = error->getErrorCode();
            errorValue = error->getValue();
        }
        auto transferredBytes(transfer->getTransferredBytes());
        auto meanSpead(transfer->getMeanSpeed());
        if (meanSpead == 0)
        {
            meanSpead = transferredBytes;
        }
        // Time is in decisecs.
        auto  finishTime = QDateTime::currentSecsSinceEpoch();
        finishTime += (transfer->getUpdateTime() - transfer->getStartTime()) / 10;

        transferItem->updateValuesTransferFinished( finishTime, errorCode, errorValue,
                                                    meanSpead, state, transferredBytes,
                                                    transfer->getParentHandle(),
                                                    transfer->getNodeHandle(),
                                                    transfer->getPublicMegaNode());
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
        if (rem != nullptr)
        {
            delete rem;
        }
    }
    else
    {
        onTransferStart(api, transfer);
    }
    mModelMutex.unlock();
}

void QTransfersModel2::onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }
    mNotificationNumber = transfer->getNotificationNumber();

    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));

    if (row >= 0)
    {
        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        auto transferredBytes(transfer->getTransferredBytes());
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

            transferItem->updateValuesTransferUpdated(remSecs.count(), 0, 0,
                                                      transfer->getMeanSpeed(), speed, priority,
                                                      state, transferredBytes,
                                                      transfer->getPublicMegaNode());
            // Re-order if priority changed
            bool sameRow(true);
            if (priority != prevPriority)
            {
                TransferTag tagOther;
                QExplicitlySharedDataPointer<TransferData> dOther;
                auto newRow (row);
                int destRowToPassToBeginMove(0);

                mModelMutex.lock();
                int lastRow(mOrder.size()-1);

                // Search after if priority is higher
                if (priority > prevPriority && row < lastRow)
                {
                    bool found (false);
                    destRowToPassToBeginMove = lastRow + 1;
                    // Test first if the transfer was not sent to bottom
                    newRow = lastRow;
                    tagOther = mOrder.at(newRow);
                    dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();

                    // Find last of same type
                    while (d->mType != dOther->mType && newRow > row)
                    {
                        newRow--;
                        tagOther = mOrder.at(newRow);
                        dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();
                    }

                    // Check if found
                    if (d->mType == dOther->mType && priority > dOther->mPriority)
                    {
                        found = true;
                        destRowToPassToBeginMove = newRow + 1;
                    }
                    else
                    {
                        newRow = row;
                    }

                    // If not, search from next row
                    while (!found && newRow < lastRow)
                    {
                        newRow++;
                        tagOther = mOrder.at(newRow);
                        dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();

                        if (d->mType == dOther->mType && priority < dOther->mPriority)
                        {
                            found = true;
                            destRowToPassToBeginMove = newRow;
                            newRow--;
                        }
                    }
                }
                // Search before if priority is lower
                else if (priority < prevPriority && row > 0)
                {
                    bool found (false);
                    destRowToPassToBeginMove = 0;
                    // Test first if the transfer was not sent to top
                    newRow = 0;
                    tagOther = mOrder.at(newRow);
                    dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();

                    // Find the first of the same type
                    while (d->mType != dOther->mType && newRow < row)
                    {
                        newRow++;
                        tagOther = mOrder.at(newRow);
                        dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();
                    }

                    // Check if found
                    if (d->mType == dOther->mType && priority < dOther->mPriority)
                    {
                        found = true;
                        destRowToPassToBeginMove = newRow;
                    }
                    else
                    {
                        newRow = row;
                    }

                    // If not, search from previous row
                    while (!found && newRow > 0)
                    {
                        newRow--;
                        tagOther = mOrder.at(newRow);
                        dOther = static_cast<TransferItem2*>(mTransfers[tagOther].data())->getTransferData();

                        if (d->mType == dOther->mType && priority > dOther->mPriority)
                        {
                            found = true;
                            newRow++;
                            destRowToPassToBeginMove = newRow;
                        }
                    }
                }

                if (newRow != row)
                {
                    if (beginMoveRows(QModelIndex(), row, row, QModelIndex(), destRowToPassToBeginMove))
                    {
                        mOrder.move(row, newRow);
                        endMoveRows();
                        sameRow = false;
                    }
                }
                mModelMutex.unlock();
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
}

void QTransfersModel2::onTransferTemporaryError(mega::MegaApi *api,mega::MegaTransfer *transfer, mega::MegaError* error)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer()
            || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }
    mNotificationNumber = transfer->getNotificationNumber();

    TransferTag tag (transfer->getTag());

    mModelMutex.lock();
    auto row (mOrder.indexOf(tag));
    if (row >= 0)
    {
        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        auto speed (transfer->getSpeed());
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes(transfer->getTransferredBytes());
        auto rem (mRemainingTimes[transfer->getTag()]);
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes - transferredBytes));

        int errorCode(MegaError::API_OK);
        auto errorValue(0LL);
        auto megaError (transfer->getLastErrorExtended());
        if (megaError != nullptr)
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
    mModelMutex.unlock();
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

        mModelMutex.lock();
        for (auto row : rows)
        {
            tags.push_back(mOrder.at(row));
        }
        mModelMutex.unlock();

        for (auto tag : tags)
        {
            auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
            auto d(transferItem->getTransferData());

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
                            .arg(QString::fromUtf8(handle)).arg(QString::fromUtf8(key));
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
    // Reverse sort to keep indexes valid after deletion
    std::sort(indexes.rbegin(), indexes.rend());

    QList<TransferTag> tags;

    mModelMutex.lock();

    // Get transfer tags from the indexes.
    for (auto index : indexes)
    {
        tags.push_back(mOrder.at(index.row()));
    }

    // First clear finished transfers (remove rows), then cancel the others.
    // This way, there is no risk of messing up the rows order with cancel requests.
    int count (0);
    int row (mOrder.size());
    for (auto tag : tags)
    {
        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d (transferItem->getTransferData());

        // Clear (remove rows of) finished transfers
        if (d->mState == MegaTransfer::STATE_COMPLETED
                || d->mState == MegaTransfer::STATE_CANCELLED
                || d->mState == MegaTransfer::STATE_FAILED)
        {
            // Init row with row of first tag
            if (count == 0)
            {
                row = mOrder.lastIndexOf(tag, row);
            }

            // Flush pooled rows (start at row+1), init row with tag's
            if (mOrder.at(row) != tag)
            {
                removeRows(row + 1, count, QModelIndex());
                count = 0;
                row = mOrder.lastIndexOf(tag, row);
            }

            // Only one tag left. Flush pooled rows (start at row).
            if (tags.size() == 1)
            {
                removeRows(row, count + 1, QModelIndex());
            }

            count++;
            row--;
            tags.removeOne(tag);
        }
        // Do not cancel/clear completing transfers
        else if (d->mState == MegaTransfer::STATE_COMPLETING)
        {
            tags.removeOne(tag);
        }
    }
    mModelMutex.unlock();

    // All remaining tags, if any, are cancelable transfers. Cancel them.
    for (auto tag : tags)
    {
        mMegaApi->cancelTransferByTag(tag);
    }
}

void QTransfersModel2::pauseTransfers(QModelIndexList& indexes, bool pauseState)
{
    QList<TransferTag> tags;

    mModelMutex.lock();
    for (auto index : indexes)
    {
        tags.push_back(mOrder.at(index.row()));
    }
    mModelMutex.unlock();

    for (auto tag : tags)
    {
        auto transferItem (static_cast<TransferItem2*>(mTransfers[tag].data()));
        auto d(transferItem->getTransferData());

        if ((!pauseState && (d->mState == MegaTransfer::STATE_PAUSED))
                || (pauseState && (d->mState == MegaTransfer::STATE_ACTIVE
                                   || d->mState == MegaTransfer::STATE_QUEUED
                                   || d->mState == MegaTransfer::STATE_RETRYING)))
        {
            d->mMegaApi->pauseTransferByTag(d->mTag, pauseState);
        }
    }
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
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
    mAreAllPaused = mPreferences->getGlobalPaused();

    emit dataChanged(index(0, 0), index(mOrder.size()-1, 0), {Qt::DisplayRole});
}

void QTransfersModel2::onRetryTransfer(TransferTag tag)
{
    auto transfer(mFailedTransfers[tag]);
    if (transfer)
    {
        TransferItem2 transferItem (qvariant_cast<TransferItem2>(mTransfers[tag]));
        transferItem.getTransferData()->mMegaApi->retryTransfer(transfer);
        removeRows(mOrder.indexOf(tag), 1, QModelIndex());
    }
}

bool QTransfersModel2::removeRows(int row, int count, const QModelIndex &parent)
{
    if (parent == QModelIndex() && count > 0 && row >= 0)
    {
        mModelMutex.lock();
        beginRemoveRows(QModelIndex(), row, row + count - 1);

        for (auto i(0); i < count; ++i)
        {
            TransferTag tag (mOrder.takeAt(row));
            auto transferItem (qvariant_cast<TransferItem2>(mTransfers.take(tag)));
            auto d (transferItem.getTransferData());

            // Keep statistics updated
            auto state(d->mState);
            auto fileType(d->mFileType);

            mNbTransfersPerState[state]--;
            mNbTransfersPerFileType[fileType]--;

            if (!(state == MegaTransfer::STATE_COMPLETED
                    || state == MegaTransfer::STATE_CANCELLED
                    || state == MegaTransfer::STATE_FAILED))
            {
                auto type(d->mType);
                mNbTransfersPerType[type]--;
            }

            if (state == MegaTransfer::STATE_FAILED)
            {
                auto transfer(mFailedTransfers.take(tag));
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
        }
        endRemoveRows();
        if (mOrder.isEmpty())
        {
            emit transfersInModelChanged(false);
        }
        mModelMutex.unlock();
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
    int lastRow = sourceRow + count - 1;

    if (sourceParent == destinationParent
            && (destinationChild < sourceRow
                || destinationChild > lastRow))
    {
        // To keep order, do from first to last if destination is before first,
        // and from last to first if destination is after last.
        bool ascending (destinationChild < sourceRow ? false : true);

        QList<TransferTag> tagsToMove;

        mModelMutex.lock();
        const int rowCount = mOrder.size();

        for (auto row(sourceRow); row <= lastRow; ++row)
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
            auto d (qvariant_cast<TransferItem2>(mTransfers[tag]).getTransferData());
            if (destinationChild == 0)
            {
                d->mMegaApi->moveTransferToFirstByTag(d->mTag);
            }
            else if (destinationChild == rowCount)
            {
                d->mMegaApi->moveTransferToLastByTag(d->mTag);
            }
            else if (destinationChild < lastRow)
            {
                d->mMegaApi->moveTransferUpByTag(d->mTag);
            }
            else if (destinationChild > sourceRow)
            {
                d->mMegaApi->moveTransferDownByTag(d->mTag);
            }
        }
        mModelMutex.unlock();
        return true;
    }
    return false;
}

Qt::ItemFlags QTransfersModel2::flags(const QModelIndex &index) const
{
    Qt::ItemFlags defaultFlags = QAbstractItemModel::flags(index);
    if (index.isValid())
    {
        return Qt::ItemIsDragEnabled | defaultFlags;
    }
    return Qt::ItemIsDropEnabled | defaultFlags;
}

Qt::DropActions QTransfersModel2::supportedDropActions() const
{
    return Qt::MoveAction;
}

void QTransfersModel2::insertTransfer(mega::MegaApi *api, mega::MegaTransfer *transfer, int row)
{
    auto tag (transfer->getTag());
    if (mTransfers.find(tag) == mTransfers.end())
    {
        int state (transfer->getState());
        int type (transfer->getType());
        QString fileName (QString::fromUtf8(transfer->getFileName()));
        QString path (QString::fromUtf8(transfer->getPath()));
        TransferData::FileTypes fileType = mFileTypes[Utilities::getExtensionPixmapName(fileName, QString())];
        auto speed (api->getCurrentSpeed(type));
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes(transfer->getTransferredBytes());
        auto remBytes (totalBytes - transferredBytes);
        auto rem (new TransferRemainingTime(speed, remBytes));
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, remBytes));
        auto priority = transfer->getPriority();
        int errorCode(MegaError::API_OK);
        auto errorValue(0LL);
        auto megaError (transfer->getLastErrorExtended());
        if (megaError != nullptr)
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

         mOrder.insert(row, tag);

         // Update statistics
         mNbTransfersPerState[state]++;
         mNbTransfersPerFileType[fileType]++;
         mNbTransfersPerType[type]++;

         if (mOrder.size() == 1)
         {
             emit transfersInModelChanged(true);
         }
    }
}
