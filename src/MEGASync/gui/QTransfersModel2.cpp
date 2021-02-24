#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "Utilities.h"

using namespace mega;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mPreferences(Preferences::instance()),
    mTransfers(QMap<TransferTag,QVariant>()),
    mOrder(QList<TransferTag>()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mNotificationNumber(0),
    mAreDlPaused(mPreferences->getDownloadsPaused()),
    mAreUlPaused(mPreferences->getUploadsPaused()),
    mIsGlobalPaused(mPreferences->getGlobalPaused())
{

    // Init File Types
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.txt"), QString())] = TransferData::TYPE_TEXT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.wav"), QString())] = TransferData::TYPE_AUDIO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.mkv"), QString())] = TransferData::TYPE_VIDEO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.tar"), QString())] = TransferData::TYPE_ARCHIVE;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.odt"), QString())] = TransferData::TYPE_DOCUMENT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.png"), QString())] = TransferData::TYPE_IMAGE;

    // Connect to transfer changes signals
    mMegaApi->addTransferListener(this);

    // Connect to pause state change signal
    QObject::connect((MegaApplication *)qApp, &MegaApplication::pauseStateChanged,
                      this, &QTransfersModel2::onPauseStateChanged);
}

int QTransfersModel2::rowCount(const QModelIndex &parent) const
{
    if (parent.isValid())
    {
        return 0;
    }
    return mTransfers.size();

//    int numRows (3);
//
//    if (parent.isValid() == parent.column() == 0)
//    {

//        switch (parent.row())
//        {
//            case 0:
//            {
//                numRows = mTransfers.size();
//                break;
//            }
//            case 1:
//            {
//                numRows = 4;
//                break;
//            }
//            case 2:
//            {
//                numRows = 5;
//                break;
//            }
//        }
//    }

//    return numRows;
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
        auto ti (static_cast<TransferItem2 *>(index.internalPointer()));

        // Update paused state if necessary
        int state (ti->getState());
        switch (state)
        {
            case MegaTransfer::STATE_RETRYING:
            case MegaTransfer::STATE_QUEUED:
            case MegaTransfer::STATE_ACTIVE:
            case MegaTransfer::STATE_PAUSED:
            {
                auto type (ti->getType());
                auto unPausedState (ti->getUnpausedState());

                if (unPausedState == MegaTransfer::STATE_NONE)
                {
                    if ((type == MegaTransfer::TYPE_DOWNLOAD && mAreDlPaused)
                            || (type == MegaTransfer::TYPE_UPLOAD && mAreUlPaused))
                    {
                        ti->setPaused(true);
                    }

                }
                else
                {
                    if ((type == MegaTransfer::TYPE_DOWNLOAD && !mAreDlPaused)
                            || (type == MegaTransfer::TYPE_UPLOAD && !mAreUlPaused))
                    {
                        ti->setPaused(false);
                    }
                }
                break;
            }
        }
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
    if (!hasIndex(row, column, parent))//Check out
    {
        return QModelIndex();
    }

    int tag(mOrder.at(row));
    return createIndex(row, column, (void*) mTransfers[tag].data());
}

QTransfersModel2::~QTransfersModel2()
{
    mMegaApi->removeTransferListener(this);
    qDeleteAll(mRemainingTimes);
}

void QTransfersModel2::initModel()
{
    QTransfersModel2* transferModel = this;

    mThreadPool->push([this, transferModel]()
    {//thread pool function

        if (!transferModel)
        {
            return;
        }

        Utilities::queueFunctionInAppThread([this, transferModel]()
        {//queued function

            if (transferModel) //Check if this is not deleted
            {
                auto transfers (mMegaApi->getTransfers());
                auto transferData(mMegaApi->getTransferData());
                mNotificationNumber = transferData->getNotificationNumber();

                QList<TransferTag> transfersToAdd;

                for (auto i (0); i < transfers->size(); ++i)
                {
                    mega::MegaTransfer* mt (transfers->get(i));

                    if (!mt->isStreamingTransfer()
                            && !mt->isFolderTransfer()
                            && mt->getPriority())
                    {
                        transfersToAdd.append(i);
                    }
                }

                auto nbRows(transfersToAdd.size());

                if (nbRows > 0)
                {
                    // Load in chunks for responsiveness
                    constexpr int rowsPerChunk (50);
                    auto nbChunks (nbRows / rowsPerChunk);

                    if ((nbChunks * rowsPerChunk) < nbRows)
                    {
                        nbChunks++;
                    }

                    auto remainingRows(nbRows);

                    for (auto chunk(0); chunk < nbChunks; ++chunk)
                    {
                        auto first (nbRows - remainingRows);
                        auto last (first + std::min(remainingRows, rowsPerChunk));

                        auto nbRowsInModel (mOrder.size());
                        beginInsertRows(QModelIndex(), nbRowsInModel,
                                        nbRowsInModel -1 + std::min(remainingRows, rowsPerChunk));

                        for (auto row (first); row < last; ++row)
                        {
                            insertTransfer(transfers->get(transfersToAdd[row]));
                        }

                        endInsertRows();
                        remainingRows -= rowsPerChunk;
                    }
                }
                delete transfers;
            }
        });//end of queued function

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

    if (!transfer->getPriority())
    {
        return;
    }

    auto row (mOrder.size());

    beginInsertRows(QModelIndex(), row, row);

    insertTransfer(transfer);

    endInsertRows();
}

void QTransfersModel2::onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error)
{
    if (transfer->isStreamingTransfer() || transfer->isFolderTransfer())
    {
        return;
    }

    if (mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    if (!transfer->getPriority())
    {
        return;
    }

    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));

    if (row >= 0)
    {
        auto ti (static_cast<TransferItem2*>(index(row, 0, QModelIndex()).internalPointer()));


        ti->updateValuesTransferFinished(transfer->getUpdateTime(),
                                         0,
                                         0,
                                         transfer->getMeanSpeed(),
                                         transfer->getState(),
                                         transfer->getTransferredBytes());

        emit dataChanged(index(row, 0), index(row, 0));

        auto rem (mRemainingTimes.take(tag));
        if (rem != nullptr)
        {
            delete rem;
        }
    }
}

