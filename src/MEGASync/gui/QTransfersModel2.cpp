#include "QTransfersModel2.h"
#include "MegaApplication.h"

using namespace mega;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mTransfers(QMap<TransferTag, TransferItem2*>()),
    mOrder(QList<TransferTag>()),
    mThreadPool(ThreadPoolSingleton::getInstance()),
    mNotificationNumber(0)

{
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
    auto transfers (mMegaApi->getTransfers());
    std::shared_ptr<MegaTransferData> transferData(mMegaApi->getTransferData());
    mNotificationNumber = transferData->getNotificationNumber();

    beginInsertRows(QModelIndex(), 0, transfers->size()); // Not good... can have folders....
    for (auto i (0); i < transfers->size(); ++i)
    {
        mega::MegaTransfer* mt (transfers->get(i));

        if (!mt->isStreamingTransfer()
                && !mt->isFolderTransfer()
                && mt->getPriority())
        {
            TransferTag tag (mt->getTag());

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
                        TYPE_TEXT,
                        QString::fromUtf8(mt->getFileName()));

            TransferItem2* ti = new TransferItem2(dataRow);

            mOrder.append(tag);
            mTransfers.insert(tag, ti);
        }
    }
    endInsertRows();

    delete transfers;
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
    auto ti (*(mTransfers.find(tag)));

    ti->updateValuesTransferFinished(transfer->getUpdateTime(),
                                     0,
                                     0,
                                     transfer->getMeanSpeed(),
                                     transfer->getState(),
                                     transfer->getTransferredBytes());

    emit dataChanged(index(row, 1), index(row, 1));
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
    auto ti (*(mTransfers.find(tag)));

    ti->updateValuesTransferUpdated(transfer->getUpdateTime(),
                                    0,
                                    0,
                                    transfer->getMeanSpeed(),
                                    transfer->getState(),
                                    transfer->getPriority(),
                                    transfer->getSpeed(),
                                    transfer->getTransferredBytes());

    emit dataChanged(index(row, 1), index(row, 1));
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
                                    transfer->getState(),
                                    transfer->getPriority(),
                                    transfer->getSpeed(),
                                    transfer->getTransferredBytes());

    emit dataChanged(index(row, 1), index(row, 1));
}

