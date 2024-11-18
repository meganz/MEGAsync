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
            if (data->isCompleted())
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
        else if (mType == TransferData::TransferType::TRANSFER_DOWNLOAD &&
                 mTrackedTransfers.contains(data->mNodeHandle))
        {
            emitSignal(data, QVariant::fromValue<uint64_t>(data->mNodeHandle));
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
