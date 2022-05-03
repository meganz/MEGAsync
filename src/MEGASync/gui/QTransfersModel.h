#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include "TransferItem.h"
#include <megaapi.h>
#include "QTMegaTransferListener.h"
#include <deque>
#include <memory>
#include "Utilities.h"

struct TransferCachedData {
    //Fixme: Once initialization is completed, you can get rid off all cached data except tag and priority
    int type;
    QString filename;    
    int errorCode;
    long long errorValue;
    int state;
    long long finishedTime;
    long long totalSize;
    int tag;
    unsigned long long priority;
    long long speed;
    long long meanSpeed;
    long long transferredBytes;
    int64_t updateTime;
    bool publicNode = false;
    bool isSyncTransfer = false;
    int nodeAccess = mega::MegaShare::ACCESS_UNKNOWN;
};

class TransferItemData
{
public:
    TransferItemData(mega::MegaTransfer *transfer = nullptr)
    {
        if (transfer)
        {
            data.type = transfer->getType();
            data.filename = QString::fromUtf8(transfer->getFileName());
            data.isSyncTransfer = transfer->isSyncTransfer();
            data.errorCode = transfer->getLastError().getErrorCode();
            data.errorValue = transfer->getLastErrorExtended() ? transfer->getLastErrorExtended()->getValue() : 0;
            data.state = transfer->getState();
            data.finishedTime = transfer->getUpdateTime();
            data.totalSize = transfer->getTotalBytes();
            data.tag = transfer->getTag();
            data.priority = transfer->getPriority();
            data.speed = transfer->getSpeed();
            data.meanSpeed = transfer->getMeanSpeed();
            data.transferredBytes = transfer->getTransferredBytes();
            data.updateTime = transfer->getUpdateTime();

            std::unique_ptr<mega::MegaNode>publicNode(transfer->getPublicMegaNode());
            data.publicNode = publicNode ? true : false;
        }
    }

    TransferCachedData data;
};

Q_DECLARE_METATYPE(TransferItemData*);

typedef std::deque<TransferItemData*>::iterator transfer_it;

class QTransfersModel : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    enum {
        TYPE_DOWNLOAD = 0,
        TYPE_UPLOAD,
        TYPE_FINISHED,
        TYPE_CUSTOM_TRANSFERS
    };

    explicit QTransfersModel(int type, QObject *parent = 0);

    void refreshTransfers();
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    virtual QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    int getModelType();
    virtual ~QTransfersModel();

    virtual void removeTransferByTag(int transferTag) = 0;
    virtual void removeAllTransfers() = 0;

    void retryAllTransfers();

    virtual mega::MegaTransfer *getTransferByTag(int tag) = 0;

    QCache<int, TransferItem> transferItems;
    mega::MegaApi *megaApi;

signals:
    void noTransfers();
    void onTransferAdded();

private slots:
    virtual void refreshTransferItem(int tag) = 0;

protected:
    QMap<int, TransferItemData*> transfers;
    std::deque<TransferItemData*> transferOrder;
    int type;
    ThreadPool* mThreadPool;
};

#endif // QTRANSFERSMODEL_H
