#include "MegaTransferView.h"
#include <QAbstractItemModel>
#include <QMouseEvent>

MegaTransferView::MegaTransferView(QWidget *parent) :
    QTreeView(parent), last_row(-1)
{
    setMouseTracking(true);
    lastItemHovered = NULL;
}

void MegaTransferView::mouseMoveEvent(QMouseEvent *event)
{
    QAbstractItemModel *model = this->model();
    if (model)
    {
        QModelIndex index = indexAt(event->pos());
        if (index.isValid())
        {
             if (index.row() != last_row)
             {
                last_row = index.row();
                if (lastItemHovered)
                {
                    lastItemHovered->mouseHoverTransfer(false);
                }
                TransferItem *ti = qvariant_cast<TransferItem*>(index.data());
                if (ti)
                {
                    lastItemHovered = ti;
                    ti->mouseHoverTransfer(true);
                }
             }
        }
        else
        {
            if (lastItemHovered)
            {
                lastItemHovered->mouseHoverTransfer(false);
                last_row = -1;
                lastItemHovered = NULL;
            }

        }
    }
    QTreeView::mouseMoveEvent(event);
}
