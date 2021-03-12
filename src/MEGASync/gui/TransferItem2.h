#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include "ui_TransferManagerItem.h"

#include "Utilities.h"

#include <QSharedData>
#include <QMimeDatabase>

typedef int TransferTag;


class TransferData : public QSharedData
{
    public:

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
    mega::MegaNode* mPublicNode;
    FileTypes mFileType;
    mega::MegaHandle mParentHandle;
    mega::MegaHandle mNodeHandle;
    mega::MegaApi* mMegaApi;
    QString   mFilename;
    QString   mPath;

    TransferData(){}

    TransferData(TransferData const* dr) :
        mType(dr->mType), mErrorCode(dr->mErrorCode),  mState(dr->mState), mTag(dr->mTag),
        mErrorValue(dr->mErrorValue), mFinishedTime(dr->mFinishedTime), mRemainingTime(dr->mRemainingTime),
        mTotalSize(dr->mTotalSize), mPriority(dr->mPriority), mSpeed(dr->mSpeed), mMeanSpeed(dr->mMeanSpeed),
        mTransferredBytes(dr->mTransferredBytes),
        mPublicNode(dr->mPublicNode), mFileType(dr->mFileType),
        mParentHandle (dr->mParentHandle), mNodeHandle (dr->mNodeHandle), mMegaApi(dr->mMegaApi),
        mFilename(dr->mFilename), mPath(dr->mPath){}

    TransferData(int type, int errorCode, int state, int tag, long long errorValue,
                    int64_t finishedTime, int64_t remainingTime, long long totalSize, unsigned long long priority,
                    long long speed, long long meanSpeed, long long transferredBytes,
                    mega::MegaNode* publicNode, FileTypes fileType,
                    mega::MegaHandle parentHandle, mega::MegaHandle nodeHandle,
                 mega::MegaApi* megaApi, QString fileName, QString path) :
         mType(type), mErrorCode(errorCode),  mState(state), mTag(tag),
         mErrorValue(errorValue), mFinishedTime(finishedTime), mRemainingTime(remainingTime),
         mTotalSize(totalSize), mPriority(priority), mSpeed(speed), mMeanSpeed(meanSpeed),
         mTransferredBytes(transferredBytes),
         mPublicNode(publicNode), mFileType(fileType),
         mParentHandle(parentHandle), mNodeHandle(nodeHandle),mMegaApi(megaApi), mFilename(fileName), mPath(path){}
};
Q_DECLARE_TYPEINFO(TransferData, Q_MOVABLE_TYPE);


class TransferItem2
{
    public:
        TransferItem2() : d(new TransferData){}
        TransferItem2(const TransferItem2& tdr) : d(new TransferData(tdr.d.constData())) {}
        TransferItem2(const TransferData& dataRow) : d(new TransferData(dataRow)) {}

        void updateValuesTransferFinished(int64_t finishTime,
                                          int errorCode, long long errorValue,
                                          long long meanSpeed,
                                          int state, long long transferedBytes,
                                          mega::MegaHandle parentHandle,
                                          mega::MegaHandle nodeHandle,
                                          mega::MegaNode* publicNode);

        void updateValuesTransferUpdated(int64_t remainingTime,
                                         int errorCode, long long errorValue,
                                         long long meanSpeed,
                                         long long speed,
                                         unsigned long long priority,
                                         int state, long long transferedBytes,
                                         mega::MegaNode* publicNode);

        QExplicitlySharedDataPointer<TransferData> getTransferData() const
        {
            return d;
        }

    protected:
            QExplicitlySharedDataPointer<TransferData> d;
};
Q_DECLARE_METATYPE(TransferData::FileTypes)
Q_DECLARE_METATYPE(TransferData)
Q_DECLARE_METATYPE(TransferData*)
Q_DECLARE_METATYPE(TransferItem2)

#endif // TRANSFERITEM2_H
