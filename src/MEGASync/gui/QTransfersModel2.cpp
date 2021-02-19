#include "QTransfersModel2.h"
#include "MegaApplication.h"
#include "Utilities.h"

using namespace mega;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mTransfers(QMap<TransferTag, TransferItem2*>()),
    mOrder(QList<TransferTag>()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mNotificationNumber(0)

{

    // Init File Types
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.txt"), QString())] = TYPE_TEXT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.wav"), QString())] = TYPE_AUDIO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.mkv"), QString())] = TYPE_VIDEO;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.tar"), QString())] = TYPE_ARCHIVE;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.odt"), QString())] = TYPE_DOCUMENT;
    mFileTypes[Utilities::getExtensionPixmapName(QLatin1Literal("a.png"), QString())] = TYPE_IMAGE;

    // Connect to transfer changes signals
    mMegaApi->addTransferListener(this);
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
        auto tag (mOrder.at(index.row()));
        auto ti  (*(mTransfers.find(tag)));
        return QVariant::fromValue(*ti);
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

   return createIndex(row, column, mOrder.at(row));
}

QTransfersModel2::~QTransfersModel2()
{
    mMegaApi->removeTransferListener(this);
    qDeleteAll(mTransfers);
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
                std::shared_ptr<MegaTransferData> transferData(mMegaApi->getTransferData());
                mNotificationNumber = transferData->getNotificationNumber();

                for (auto i (0); i < transfers->size(); ++i)
                {
                    mega::MegaTransfer* mt (transfers->get(i));

                    if (!mt->isStreamingTransfer()
                            && !mt->isFolderTransfer()
                            && mt->getPriority())
                    {
                        mOrder.append(i);
                    }
                }

                auto nbRows(mOrder.size());

                if (nbRows > 0)
                {
                    // Load in chunks for responsiveness

                    beginInsertRows(QModelIndex(), 0, nbRows-1);
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

                        beginInsertRows(QModelIndex(), first, last);

                        for (auto row (first); row < last; ++row)
                        {
                            mega::MegaTransfer* mt (transfers->get(mOrder[row]));

                            TransferTag tag (mt->getTag());
                            QString fileName (QString::fromUtf8(mt->getFileName()));

                            FileTypes fileType = mFileTypes[Utilities::getExtensionPixmapName(fileName, QString())];

                            TransferDataRow dataRow(
                                        mt->getType(),
                                        0,
                                        mt->getState(),
                                        tag,
                                        0,
                                        0,
                                        42,
                                        mt->getTotalBytes(),
                                        mt->getPriority(),
                                        mt->getSpeed(),
                                        mt->getMeanSpeed(),
                                        mt->getTransferredBytes(),
                                        mt->getUpdateTime(),
                                        false,
                                        mt->isSyncTransfer(),
                                        fileType,
                                        fileName);

                            TransferItem2* ti = new TransferItem2(dataRow);

                            mOrder[row] = tag;
                            mTransfers.insert(tag, ti);
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
    TransferTag tag (transfer->getTag());

    beginInsertRows(QModelIndex(), row, row);

    TransferDataRow dataRow(
                transfer->getType(),
                0,
                transfer->getState(),
                tag,
                0,
                0,
                42,
                transfer->getTotalBytes(),
                transfer->getPriority(),
                transfer->getSpeed(),
                transfer->getMeanSpeed(),
                transfer->getTransferredBytes(),
                transfer->getUpdateTime(),
                false,
                transfer->isSyncTransfer(),
                TYPE_TEXT,
                QString::fromUtf8(transfer->getFileName()));


    TransferItem2* ti = new TransferItem2(dataRow);

    mOrder.append(tag);
    mTransfers.insert(tag, ti);

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
        auto ti (mTransfers[tag]);

        ti->updateValuesTransferFinished(transfer->getUpdateTime(),
                                         0,
                                         0,
                                         transfer->getMeanSpeed(),
                                         transfer->getState(),
                                         transfer->getTransferredBytes());

        emit dataChanged(index(row, 0), index(row, 0));
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
    auto ti (mTransfers[tag]);

    ti->updateValuesTransferUpdated(transfer->getUpdateTime(),
                                    0,
                                    0,
                                    transfer->getMeanSpeed(),
                                    transfer->getSpeed(),
                                    transfer->getPriority(),
                                    transfer->getState(),
                                    transfer->getTransferredBytes());

    emit dataChanged(index(row, 0), index(row, 0));
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
    auto ti (*(mTransfers.find(tag)));

    ti->updateValuesTransferUpdated(transfer->getUpdateTime(),
                                    transfer->getLastError().getErrorCode(),
                                    transfer->getLastErrorExtended()->getValue(),
                                    transfer->getMeanSpeed(),
                                    transfer->getSpeed(),
                                    transfer->getPriority(),
                                    transfer->getState(),
                                    transfer->getTransferredBytes());

    emit dataChanged(index(row, 0), index(row, 0));
}

