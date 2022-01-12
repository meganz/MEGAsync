#ifndef MEGATRANSFERDELEGATE_H
#define MEGATRANSFERDELEGATE_H

#include <QStyledItemDelegate>
#include "TransferItem.h"
#include "TransferManagerItem.h"
#include "CustomTransferItem.h"
#include "QTransfersModel.h"

class InfoDialogCurrentTransfersProxyModel;

class MegaTransferDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaTransferDelegate(InfoDialogCurrentTransfersProxyModel *model, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    InfoDialogCurrentTransfersProxyModel *mModel;
    void processCancel(int tag);
    void processShowInFolder(const QModelIndex& index);

private:
    mutable QCache<int, TransferItem> mTransferItems;
};

#endif // MEGATRANSFERDELEGATE_H
