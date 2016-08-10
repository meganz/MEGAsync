#ifndef MEGATRANSFERVIEW_H
#define MEGATRANSFERVIEW_H

#include <QTreeView>
#include "TransferItem.h"

class MegaTransferView : public QTreeView
{
    Q_OBJECT
public:
    MegaTransferView(QWidget *parent = 0);

private:
    int last_row;
    TransferItem *lastItemHovered;

protected:
    virtual void mouseMoveEvent(QMouseEvent *event);

};

#endif // MEGATRANSFERVIEW_H
