#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include "TransferItem.h"
#include <megaapi.h>
#include "QTMegaTransferListener.h"

class QTransfersModel : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    enum {
        TYPE_DOWNLOAD = 0,
        TYPE_UPLOAD,
        TYPE_LOCAL_HTTP_DOWNLOAD,
        TYPE_ALL,
        TYPE_FINISHED
    };

    explicit QTransfersModel(int type, QObject *parent = 0);
    void setupModelTransfers(mega::MegaTransferList *transfers);
    TransferItem *transferFromIndex(const QModelIndex &index) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual ~QTransfersModel();

    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);

signals:
    void noTransfers(int type);
    void onTransferAdded();

private:
    void updateTransferInfo(mega::MegaTransfer *transfer);
    void insertTransfer(mega::MegaTransfer *transfer, const QModelIndex &parent);
    void removeTransfer(mega::MegaTransfer *transfer, const QModelIndex &parent);

protected:
    QMap<int, TransferItem*> transfers;
    QList<int> transfersOrder;
    mega::QTMegaTransferListener *delegateListener;
    int type;
};

#endif // QTRANSFERSMODEL_H
