#ifndef MEGATRANSFERDELEGATE_H
#define MEGATRANSFERDELEGATE_H

#include "TransferItem.h"
#include "TransfersModel.h"

#include <QStyledItemDelegate>
#include <QAbstractItemView>

class TransfersSortFilterProxyBaseModel;
class TransferBaseDelegateWidget;

class MegaTransferDelegate : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaTransferDelegate(TransfersSortFilterProxyBaseModel* model,  QAbstractItemView* view);
    ~MegaTransferDelegate();

    QSize sizeHint(const QStyleOptionViewItem&, const QModelIndex&) const;

protected:
    void paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const override;
    bool event(QEvent *event) override;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index) override;
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index) override;

protected slots:
    void onHoverLeave(const QModelIndex& index, const QRect& rect);
    void onHoverEnter(const QModelIndex& index, const QRect& rect);
    void onHoverMove(const QModelIndex& index, const QRect& rect, const QPoint& point);

private:
    TransferBaseDelegateWidget *getTransferItemWidget(const QModelIndex &index, const QSize &size) const;

    TransfersSortFilterProxyBaseModel* mProxyModel;
    TransfersModel* mSourceModel;
    mutable QVector<TransferBaseDelegateWidget*> mTransferItems;
    QAbstractItemView* mView;
};

#endif // MEGATRANSFERDELEGATE_H
