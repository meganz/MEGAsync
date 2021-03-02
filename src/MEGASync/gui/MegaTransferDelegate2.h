#ifndef MEGATRANSFERDELEGATE2_H
#define MEGATRANSFERDELEGATE2_H

#include "TransferItem2.h"
#include "TransferManagerItem2.h"
#include "QTransfersModel2.h"

#include <QStyledItemDelegate>

class MegaTransferDelegate2 : public QStyledItemDelegate
{
    Q_OBJECT

public:
    MegaTransferDelegate2(QAbstractItemModel *model, QWidget* view, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    QAbstractItemModel* mModel;
    QTransfersModel2* mSourceModel;

    QMap<int, Ui::TransferManagerItem*> mUis;

    QWidget* mView;

    void processCancel(int tag);
    void processShowInFolder(int tag);

signals:
    void transferPaused(const TransferTag tag);
    void transferCanceled(const TransferTag tag);

private slots:
    void onClearTransfers(int firstRow, int amount);

private:
    void setupUi(Ui::TransferManagerItem& ui, const TransferItem2& transferItem, QWidget* w) const;
    void updateUi(Ui::TransferManagerItem& ui, const TransferItem2& transferItem) const;
    TransferManagerItem2 * getTransferItemWidget(int row, int itemHeight) const;
};

#endif // MEGATRANSFERDELEGATE2_H
