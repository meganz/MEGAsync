#include "MegaTransferDelegate.h"
#include "control/Utilities.h"
#include "Preferences.h"
#include "gui/QMegaMessageBox.h"
#include "megaapi.h"
#include "TransfersModel.h"
#include "MegaApplication.h"
#include "platform/Platform.h"
#include "TransfersWidget.h"
#include "TransferManagerDelegateWidget.h"
#include "MegaDelegateHoverManager.h"

#include <QPainter>
#include <QEvent>
#include <QMessageBox>
#include <QPainterPath>
#include <QToolTip>
#include <QSortFilterProxyModel>

using namespace mega;

//////

MegaTransferDelegate::MegaTransferDelegate(TransfersSortFilterProxyBaseModel* model,  QAbstractItemView* view)
    : QStyledItemDelegate(view),
      mProxyModel (model),
      mSourceModel (qobject_cast<TransfersModel*>(
                        mProxyModel->sourceModel())),
      mView (view)
{
}

void MegaTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{   
    auto rowCount (index.model()->rowCount());
    auto row (index.row());

    if (index.isValid() && row < rowCount)
    {
        auto pos (option.rect.topLeft());
        auto width (option.rect.width());
        auto height (option.rect.height());
        auto transferItem (qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
        auto data = transferItem.getTransferData();

        TransferBaseDelegateWidget* w (getTransferItemWidget(index, option.rect.size()));
        if(!w)
        {
            return;
        }

        // Move if position changed
        if (w->pos() != pos)
        {
            w->move(pos);
        }

        // Resize if window resized
        if (w->width() != width)
        {
            w->resize(width, height);
        }

        w->updateUi(data, row);

        // Draw border if selected
        if (option.state & QStyle::State_Selected)
        {
            static const QPen pen (QColor::fromRgbF(0.84, 0.84, 0.84, 1), 1);
            QPainterPath path;
            path.addRoundedRect(QRectF(option.rect.x() + 16.,
                                       option.rect.y() + 4.,
                                       width - 17.,
                                       height - 7.),
                                10, 10);
            painter->setPen(pen);
            painter->fillPath(path, Qt::white);
            painter->drawPath(path);
        }
        painter->save();

        painter->translate(pos);
        w->render(painter, QRegion(0, 0, width, height));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

bool MegaTransferDelegate::event(QEvent *event)
{
    if(auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        if(hoverEvent->type() == QEvent::Leave)
        {
            onHoverLeave(hoverEvent->index(),hoverEvent->rect());
        }
        else if(hoverEvent->type() == QEvent::Enter)
        {
            onHoverEnter(hoverEvent->index(), hoverEvent->rect());
        }
        else if(hoverEvent->type() == QEvent::MouseMove)
        {
            onHoverMove(hoverEvent->index(), hoverEvent->rect(), hoverEvent->mousePos());
        }
    }

    return QStyledItemDelegate::event(event);
}

TransferBaseDelegateWidget *MegaTransferDelegate::getTransferItemWidget(const QModelIndex& index, const QSize& size) const
{ 
    auto nbRowsMaxInView(1);
    if(size.height() > 0)
    {
        nbRowsMaxInView = mView->height() / size.height() + 1;
    }
    auto row (index.row() % nbRowsMaxInView);

    TransferBaseDelegateWidget* item(nullptr);

    if(row >= mTransferItems.size())
    {
       item = mProxyModel->createTransferManagerItem(mView);
       mTransferItems.append(item);
    }
    else
    {
        item = mTransferItems.at(row);
    }

    item->setCurrentIndex(index);

    return item;
}

bool MegaTransferDelegate::editorEvent(QEvent* event, QAbstractItemModel*,
                                        const QStyleOptionViewItem& option,
                                        const QModelIndex& index)
{

    //Process the event first to allow the view to select the item in case of mouseButtonRelease
    auto result = QStyledItemDelegate::editorEvent(event, mProxyModel, option, index);

    if (index.isValid())
    {
        switch (event->type())
        {
            case QEvent::MouseButtonRelease:
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                if( me->button() == Qt::LeftButton )
                {
                    TransferBaseDelegateWidget* currentRow (getTransferItemWidget(index, option.rect.size()));
                    auto w (currentRow->childAt(me->pos() - currentRow->pos()));
                    if (w)
                    {
                        auto t (qobject_cast<QToolButton*>(w));
                        if (t)
                        {
                            t->click();
                        }
                    }
                }
                break;
            }
            case QEvent::MouseButtonDblClick:
            {
                QMouseEvent* me = static_cast<QMouseEvent*>(event);
                if( me->button() == Qt::LeftButton )
                {
                    TransferBaseDelegateWidget* currentRow (getTransferItemWidget(index, option.rect.size()));
                    if (currentRow)
                    {
                        QApplication::postEvent(currentRow, new QEvent(QEvent::MouseButtonDblClick));
                    }
                }
                break;
            }
            default:
                break;
        }
    }

    return result;
}

bool MegaTransferDelegate::helpEvent(QHelpEvent* event, QAbstractItemView* view,
                                      const QStyleOptionViewItem& option, const QModelIndex& index)
{
    if (event->type() == QEvent::ToolTip && index.isValid())
    {
        auto currentRow (getTransferItemWidget(index, option.rect.size()));
        auto widget (currentRow->childAt(event->pos() - currentRow->pos()));
        if (widget)
        {
            QToolTip::showText(event->globalPos(), widget->toolTip());
        }
    }
    return QStyledItemDelegate::helpEvent(event, view, option, index);
}

QSize MegaTransferDelegate::sizeHint(const QStyleOptionViewItem&,
                                      const QModelIndex&) const
{
    return QSize(772, 64);
}

void MegaTransferDelegate::onHoverLeave(const QModelIndex& index, const QRect& rect)
{
    auto currentRow (getTransferItemWidget(index, rect.size()));
    if(currentRow)
    {
        currentRow->mouseHoverTransfer(false, QPoint());
    }
}

void MegaTransferDelegate::onHoverEnter(const QModelIndex& index, const QRect& rect)
{
    auto currentRow (getTransferItemWidget(index, rect.size()));
    if(currentRow)
    {
        currentRow->mouseHoverTransfer(true, QPoint());
    }
}

void MegaTransferDelegate::onHoverMove(const QModelIndex &index, const QRect &rect, const QPoint& pos)
{
    auto currentRow (getTransferItemWidget(index, rect.size()));
    if(currentRow)
    {
        auto hoverType = currentRow->mouseHoverTransfer(true, pos);

        if(hoverType != TransferBaseDelegateWidget::ActionHoverType::NONE)
        {
            if(hoverType == TransferBaseDelegateWidget::ActionHoverType::HOVER_ENTER)
            {
               mView->setCursor(Qt::PointingHandCursor);
            }
            else
            {
                if(mView->cursor() != Qt::ArrowCursor)
                {
                    mView->setCursor(Qt::ArrowCursor);
                }
            }

            mView->update(rect);
        }
        else
        {
            if(mView->cursor() != Qt::ArrowCursor)
            {
                mView->setCursor(Qt::ArrowCursor);
            }
        }
    }
}
