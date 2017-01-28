#ifndef MEGATRANSFERDELEGATE_H
#define MEGATRANSFERDELEGATE_H

#include <QStyledItemDelegate>
#include "TransferItem.h"
#include "QTransfersModel.h"

class MegaTransferDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaTransferDelegate(QTransfersModel *model, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    QTransfersModel *model;
};

#endif // MEGATRANSFERDELEGATE_H
