#ifndef TRANSFERMANAGERITEM2_H
#define TRANSFERMANAGERITEM2_H

#include "TransferItem2.h"

#include <QWidget>
#include <QDateTime>

namespace Ui {
class TransferManagerItem;
}

class TransferManagerItem2 : public TransferItem2
{
    Q_OBJECT

public:
    explicit TransferManagerItem2(TransferDataRow& transferData, QWidget *parent = 0);
    virtual ~TransferManagerItem2();

    void paint(QPainter *painter, const QRect &rect) const;

private:
    Ui::TransferManagerItem *mUi;
    QString mActiveStatus;

signals:
    void transferPaused(const TransferTag tag);
    void transferCanceled(const TransferTag tag);


private slots:
    void on_tCancelTransfer_clicked();
    void on_tPauseTransfer_clicked();
};

#endif // TRANSFERMANAGERITEM2_H
