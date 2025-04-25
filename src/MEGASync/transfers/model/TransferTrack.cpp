#include "TransferTrack.h"

TransferTrack::TransferTrack(const QString& id, TransferData::TransferType type):
    QObject(),
    mId(id),
    mType(type)
{}

void TransferTrack::addTransferToTrack(const QVariant& transferId)
{
    if (!mTrackedTransfers.contains(transferId))
    {
        mTrackedTransfers.append(transferId);
    }
}

void TransferTrack::checkTransfer(QExplicitlySharedDataPointer<TransferData> data)
{
    if (data->mType.testFlag(mType))
    {
        auto emitSignal =
            [this](QExplicitlySharedDataPointer<TransferData> data, QVariant transferId)
        {
            if (data->isFinished())
            {
                emit transferFinished(data);
                mTrackedTransfers.removeAll(transferId);
            }
            else
            {
                emit transferStarted(data);
            }
        };

        if (mType == TransferData::TransferType::TRANSFER_UPLOAD)
        {
            auto path(data->path());
            if (mTrackedTransfers.contains(path))
            {
                emitSignal(data, path);
            }
        }
        else if (mType == TransferData::TransferType::TRANSFER_DOWNLOAD)
        {
            auto handle(QVariant::fromValue<uint64_t>(data->mNodeHandle));
            if (mTrackedTransfers.contains(handle))
            {
                emitSignal(data, handle);
            }
        }
    }
}

QString TransferTrack::id() const
{
    return mId;
}

bool TransferTrack::areAllTransfersFinished() const
{
    return mTrackedTransfers.isEmpty();
}
