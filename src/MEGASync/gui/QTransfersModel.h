#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include "QTMegaTransferListener.h"
#include "TransferItem.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>
#include <QLinkedList>
#include <QtConcurrent/QtConcurrent>

#include <set>

struct TransfersCount
{
    long long leftUploadBytes;
    long long completedUploadBytes;
    long long leftDownloadBytes;
    long long completedDownloadBytes;

    int activeDownloadState;
    int activeUploadState;
    int remainingUploads;
    int remainingDownloads;

    int totalUploads;
    int totalDownloads;
    int currentUpload;
    int currentDownload;

    TransfersCount():
        leftUploadBytes(0),
        completedUploadBytes(0),
        leftDownloadBytes(0),
        completedDownloadBytes(0),
        activeDownloadState(0),
        activeUploadState(0),
        remainingUploads(0),
        remainingDownloads(0),
        totalUploads(0),
        totalDownloads(0),
        currentUpload(0),
        currentDownload(0)
    {}
};

class QTransfersModel : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    explicit QTransfersModel(QObject* parent = 0);
    ~QTransfersModel();

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
    void openFolderByIndex(const QModelIndex& index);
    void cancelClearTransfers(const QModelIndexList& indexes,  bool cancel = true, bool clear = true);
    void pauseTransfers(const QModelIndexList& indexes, bool pauseState);
    void pauseResumeTransferByTag(TransferTag tag, bool pauseState);

    void lockModelMutex(bool lock);

    long long  getNumberOfTransfersForState(TransferData::TransferState state) const;
    long long  getNumberOfTransfersForType(TransferData::TransferType type) const;
    long long  getNumberOfTransfersForFileType(TransferData::FileType fileType) const;
    long long  getNumberOfFinishedForFileType(TransferData::FileType fileType) const;

    const TransfersCount& getTransfersCount();
    void resetTransfersCount();

    void initModel();

    void startTransfer(std::unique_ptr<mega::MegaTransfer> transfer);
    void updateTransfer(mega::MegaTransfer* transfer);
    void finishTransfer(mega::MegaApi* api, std::unique_ptr<mega::MegaTransfer> transfer,
                        std::unique_ptr<mega::MegaError> error);
    void transferTemporaryError(mega::MegaTransfer *transfer, mega::MegaError *error);

    void onTransferStart(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error);
    void onTransferUpdate(mega::MegaApi*, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi* api,mega::MegaTransfer* transfer,mega::MegaError* error);

signals:
    void pauseStateChanged(bool pauseState);
    void transfersDataUpdated();

public slots:
    void onRetryTransfer(TransferTag tag);
    void pauseResumeAllTransfers();
    void cancelClearAllTransfers();
    bool onTimerTransfers();

private slots:
    void onPauseStateChanged();

private:
    void insertTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer, int row, bool signal = true);
    void updateTransfersCount();

private:
    static constexpr int INIT_ROWS_PER_CHUNK = 5000;
    static constexpr int MAX_TRANSFERS_INSTANT_UPDATE = 1000;

    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;

    QHash<TransferTag, QVariant> mTransfers;
    std::map<TransferTag, std::unique_ptr<mega::MegaTransfer>> mFailedTransfers;
    QMap<TransferTag, TransferRemainingTime*> mRemainingTimes;
    QList<TransferTag> mOrder;
    ThreadPool* mThreadPool;
    QHash<QString, TransferData::FileType> mFileTypes;
    QReadWriteLock* mModelMutex;

    long long mUpdateNotificationNumber;
//    QFuture<void> mInitFuture;

    TransfersCount mTransfersCount;

    bool mAreAllPaused;

    bool mModelHasTransfers;
    QMap<TransferData::FileType, long long> mNbTransfersPerFileType;
    QMap<TransferData::FileType, long long> mNbFinishedPerFileType;
    QMap<int, long long> mNbTransfersPerType;
    QMap<TransferData::TransferState, long long> mNbTransfersPerState;

    QTimer timer;

    mega::QTMegaTransferListener* mListener;

    std::map<TransferTag, std::unique_ptr<mega::MegaTransfer>> mCacheStartTransfers;
    std::list<std::pair<std::unique_ptr<mega::MegaTransfer>, std::unique_ptr<mega::MegaError>>> mCacheFinishedTransfers;
    std::map<TransferTag, std::pair<std::unique_ptr<mega::MegaTransfer>, std::unique_ptr<mega::MegaError>>> mCacheUpdateTransfers;
    std::set<TransferTag> mUpdatedTransfers;
};

#endif // QTRANSFERSMODEL_H
