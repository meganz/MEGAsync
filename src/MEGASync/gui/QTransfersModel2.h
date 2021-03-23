#ifndef QTRANSFERSMODEL2_H
#define QTRANSFERSMODEL2_H

#include "QTMegaTransferListener.h"
#include "TransferItem2.h"
#include "TransferRemainingTime.h"

#include <megaapi.h>

#include <QAbstractItemModel>

class QTransfersModel2 : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    explicit QTransfersModel2(QObject* parent = 0);

    virtual Qt::ItemFlags flags(const QModelIndex& index) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual QMimeData* mimeData(const QModelIndexList& indexes) const;
    virtual bool dropMimeData(const QMimeData* data, Qt::DropAction action, int destRow,
                                                int column, const QModelIndex& parent);

    int rowCount(const QModelIndex& parent) const;
    int columnCount(const QModelIndex& parent = QModelIndex()) const;
    QVariant data(const QModelIndex& index, int role) const;
    QModelIndex parent(const QModelIndex& index) const;
    QModelIndex index(int row, int column, const QModelIndex& parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex& parent = QModelIndex());
    bool moveRows(const QModelIndex& sourceParent, int sourceRow, int count,
                  const QModelIndex& destinationParent, int destinationChild);
    bool areDlPaused();
    bool areUlPaused();
    void getLinks(QList<int>& rows);
    void cancelClearTransfers(QModelIndexList& indexes);
    void pauseTransfers(QModelIndexList& indexes, bool pauseState);
    void pauseResumeAllTransfers();
    void pauseResumeDownloads();
    void pauseResumeUploads();
    void cancelAllTransfers();

    long long  getNumberOfTransfersForState(int state);
    long long  getNumberOfTransfersForType(int type);
    long long  getNumberOfTransfersForFileType(TransferData::FileTypes fileType);

    ~QTransfersModel2();

    void initModel();

    void onTransferStart(mega::MegaApi* api, mega::MegaTransfer* transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer* transfer, mega::MegaError* error);
    void onTransferUpdate(mega::MegaApi* api, mega::MegaTransfer* transfer);
    void onTransferTemporaryError(mega::MegaApi* api,mega::MegaTransfer* transfer, mega::MegaError* error);

signals:
    void transfersInModelChanged(bool weHaveTransfers);

public slots:
    void onRetryTransfer(TransferTag tag);

private slots:
    void onPauseStateChanged();

private:
    static constexpr int INIT_ROWS_PER_CHUNK = 500;

    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;

    QMap<TransferTag, QVariant> mTransfers;
    QMap<TransferTag, mega::MegaTransfer*> mFailedTransfers;
    QMap<TransferTag, TransferRemainingTime*> mRemainingTimes;
    QList<TransferTag> mOrder;
    ThreadPool* mThreadPool;
    QHash<QString, TransferData::FileTypes> mFileTypes;
    QMutex mModelMutex;

    long long mNotificationNumber;

    bool mAreDlPaused;
    bool mAreUlPaused;
    bool mAreAllPaused;

    QMap<TransferData::FileTypes, long long> mNbTransfersPerFileType;
    QMap<int, long long> mNbTransfersPerType;
    QMap<int, long long> mNbTransfersPerState;

    void insertTransfer(mega::MegaApi* api, mega::MegaTransfer* transfer, int row);
};

#endif // QTRANSFERSMODEL2_H
