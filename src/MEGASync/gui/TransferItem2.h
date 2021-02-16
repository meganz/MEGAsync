#ifndef TRANSFERITEM2_H
#define TRANSFERITEM2_H

#include <QTransfersModel2.h>

#include <QWidget>

class TransferItem2 : public QWidget
{
    Q_OBJECT
public:

    explicit TransferItem2(TransferDataRow& transferData, QWidget *parent = 0);

    virtual void paint(QPainter *painter, const QRect &rect) const;

signals:

protected:
    TransferDataRow mTransferData;

};

#endif // TRANSFERITEM2_H