void QTransfersModel2::onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    if (transfer->isStreamingTransfer()
            || transfer->isFolderTransfer() || mNotificationNumber >= transfer->getNotificationNumber())
    {
        return;
    }

    if (!transfer->getPriority())
    {
        return;
    }

    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));
    if (row >= 0)
    {
        auto ti (static_cast<TransferItem2*>(index(row, 0, QModelIndex()).internalPointer()));

        auto speed (transfer->getSpeed());
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes(transfer->getTransferredBytes());
        TransferRemainingTime* rem (mRemainingTimes[transfer->getTag()]);
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes-transferredBytes));

        ti->updateValuesTransferUpdated(transfer->getUpdateTime(),
                                        remSecs.count(),
                                        0,
                                        0,
                                        transfer->getMeanSpeed(),
                                        speed,
                                        transfer->getPriority(),
                                        transfer->getState(),
                                        transferredBytes);

        emit dataChanged(index(row, 0), index(row, 0));
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

    if (!transfer->getPriority())
    {
        return;
    }

    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));

    if (row >= 0)
    {
        auto transferItem (static_cast<TransferItem2*>(index(row, 0, QModelIndex()).internalPointer()));

        auto speed (transfer->getSpeed());
        auto totalBytes (transfer->getTotalBytes());
        auto transferredBytes(transfer->getTransferredBytes());
        TransferRemainingTime* rem (mRemainingTimes[transfer->getTag()]);
        auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes-transferredBytes));

        int errorCode(MegaError::API_OK);
        auto errorValue(0LL);
        auto megaError (transfer->getLastErrorExtended());
        if (megaError != nullptr)
        {
            errorCode = megaError->getErrorCode();
            errorValue = megaError->getValue();
        }

        transferItem->updateValuesTransferUpdated(transfer->getUpdateTime(),
                                                  remSecs.count(),
                                                  errorCode,
                                                  errorValue,
                                                  transfer->getMeanSpeed(),
                                                  speed,
                                                  transfer->getPriority(),
                                                  transfer->getState(),
                                                  transferredBytes);

        emit dataChanged(index(row, 0), index(row, 0));
    }
}

void QTransfersModel2::onPauseStateChanged()
{
    mAreDlPaused = mPreferences->getDownloadsPaused();
    mAreUlPaused = mPreferences->getUploadsPaused();
    mIsGlobalPaused = mPreferences->getGlobalPaused();

    emit dataChanged(index(0, 0), index(0, mOrder.size()-1));
}

void QTransfersModel2::insertTransfer(mega::MegaTransfer *transfer)
{
    TransferTag tag (transfer->getTag());
    int state (transfer->getState());
    int type (transfer->getType());
    QString fileName (QString::fromUtf8(transfer->getFileName()));
    TransferData::FileTypes fileType = mFileTypes[Utilities::getExtensionPixmapName(fileName, QString())];
    auto speed (transfer->getSpeed());

    auto totalBytes (transfer->getTotalBytes());
    auto transferredBytes(transfer->getTransferredBytes());
    TransferRemainingTime* rem (new TransferRemainingTime());
    auto remSecs (rem->calculateRemainingTimeSeconds(speed, totalBytes-transferredBytes));

    int errorCode(MegaError::API_OK);
    auto errorValue(0LL);
    auto megaError (transfer->getLastErrorExtended());
    if (megaError != nullptr)
    {
        errorCode = megaError->getErrorCode();
        errorValue = megaError->getValue();
    }

    bool isPublic (transfer->getPublicMegaNode());

    TransferData dataRow(
                type,
                errorCode,
                state,
                MegaTransfer::STATE_NONE,
                tag,
                errorValue,
                0,
                remSecs.count(),
                totalBytes,
                transfer->getPriority(),
                speed,
                transfer->getMeanSpeed(),
                transferredBytes,
                transfer->getUpdateTime(),
                isPublic,
                transfer->isSyncTransfer(),
                fileType,
                mMegaApi,
                fileName);

     mTransfers[tag] = QVariant::fromValue(TransferItem2(dataRow));
     mRemainingTimes[tag] = rem;
     mOrder.append(tag);
}
