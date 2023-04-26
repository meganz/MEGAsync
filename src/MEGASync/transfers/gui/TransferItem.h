#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "Utilities.h"
#include "megaapi.h"

#include <QSharedData>
#include <QMimeDatabase>

//Place here as they represent the number of real columns
enum class SortCriterion
{
    NAME       = 0,
    TOTAL_SIZE = 1,
    SPEED      = 2,
    PRIORITY   = 3,
    TIME       = 4,
    LAST       = 5
};

static const QColor UPLOAD_TRANSFER_COLOR = QColor("#2BA6DE");
static const QColor DOWNLOAD_TRANSFER_COLOR = QColor("#31B500");

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
    static const TransferStates PROCESSING_STATES_MASK;
    static const TransferStates PENDING_STATES_MASK;
    static const TransferStates ACTIVE_STATES_MASK;
    static const TransferStates STATE_MASK;

    static const TransferTypes TYPE_MASK;

    TransferData(mega::MegaTransfer* transfer = nullptr){update(transfer);}
    ~TransferData(){}

    TransferData(TransferData const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode), mTag(dr->mTag), mFolderTransferTag(dr->mFolderTransferTag),
        mErrorValue(dr->mErrorValue), mTemporaryError(dr->mTemporaryError),
        mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed),
        mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes),
        mNotificationNumber(dr->mNotificationNumber),
        mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle), mFailedTransfer(dr->mFailedTransfer),
        mFilename(dr->mFilename), mNodeAccess(mega::MegaShare::ACCESS_UNKNOWN),
        mPath(dr->mPath), mFinishedTime(dr->mFinishedTime),mState(dr->mState), mIgnorePauseQueueState(dr->mIgnorePauseQueueState)
    {}

    void update(mega::MegaTransfer* transfer);
    bool hasChanged(QExplicitlySharedDataPointer<TransferData> data);
    void removeFailedTransfer();

    void setPauseResume(bool isPaused);
    bool ignoreUpdate(const TransferState &state);
    void resetIgnoreUpdateUntilSameState();

    static TransferData::TransferState convertState(int state);

    TransferTypes                       mType;
    int                                 mErrorCode = 0;
    int                                 mTag = 0;
    int                                 mFolderTransferTag = 0;
    long long                           mErrorValue = 0;
    bool                                mTemporaryError = false;
    int64_t                             mRemainingTime = 0;
    unsigned long long                  mTotalSize = 0;
    unsigned long long                  mPriority = 0;
    unsigned long long                  mSpeed = 0;
    unsigned long long                  mMeanSpeed = 0;
    unsigned long long                  mTransferredBytes = 0;
    long long                           mNotificationNumber = 0;
    Utilities::FileType                 mFileType = Utilities::FileType::TYPE_OTHER;
    mega::MegaHandle                    mParentHandle = 0;
    mega::MegaHandle                    mNodeHandle = 0;
    std::shared_ptr<mega::MegaTransfer> mFailedTransfer;
    QString                             mFilename;
    int                                 mNodeAccess = 0;

    void setState(const TransferState& state);
    void setPreviousState(const TransferState& state);
    TransferState getState() const;
    TransferState getPreviousState() const;
    void resetStateHasChanged();
    bool stateHasChanged() const;

    QString path() const;
    bool isPublicNode() const;
    bool isCancelable() const;
    bool isFinished() const;
    bool wasFinished() const;
    bool isUpload() const;
    bool isSyncTransfer() const;
    bool isActiveOrPending() const;
    bool wasActiveOrPending() const;
    bool isActive() const;
    bool wasActive() const;
    bool isPaused() const;
    bool wasPaused() const;
    bool isProcessing() const;
    bool wasProcessing() const;
    bool isCompleted() const;
    bool isCompleting() const;
    bool isFailed() const;
    bool canBeRetried() const;
    bool isPermanentFail() const;
    bool isCancelled() const;
    int64_t getRawFinishedTime() const;
    int64_t getSecondsSinceFinished() const;
    QString getFormattedFinishedTime() const;
    QString getFullFormattedFinishedTime() const;

private:
    QString         mPath;
    int64_t         mFinishedTime = 0;
    TransferState   mState = TransferState::TRANSFER_NONE;
    TransferState   mPreviousState = TransferState::TRANSFER_NONE;
    bool            mIgnorePauseQueueState = false;

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
