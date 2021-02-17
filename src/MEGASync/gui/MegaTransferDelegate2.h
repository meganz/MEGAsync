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
    MegaTransferDelegate2(QTransfersModel2 *model, QWidget* view, QObject *parent = 0);
    void paint(QPainter *painter, const QStyleOptionViewItem &option,const QModelIndex &index) const;
    QSize sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const;
    bool editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index);
    bool helpEvent(QHelpEvent *event, QAbstractItemView *view, const QStyleOptionViewItem &option, const QModelIndex &index);

protected:
    QTransfersModel2* mModel;

    QMap<int, Ui::TransferManagerItem*> mUis;

    QWidget* mView;

    void processCancel(int tag);
    void processShowInFolder(int tag);

    signals:
        void transferPaused(const TransferTag tag);
        void transferCanceled(const TransferTag tag);


    private slots:
        void on_tCancelTransfer_clicked();
        void on_tPauseTransfer_clicked();
};

#endif // MEGATRANSFERDELEGATE2_H
