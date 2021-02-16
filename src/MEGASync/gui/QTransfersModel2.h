#ifndef QTRANSFERSMODEL2_H
#define QTRANSFERSMODEL2_H

#include "QTMegaTransferListener.h"
#include "Utilities.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>

typedef int TransferTag;


enum FileTypes
{
    TYPE_TEXT  = 0,
    TYPE_AUDIO = 1,
    TYPE_VIDEO = 2,
};

struct TransferData {
    TransferTag           mMegaTransferTag;
    TransferRemainingTime mRemTime;
    FileTypes             mFileType;
    uint64_t              mFinishedTime;
};

struct TransferDataRow {
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

    TransferDataRow(){};

    TransferDataRow(int type, int errorCode, int state, int tag, int errorValue,
                    int64_t finishedTime, int64_t remainingTime, long long totalSize, unsigned long long mPriority,
                    long long speed, long long MeanSpead, long long transferredBytes,
                    int64_t updateTime, bool publicNode, bool isSyncTransfer, FileTypes fileType,
                    QString fileName) :
         mType(type), mErrorCode(errorCode),  mState(state), mTag(tag),
         mErrorValue(errorValue), mFinishedTime(finishedTime), mRemainingTime(remainingTime),
         mTotalSize(totalSize), mPriority(mPriority), mSpeed(speed), mMeanSpeed(MeanSpead),
         mTransferredBytes(transferredBytes), mUpdateTime(updateTime),
         mPublicNode(publicNode), mIsSyncTransfer(isSyncTransfer), mFileType(fileType),
         mFilename(fileName){}
};


Q_DECLARE_METATYPE(TransferDataRow);


class QTransfersModel2 : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    explicit QTransfersModel2(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex parent(const QModelIndex & index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;

    ~QTransfersModel2();

    virtual void removeAllTransfers() = 0;

    void initModel();

    void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error);
    void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    void onTransferTemporaryError(mega::MegaApi *api,mega::MegaTransfer *transfer, mega::MegaError* error);

signals:

private slots:



protected:
    mega::MegaApi* mMegaApi;
    QMap<TransferTag, TransferData*> mTransfers;
    QList<TransferTag> mOrder;
    ThreadPool*    mThreadPool;
};

#endif // QTRANSFERSMODEL2_H
