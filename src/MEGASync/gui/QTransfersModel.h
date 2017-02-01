#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include <QCache>
#include "TransferItem.h"
#include <megaapi.h>
#include "QTMegaTransferListener.h"
#include <deque>

class TransferItemData
{
public:
    int tag;
    unsigned long long priority;
};

typedef std::deque<TransferItemData*>::iterator transfer_it;

class QTransfersModel : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    enum {
        TYPE_DOWNLOAD = 0,
        TYPE_UPLOAD,
        TYPE_FINISHED
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
};

#endif // QTRANSFERSMODEL_H
