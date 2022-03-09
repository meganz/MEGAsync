#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "Utilities.h"
#include "megaapi.h"

#include <QSharedData>
#include <QMimeDatabase>

//Place here as they represent the number of real columns
enum class SortCriterion
{
    PRIORITY   = 0,
    TOTAL_SIZE = 1,
    NAME       = 2,
    LAST       = 3
};

typedef int TransferTag;

class TransferData : public QSharedData
{
public:
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

    TransferData(mega::MegaTransfer* transfer = nullptr){update(transfer);}
    ~TransferData(){}

    TransferData(TransferData const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode),  mState(dr->mState), mTag(dr->mTag),
        mErrorValue(dr->mErrorValue), mTemporaryError(dr->mTemporaryError),
        mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed),
        mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes),
        mNotificationNumber(dr->mNotificationNumber),
        mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle), mFailedTransfer(dr->mFailedTransfer),
        mFilename(dr->mFilename), mNodeAccess(mega::MegaShare::ACCESS_UNKNOWN),
        mPath(dr->mPath), mFinishedTime(dr->mFinishedTime)
    {}

    void update(mega::MegaTransfer* transfer);
    void removeFailedTransfer();

    TransferTypes                       mType;
    int                                 mErrorCode;
    TransferState                       mState;
    int                                 mTag;
    long long                           mErrorValue;
    bool                                mTemporaryError;
    int64_t                             mRemainingTime;
    unsigned long long                  mTotalSize;
    unsigned long long                  mPriority;
    unsigned long long                  mSpeed;
    unsigned long long                  mMeanSpeed;
    unsigned long long                  mTransferredBytes;
    long long                           mNotificationNumber;
    Utilities::FileType                 mFileType;
    mega::MegaHandle                    mParentHandle;
    mega::MegaHandle                    mNodeHandle;
    std::shared_ptr<mega::MegaTransfer> mFailedTransfer;
    QString                             mFilename;
    int                                 mNodeAccess;

    QString path() const;
    bool isPublicNode() const;
    bool isCancelable() const;
    bool isFinished() const;
    bool isDownload() const;
    bool isUpload() const;
    bool isSyncTransfer() const;
    uint64_t getFinishedTime() const;

private:
    QString   mPath;
    int64_t   mFinishedTime;
};
Q_DECLARE_TYPEINFO(TransferData, Q_MOVABLE_TYPE);
Q_DECLARE_METATYPE(TransferData)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferStates)
Q_DECLARE_OPERATORS_FOR_FLAGS(TransferData::TransferTypes)

class TransferItem
{
    public:
        TransferItem() : d(new TransferData()){}
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
