#ifndef QTRANSFERSMODEL2_H
#define QTRANSFERSMODEL2_H

#include "QTMegaTransferListener.h"
#include "Utilities.h"
#include "TransferRemainingTime.h"
#include "TransferItem2.h"
#include <megaapi.h>

#include <QAbstractItemModel>

class QTransfersModel2 : public QAbstractItemModel, public mega::MegaTransferListener , public mega::MegaRequestListener
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

//    QMap<TransferTag, TransferItem2*> mTransfers;
    QMap<TransferTag, QVariant> mTransfers;
    QList<TransferTag> mOrder;
    ThreadPool*    mThreadPool;
    QHash<QString, FileTypes> mFileTypes;

    long long mNotificationNumber;

    bool mAreDlPaused;
    bool mAreUlPaused;
    bool mIsGlobalPaused;

    void insertTransfer(mega::MegaTransfer *transfer);
};

#endif // QTRANSFERSMODEL2_H
