#include "QTransfersModel2.h"
#include "MegaApplication.h"

using namespace mega;

QTransfersModel2::QTransfersModel2(QObject *parent) :
    QAbstractItemModel(parent),
    mMegaApi(((MegaApplication *)qApp)->getMegaApi()),
    mTransfers(QMap<TransferTag, TransferData*>()),
    mOrder(QList<TransferTag>()),
    mThreadPool(ThreadPoolSingleton::getInstance())

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

        auto td (*(mTransfers.find(mOrder.at(index.row()))));
        auto mt(mMegaApi->getTransferByTag(td->mMegaTransferTag));

        TransferDataRow row(mt->getType(),
                            mt->getLastError().getErrorCode(),
                            mt->getState(),
                            td->mMegaTransferTag,
                            mt->getLastErrorExtended()->getValue(),
                            td->mFinishedTime,
                            td->mRemTime.mRemainingSeconds,
                            mt->getTotalBytes(),
                            mt->getPriority(),
                            mt->getSpeed(),
                            mt->getMeanSpeed(),
                            mt->getTransferredBytes(),
                            mt->getUpdateTime(),
                            mt->getPublicMegaNode(),
                            mt->isSyncTransfer(),
                            td->mFileType,
                            QString::fromUtf8(mt->getFileName()));

        delete mt;
        return QVariant::fromValue(row);
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

    for (auto i (0); i < transfers->size(); ++i)
    {
        TransferData* td = new TransferData();
        mega::MegaTransfer* mt (transfers->get(i));

        td->mMegaTransferTag = mt->getTag();
        // TODO: get right type
        td->mFileType = TYPE_TEXT;
        td->mFinishedTime = 0;
        td->mRemTime = TransferRemainingTime();
        td->mRemTime.calculateRemainingTimeSeconds(mt->getSpeed(), mt->getTransferredBytes());

        mOrder.append(td->mMegaTransferTag);
    }

    delete transfers;
}

void QTransfersModel2::onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    TransferTag tag (transfer->getTag());
    auto td (new TransferData());
    auto row (mOrder.size());

    beginInsertRows(QModelIndex(), row, row);

    td->mMegaTransferTag = tag;
    td->mFileType = TYPE_TEXT;
    td->mFinishedTime = 0;
    td->mRemTime = TransferRemainingTime();
    td->mRemTime.calculateRemainingTimeSeconds(transfer->getSpeed(), transfer->getTransferredBytes());
    mTransfers.insert(tag, td);
    mOrder.append(tag);

    endInsertRows();
}

void QTransfersModel2::onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error)
{
    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));
    auto td (*(mTransfers.find(tag)));

    td->mFinishedTime = transfer->getUpdateTime();
    td->mRemTime.calculateRemainingTimeSeconds(transfer->getSpeed(), transfer->getTransferredBytes());

    emit dataChanged(index(row, 1), index(row, 1));
}

void QTransfersModel2::onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer)
{
    TransferTag tag (transfer->getTag());
    auto row (mOrder.indexOf(tag));
    auto td (*(mTransfers.find(tag)));

    td->mRemTime.calculateRemainingTimeSeconds(transfer->getSpeed(), transfer->getTransferredBytes());

    emit dataChanged(index(row, 1), index(row, 1));
}

void QTransfersModel2::onTransferTemporaryError(mega::MegaApi *api,mega::MegaTransfer *transfer, mega::MegaError* error)
{

}

