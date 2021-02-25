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
    explicit QTransfersModel2(QObject *parent = 0);

    int rowCount(const QModelIndex &parent) const;
    int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    QModelIndex parent(const QModelIndex & index) const;
    QModelIndex index(int row, int column, const QModelIndex &parent = QModelIndex()) const;
    bool removeRows(int row, int count, const QModelIndex &parent = QModelIndex());

    ~QTransfersModel2();

    void initModel();

    void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* error);
    void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    void onTransferTemporaryError(mega::MegaApi *api,mega::MegaTransfer *transfer, mega::MegaError* error);

private slots:
    void onPauseStateChanged();


private:
    mega::MegaApi* mMegaApi;
    Preferences* mPreferences;

    QMap<TransferTag, QVariant> mTransfers;
    QMap<TransferTag, TransferRemainingTime*> mRemainingTimes;
    QList<TransferTag> mOrder;
    ThreadPool*    mThreadPool;
    QHash<QString, TransferData::FileTypes> mFileTypes;

    long long mNotificationNumber;

    bool mAreDlPaused;
    bool mAreUlPaused;
    bool mIsGlobalPaused;

    QMap<int, long long> mNbTransfersPerFileType;
    QMap<int, long long> mNbTransfersPerType;
    QMap<int, long long> mNbTransfersPerState;

    void insertTransfer(mega::MegaApi* api, mega::MegaTransfer *transfer);
};

#endif // QTRANSFERSMODEL2_H
