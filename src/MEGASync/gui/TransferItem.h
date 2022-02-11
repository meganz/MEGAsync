#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "Utilities.h"
#include "megaapi.h"

#include <QSharedData>
#include <QMimeDatabase>
#include <QDebug>

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

    //static const QHash<QString, TransferData::FileType> ExtensionFileTypes;

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

    static const TransferStates FINISHED_STATES_MASK;
    static const TransferStates PAUSABLE_STATES_MASK;
    static const TransferStates CANCELABLE_STATES_MASK;
    static const TransferStates ACTIVE_STATES_MASK;
    static const TransferStates STATE_MASK;

    static const TransferTypes TYPE_MASK;

    TransferData(){}

    TransferData(TransferData const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode),  mState(dr->mState), mTag(dr->mTag),
        mErrorValue(dr->mErrorValue), mFinishedTime(dr->mFinishedTime),
        mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed),
        mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes),
        mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle),
        mFilename(dr->mFilename), mPath(dr->mPath), mNodeAccess(mega::MegaShare::ACCESS_UNKNOWN), mIsPublicNode(dr->mIsPublicNode), mUpdated(false)
    {
        mIsSyncTransfer = mType.testFlag(TransferData::TransferType::TRANSFER_SYNC);
    }

    TransferData(mega::MegaTransfer*) : mUpdated(false)
    {}

    void update();
    void update(mega::MegaTransfer* transfer);
    void updateBasicInfo(mega::MegaTransfer* transfer);

    bool isFinished()
    {
        return mState == mega::MegaTransfer::STATE_COMPLETED
                || mState == mega::MegaTransfer::STATE_FAILED;
    }

    bool isUpdated()
    {
        return mUpdated;
    }

    void setUpdated(bool state)
    {
        mUpdated = state;
    }

    //Initialise with these values
    TransferState mState;
    int       mTag;
    unsigned long long mPriority;
    QString   mFilename;
    TransferTypes mType;

    //Updated only when paint needed
    int       mErrorCode;
    bool mIsSyncTransfer;
    long long mErrorValue;
    int64_t   mFinishedTime;
    int64_t   mRemainingTime;
    unsigned long long mTotalSize;
    unsigned long long mSpeed;
    unsigned long long mMeanSpeed;
    unsigned long long mTransferredBytes;
    bool mIsPublicNode;
    int mNodeAccess;
    FileType mFileType;
    mega::MegaHandle mParentHandle;
    mega::MegaHandle mNodeHandle;

    //Is it Updated?
    bool mUpdated;

    QString path() const;

private:
    QString   mPath;

};
Q_DECLARE_TYPEINFO(TransferData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TransferData)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::FileTypes)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferStates)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferTypes)
Q_DECLARE_METATYPE(TransferData::FileType)

class TransferItem
{
    public:
        TransferItem() : d(new TransferData){}
        TransferItem(const TransferItem& tdr) : d(tdr.d) {}
        TransferItem(const QExplicitlySharedDataPointer<TransferData>& tdr) : d(tdr) {}

        QExplicitlySharedDataPointer<TransferData> getTransferData() const
        {
            return d;
        }

    protected:
            QExplicitlySharedDataPointer<TransferData> d;
};
Q_DECLARE_METATYPE(TransferItem)
#endif // TRANSFERITEM2_H
