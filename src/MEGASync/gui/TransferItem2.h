#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "ui_TransferManagerItem.h"

#include "Utilities.h"
#include "megaapi.h"

#include <QSharedData>
#include <QMimeDatabase>

typedef int TransferTag;


class TransferData : public QSharedData
{
    public:

    enum FileType
    {
        TYPE_OTHER    = 0x01,
        TYPE_AUDIO    = 0x02,
        TYPE_VIDEO    = 0x04,
        TYPE_ARCHIVE  = 0x08,
        TYPE_DOCUMENT = 0x10,
        TYPE_IMAGE    = 0x20,
        TYPE_TEXT     = 0x40,
    };
    Q_DECLARE_FLAGS(FileTypes, FileType)

    enum TransferState
    {
        TRANSFER_NONE       = 1 << mega::MegaTransfer::STATE_NONE,
        TRANSFER_QUEUED     = 1 << mega::MegaTransfer::STATE_QUEUED,
        TRANSFER_ACTIVE     = 1 << mega::MegaTransfer::STATE_ACTIVE,
        TRANSFER_PAUSED     = 1 << mega::MegaTransfer::STATE_PAUSED,
        TRANSFER_RETRYING   = 1 << mega::MegaTransfer::STATE_RETRYING,
        TRANSFER_COMPLETING = 1 << mega::MegaTransfer::STATE_COMPLETING,
        TRANSFER_COMPLETED  = 1 << mega::MegaTransfer::STATE_COMPLETED,
        TRANSFER_CANCELLED  = 1 << mega::MegaTransfer::STATE_CANCELLED,
        TRANSFER_FAILED     = 1 << mega::MegaTransfer::STATE_FAILED,
    };
    Q_DECLARE_FLAGS(TransferStates, TransferState)

    int       mType;
    int       mErrorCode;
    TransferState mState;
    int       mTag;
    long long mErrorValue;
    int64_t   mFinishedTime;
    int64_t   mRemainingTime;
    long long mTotalSize;
    unsigned long long mPriority;
    long long mSpeed;
    long long mMeanSpeed;
    long long mTransferredBytes;
    mega::MegaNode* mPublicNode;
    FileType mFileType;
    mega::MegaHandle mParentHandle;
    mega::MegaHandle mNodeHandle;
    mega::MegaApi* mMegaApi;
    QString   mFilename;
    QString   mPath;

    TransferData(){}

    TransferData(TransferData const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode),  mState(dr->mState), mTag(dr->mTag),
        mErrorValue(dr->mErrorValue), mFinishedTime(dr->mFinishedTime),
        mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed),
        mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes),
        mPublicNode(dr->mPublicNode), mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle), mMegaApi(dr->mMegaApi),
        mFilename(dr->mFilename), mPath(dr->mPath){}

    TransferData(int type, int errorCode, TransferState state, int tag, long long errorValue,
                 int64_t finishedTime, int64_t remainingTime, long long totalSize,
                 unsigned long long priority,
                 long long speed, long long meanSpeed, long long transferredBytes,
                 mega::MegaNode* publicNode, FileType fileType,
                 mega::MegaHandle parentHandle, mega::MegaHandle nodeHandle,
                 mega::MegaApi* megaApi, QString fileName, QString path) :
        mType(type), mErrorCode(errorCode),  mState(state), mTag(tag),
        mErrorValue(errorValue), mFinishedTime(finishedTime), mRemainingTime(remainingTime),
        mTotalSize(totalSize), mPriority(priority), mSpeed(speed), mMeanSpeed(meanSpeed),
        mTransferredBytes(transferredBytes),
        mPublicNode(publicNode), mFileType(fileType),
        mParentHandle(parentHandle), mNodeHandle(nodeHandle),mMegaApi(megaApi),
        mFilename(fileName), mPath(path){}
};
Q_DECLARE_TYPEINFO(TransferData, Q_MOVABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::FileTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferStates)
Q_DECLARE_METATYPE(TransferData::FileType)

class TransferItem2
{
    public:
        TransferItem2() : d(new TransferData){}
        TransferItem2(const TransferItem2& tdr) : d(new TransferData(tdr.d.constData())) {}
        TransferItem2(const TransferData& dataRow) : d(new TransferData(dataRow)) {}

        void updateValuesTransferFinished(int64_t finishTime,
                                          int errorCode, long long errorValue,
                                          long long meanSpeed,
                                          TransferData::TransferState state,
                                          long long transferedBytes,
                                          mega::MegaHandle parentHandle,
                                          mega::MegaHandle nodeHandle,
                                          mega::MegaNode* publicNode);

        void updateValuesTransferUpdated(int64_t remainingTime,
                                         int errorCode, long long errorValue,
                                         long long meanSpeed,
                                         long long speed,
                                         unsigned long long priority,
                                         TransferData::TransferState state,
                                         long long transferedBytes,
                                         mega::MegaNode* publicNode);

        QExplicitlySharedDataPointer<TransferData> getTransferData() const
        {
            return d;
        }

    protected:
            QExplicitlySharedDataPointer<TransferData> d;
};
Q_DECLARE_METATYPE(TransferItem2)
#endif // TRANSFERITEM2_H
