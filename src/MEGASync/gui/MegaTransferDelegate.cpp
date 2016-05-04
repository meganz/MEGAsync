#include "MegaTransferDelegate.h"
#include <QPainter>
#include <QPushButton>

void MegaTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().canConvert<TransferItem*>())
    {
        // example of simple drawing (selection) before widget
        if (option.state & QStyle::State_Selected)
        {
            painter->fillRect(option.rect, option.palette.highlight());
        }

        TransferItem *ti = qvariant_cast<TransferItem*>(index.data());
        painter->save();
        painter->translate(option.rect.topLeft());
        //ti->render(painter, QPoint(), option.rect, QWidget::DrawChildren);
        ti->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));
        painter->restore();

        }
        else
        {
            QStyledItemDelegate::paint(painter, option, index);
        }

}

QSize MegaTransferDelegate::sizeHint(const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    if (index.data().canConvert<TransferItem*>())
    {
        TransferItem *ti = qvariant_cast<TransferItem*>(index.data());
        return ti->sizeHint();
    }
    else
    {
        return QStyledItemDelegate::sizeHint(option, index);
    }
}
