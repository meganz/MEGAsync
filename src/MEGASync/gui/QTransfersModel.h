#ifndef QTRANSFERSMODEL_H
#define QTRANSFERSMODEL_H

#include <QAbstractItemModel>
#include "TransferItem.h"
#include <megaapi.h>

class QTransfersModel : public QAbstractItemModel
{
    Q_OBJECT
public:
    explicit QTransfersModel(QObject *parent = 0);

    void insertTransfer(TransferItem *transfer, const QModelIndex &parent);
    TransferItem *transferFromIndex(const QModelIndex &index) const;
    virtual int columnCount(const QModelIndex & parent = QModelIndex()) const;
    QVariant data(const QModelIndex &index, int role) const;
    virtual QModelIndex parent(const QModelIndex & index) const;
    virtual QModelIndex index(int row, int column, const QModelIndex &parent) const;
    virtual int rowCount(const QModelIndex &parent) const;
    virtual ~QTransfersModel();

protected:
    QList<TransferItem*> transfers;
};

#endif // QTRANSFERSMODEL_H
