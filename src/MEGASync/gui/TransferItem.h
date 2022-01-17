#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

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
        TRANSFER_NONE         = 1 << mega::MegaTransfer::STATE_NONE,
        TRANSFER_QUEUED       = 1 << mega::MegaTransfer::STATE_QUEUED,
        TRANSFER_ACTIVE       = 1 << mega::MegaTransfer::STATE_ACTIVE,
        TRANSFER_PAUSED       = 1 << mega::MegaTransfer::STATE_PAUSED,
        TRANSFER_RETRYING     = 1 << mega::MegaTransfer::STATE_RETRYING,
        TRANSFER_COMPLETING   = 1 << mega::MegaTransfer::STATE_COMPLETING,
        TRANSFER_COMPLETED    = 1 << mega::MegaTransfer::STATE_COMPLETED,
        TRANSFER_CANCELLED    = 1 << mega::MegaTransfer::STATE_CANCELLED,
        TRANSFER_FAILED       = 1 << mega::MegaTransfer::STATE_FAILED,
    };
    Q_DECLARE_FLAGS(TransferStates, TransferState)

    enum TransferType
    {
        TRANSFER_DOWNLOAD     = 1 << mega::MegaTransfer::TYPE_DOWNLOAD,
        TRANSFER_UPLOAD       = 1 << mega::MegaTransfer::TYPE_UPLOAD,
        TRANSFER_LTCPDOWNLOAD = 1 << mega::MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD,
        TRANSFER_SYNC         = 1 << (mega::MegaTransfer::TYPE_LOCAL_TCP_DOWNLOAD + 1),
    };
    Q_DECLARE_FLAGS(TransferTypes, TransferType)

    static const TransferStates STATE_MASK;
    static const TransferTypes TYPE_MASK;

    TransferTypes mType;
    int       mErrorCode;
    TransferState mState;
    bool mIsSyncTransfer;
    int       mTag;
    long long mErrorValue;
    int64_t   mFinishedTime;
    int64_t   mRemainingTime;
    unsigned long long mTotalSize;
    unsigned long long mPriority;
    unsigned long long mSpeed;
    unsigned long long mMeanSpeed;
    unsigned long long mTransferredBytes;
    bool mIsPublicNode;
    int mNodeAccess;
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
        mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle), mMegaApi(dr->mMegaApi),
        mFilename(dr->mFilename), mPath(dr->mPath), mNodeAccess(mega::MegaShare::ACCESS_UNKNOWN), mIsPublicNode(dr->mIsPublicNode)
    {
        mIsSyncTransfer = mType.testFlag(TransferData::TransferType::TRANSFER_SYNC);
    }

    TransferData(TransferTypes type, int errorCode, TransferState state, int tag, long long errorValue,
                 int64_t finishedTime, int64_t remainingTime, unsigned long long totalSize,
                 unsigned long long priority,
                 unsigned long long speed, unsigned long long meanSpeed, unsigned long long transferredBytes,
                 FileType fileType,
                 mega::MegaHandle parentHandle, mega::MegaHandle nodeHandle,
                 mega::MegaApi* megaApi, QString fileName, QString path) :
        mType(type), mErrorCode(errorCode),  mState(state), mTag(tag),
        mErrorValue(errorValue), mFinishedTime(finishedTime), mRemainingTime(remainingTime),
        mTotalSize(totalSize), mPriority(priority), mSpeed(speed), mMeanSpeed(meanSpeed),
        mTransferredBytes(transferredBytes),
        mFileType(fileType), mIsPublicNode(false),
        mParentHandle(parentHandle), mNodeHandle(nodeHandle),mMegaApi(megaApi),
        mFilename(fileName), mPath(path)
    {
        mIsSyncTransfer = mType.testFlag(TransferData::TransferType::TRANSFER_SYNC);
    }

    bool isFinished()
    {
        return mState == mega::MegaTransfer::STATE_COMPLETED
                || mState == mega::MegaTransfer::STATE_FAILED;
    }
};
Q_DECLARE_TYPEINFO(TransferData, Q_MOVABLE_TYPE);
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::FileTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferStates)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferTypes)
Q_DECLARE_METATYPE(TransferData::FileType)

class TransferItem
{
    public:
        TransferItem() : d(new TransferData){}
//        TransferItem(const TransferData& dataRow) : d(new TransferData(dataRow)) {}
        TransferItem(const TransferItem& tdr) : d(tdr.d) {}
        TransferItem(const QExplicitlySharedDataPointer<TransferData>& tdr) : d(tdr) {}

        void updateValuesTransferFinished(int64_t finishTime,
                                          int errorCode, long long errorValue,
                                          unsigned long long meanSpeed,
                                          TransferData::TransferState state,
                                          unsigned long long transferedBytes,
                                          mega::MegaHandle parentHandle,
                                          mega::MegaHandle nodeHandle);

        void updateValuesTransferUpdated(int64_t remainingTime,
                                         int errorCode, long long errorValue,
                                         unsigned long long meanSpeed,
                                         unsigned long long speed,
                                         unsigned long long priority,
                                         TransferData::TransferState state,
                                         unsigned long long transferedBytes);

        QExplicitlySharedDataPointer<TransferData> getTransferData() const
        {
            return d;
        }

    protected:
            QExplicitlySharedDataPointer<TransferData> d;
};
Q_DECLARE_METATYPE(TransferItem)
#endif // TRANSFERITEM2_H
