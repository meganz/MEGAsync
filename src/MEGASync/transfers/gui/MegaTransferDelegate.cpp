#include "MegaTransferDelegate.h"

#include "MegaApplication.h"
#include "MegaDelegateHoverManager.h"
#include "TokenParserWidgetManager.h"
#include "TransferBaseDelegateWidget.h"
#include "TransfersModel.h"

#include <QEvent>
#include <QMouseEvent>
#include <QPainter>
#include <QScrollBar>
#include <QSortFilterProxyModel>
#include <QToolTip>

using namespace mega;

MegaTransferDelegate::MegaTransferDelegate(TransfersSortFilterProxyBaseModel* model,  QAbstractItemView* view)
    : QStyledItemDelegate(view),
      mProxyModel (model),
      mSourceModel (qobject_cast<TransfersModel*>(
                        mProxyModel->sourceModel())),
      mView (view)
{
}

MegaTransferDelegate::~MegaTransferDelegate()
{
    qDeleteAll(mTransferItems);
}

void MegaTransferDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    auto row(index.row());

    if (index.isValid())
    {
        auto pos (option.rect.topLeft());
        auto height (option.rect.height());
        auto transferItem (qvariant_cast<TransferItem>(index.data(Qt::DisplayRole)));
        auto data = transferItem.getTransferData();

        TransferBaseDelegateWidget* w (getTransferItemWidget(index, option.rect.size()));
        if(!w)
        {
            return;
        }

#ifdef Q_OS_MACOS
        auto width = mView->width();
        width -= mView->contentsMargins().left();
        width -= mView->contentsMargins().right();
        if(mView->verticalScrollBar() && mView->verticalScrollBar()->isVisible())
        {
            width -= mView->verticalScrollBar()->width();
        }
#else
        auto width (option.rect.width());
#endif

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

        if(data)
        {
            w->updateUi(data, row);
        }

        painter->save();
        painter->translate(pos);
        w->render(option, painter, QRegion(0, 0, width, height));

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
        switch (static_cast<int>(hoverEvent->type()))
        {
            case MegaDelegateHoverEvent::MouseMove:
            {
                onHoverMove(hoverEvent->index(), hoverEvent->rect(), hoverEvent->mousePos());
                break;
            }
            case MegaDelegateHoverEvent::Leave:
            {
                onHoverLeave(hoverEvent->index(),hoverEvent->rect());
                break;
            }
            case MegaDelegateHoverEvent::Enter:
            {
                onHoverEnter(hoverEvent->index(), hoverEvent->rect());
                break;
            }
            default:
                break;
        }
    }

    return QStyledItemDelegate::event(event);
}

TransferBaseDelegateWidget *MegaTransferDelegate::getTransferItemWidget(const QModelIndex& index, const QSize& size) const
{
    TransferBaseDelegateWidget* item(nullptr);
    
    if(index.isValid())
    {
        auto nbRowsMaxInView(1);
        if(size.height() > 0)
        {
            nbRowsMaxInView = mView->height() / size.height() + 1;
        }
        auto row (index.row() % nbRowsMaxInView);

        if(row >= mTransferItems.size())
        {
            item = mProxyModel->createTransferManagerItem(mView);
            TokenParserWidgetManager::instance()->applyCurrentTheme(item);
            // Setting again its own parent will tell the widget that the stylesheet needs to be
            // reloaded
            item->setParent(item->parentWidget(), item->windowFlags());

            // Refresh completely the widget
            item->show();
            TokenParserWidgetManager::instance()->polish(item);
            item->hide();

            mTransferItems.append(item);
        }
        else
        {
            item = mTransferItems.at(row);
        }

        item->setCurrentIndex(index);
    }
    
    return item;
}

QAbstractButton* MegaTransferDelegate::isButton(TransferBaseDelegateWidget* row, const QPoint& pos)
{
    auto w(row->childAt(pos));
    if (w)
    {
        return qobject_cast<QAbstractButton*>(w);
    }

    return nullptr;
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
                if (me->button() == Qt::LeftButton)
                {
                    TransferBaseDelegateWidget* currentRow(
                        getTransferItemWidget(index, option.rect.size()));
                    if (currentRow)
                    {
                        if (auto button = isButton(currentRow, me->pos() - currentRow->pos()))
                        {
                            button->click();
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
            return true;
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
                if (isButton(currentRow, pos))
                {
                    mView->setCursor(Qt::PointingHandCursor);
                }
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
