#ifndef QTRANSFERSMODEL2_H
#define QTRANSFERSMODEL2_H

#include "QTMegaTransferListener.h"
#include "TransferItem2.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QLinkedList>
#include <QtConcurrent/QtConcurrent>

class QTransfersModel2 : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    explicit QTransfersModel2(QObject* parent = 0);
    ~QTransfersModel2();

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                                int column, const QModelIndex& parent);
    bool hasChildren(const QModelIndex& parent) const;
    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                  const QModelIndex& destinationParent, int destinationChild);
    bool areAllPaused();

    void getLinks(QList<int>& rows);
    void cancelClearTransfers(const QModelIndexList& indexes,  bool cancel = true, bool clear = true);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForState(TransferData::TransferState state) const;
    long long  getNumberOfTransfersForType(TransferData::TransferType type) const;
    long long  getNumberOfTransfersForFileType(TransferData::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(TransferData::FileType fileType) const;

    void initModel();

    void startTransfer(mega::MegaTransfer* transfer);
    void updateTransfer(mega::MegaTransfer* transfer);
    void finishTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer,
                        mega::MegaError* error);
    void transferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError *error);

    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi* api,mega::MegaTransfer* transfer,mega::MegaError* error);

signals:
    void transfersInModelChanged(bool weHaveTransfers);
    void pauseStateChanged(bool pauseState);

public slots:
    void onRetryTransfer(TransferTag tag);
    void pauseResumeAllTransfers();
    void cancelClearAllTransfers();

private slots:
    void onPauseStateChanged();
    void onTimerTransfers();

private:

    static constexpr int INIT_ROWS_PER_CHUNK = 1000;

    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;

    QMap<TransferTag, QVariant> mTransfers;
    QMap<TransferTag, mega::MegaTransfer*> mFailedTransfers;
    QMap<TransferTag, TransferRemainingTime*> mRemainingTimes;
   // std::deque<TransferTag> mOrder;
    QList<TransferTag> mOrder;
    ThreadPool* mThreadPool;
    QHash<QString, TransferData::FileType> mFileTypes;
//    QMutex* mModelMutex;
    QReadWriteLock* mModelMutex;

    QFuture<void> mInitFuture;

    long long mNotificationNumber;

    bool mAreAllPaused;

    bool mModelHasTransfers;
    QMap<TransferData::FileType, long long> mNbTransfersPerFileType;
    QMap<TransferData::FileType, long long> mNbFinishedPerFileType;
    QMap<int, long long> mNbTransfersPerType;
    QMap<TransferData::TransferState, long long> mNbTransfersPerState;

    void insertTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer, int row, bool signal = true);
    void addTransfers(int rows);

    mega::QTMegaTransferListener* mListener;

    QList<mega::MegaTransfer*> mCacheStartTransfers;
    QMap<TransferTag,mega::MegaTransfer*> mCacheUpdateTransfers;
    QMap<TransferTag,mega::MegaTransfer*> mCacheFinishTransfers;
    QTimer timer;
    int mCurrentTransfers;
};

#endif // QTRANSFERSMODEL2_H
