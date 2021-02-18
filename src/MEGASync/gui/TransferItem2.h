#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "ui_TransferManagerItem.h"

#include "Utilities.h"

#include <QSharedData>
#include <QMimeDatabase>

typedef int TransferTag;


enum FileTypes
{
    TYPE_OTHER    = 0,
    TYPE_AUDIO    = 1,
    TYPE_VIDEO    = 2,
    TYPE_ARCHIVE  = 3,
    TYPE_DOCUMENT = 4,
    TYPE_IMAGE    = 5,
    TYPE_TEXT     = 6,
};

class TransferDataRow : public QSharedData
{
    public:
    int       mType;
    int       mErrorCode;
    int       mState;
    int       mTag;
    long long mErrorValue;
    int64_t   mFinishedTime;
    int64_t   mRemainingTime;
    long long mTotalSize;
    unsigned long long mPriority;
    long long mSpeed;
    long long mMeanSpeed;
    long long mTransferredBytes;
    int64_t   mUpdateTime;
    bool      mPublicNode;
    bool      mIsSyncTransfer;
    FileTypes mFileType;
    QString   mFilename;

    TransferDataRow(){}

    TransferDataRow(TransferDataRow const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode),  mState(dr->mState), mTag(dr->mTag),
        mErrorValue(dr->mErrorValue), mFinishedTime(dr->mFinishedTime), mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed), mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes), mUpdateTime(dr->mUpdateTime),
        mPublicNode(dr->mPublicNode), mIsSyncTransfer(dr->mIsSyncTransfer), mFileType(dr->mFileType),
        mFilename(dr->mFilename){}

    TransferDataRow(int type, int errorCode, int state, int tag, int errorValue,
                    int64_t finishedTime, int64_t remainingTime, long long totalSize, unsigned long long priority,
                    long long speed, long long meanSpeed, long long transferredBytes,
                    int64_t updateTime, bool publicNode, bool isSyncTransfer, FileTypes fileType,
                    QString fileName) :
         mType(type), mErrorCode(errorCode),  mState(state), mTag(tag),
         mErrorValue(errorValue), mFinishedTime(finishedTime), mRemainingTime(remainingTime),
         mTotalSize(totalSize), mPriority(priority), mSpeed(speed), mMeanSpeed(meanSpeed),
         mTransferredBytes(transferredBytes), mUpdateTime(updateTime),
         mPublicNode(publicNode), mIsSyncTransfer(isSyncTransfer), mFileType(fileType),
         mFilename(fileName){}
};
Q_DECLARE_TYPEINFO(TransferDataRow, Q_MOVABLE_TYPE);

class TransferItem2
{
public:
        TransferItem2();
        TransferItem2(const TransferItem2& ti);
        TransferItem2(const TransferDataRow& dataRow);

        ~TransferItem2() {}

        QSharedDataPointer<TransferDataRow> getTransferData() const;

        void updateValuesTransferFinished(uint64_t updateTime,
                                          int errorCode, long long errorValue,
                                          long long meanSpeed,
                                          int state, long long transferedBytes);

        void updateValuesTransferUpdated(uint64_t updateTime,
                                         int errorCode, long long errorValue,
                                         long long meanSpeed,
                                         long long speed,
                                         unsigned long long priority,
                                         int state, long long transferedBytes);

protected:
        QSharedDataPointer<TransferDataRow> d;
};

Q_DECLARE_METATYPE(TransferItem2)
Q_DECLARE_METATYPE(TransferDataRow)


#endif // TRANSFERITEM2_H
