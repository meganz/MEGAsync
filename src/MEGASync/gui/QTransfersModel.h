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

typedef bool (*comparator_function)(TransferItemData* i, TransferItemData *j);
typedef std::deque<TransferItemData*>::iterator transfer_it;

class QTransfersModel : public QAbstractItemModel, public mega::MegaTransferListener
{
    Q_OBJECT

public:
    static const int MAX_COMPLETED_ITEMS = 20;

    enum {
        TYPE_DOWNLOAD = 0,
        TYPE_UPLOAD,
        TYPE_LOCAL_HTTP_DOWNLOAD,
        TYPE_ALL,
        TYPE_FINISHED
    };

    explicit QTransfersModel(int type, QObject *parent = 0);
    void setupModelTransfers(QList<mega::MegaTransfer *>transfers);
    void setupModelTransfers(mega::MegaTransferData *transferData);
    void insertTransfer(mega::MegaTransfer *transfer);
    void removeTransfer(mega::MegaTransfer *transfer);
    void removeTransferByTag(int transferTag);
    void removeAllTransfers();
    void refreshTransfers();
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    QMimeData *mimeData(const QModelIndexList & indexes) const;
    virtual Qt::ItemFlags flags(const QModelIndex&index) const;
    virtual Qt::DropActions supportedDropActions() const;
    virtual bool dropMimeData(const QMimeData *data, Qt::DropAction action, int row, int column, const QModelIndex &parent);
    virtual ~QTransfersModel();

    int getModelType();
    mega::MegaTransfer* getFinishedTransferByTag(int tag);

    void onTransferPaused(int transferTag, bool pause);
    void onTransferCancel(int transferTag);
    void onMoveTransferToFirst(int transferTag);
    void onMoveTransferUp(int transferTag);
    void onMoveTransferDown(int transferTag);
    void onMoveTransferToLast(int transferTag);

    virtual void onTransferStart(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferFinish(mega::MegaApi* api, mega::MegaTransfer *transfer, mega::MegaError* e);
    virtual void onTransferUpdate(mega::MegaApi *api, mega::MegaTransfer *transfer);
    virtual void onTransferTemporaryError(mega::MegaApi *api, mega::MegaTransfer *transfer, mega::MegaError* e);
    QCache<int, TransferItem> transferItems;
    mega::MegaApi *megaApi;

signals:
    void noTransfers();
    void onTransferAdded();

private:
    void updateTransferInfo(mega::MegaTransfer *transfer);

private slots:
    void refreshTransferItem(int tag);

protected:
    comparator_function priority_comparator;
    QMap<int, TransferItemData*> transfers;
    std::deque<TransferItemData*> transferOrder;
    int type;
};

#endif // QTRANSFERSMODEL_H
