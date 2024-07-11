#include "NotificationAlertDelegate.h"

#include "NotificationAlertModel.h"
#include "MegaDelegateHoverManager.h"
#include "NotificationAlertProxyModel.h"
#include "MegaUserAlertExt.h"
#include "MegaNotificationExt.h"

#include <QSortFilterProxyModel>
#include <QPainter>
#include <QTreeView>
#include <QTimer>

NotificationAlertDelegate::NotificationAlertDelegate(QAbstractItemModel* proxyModel,
                                                     QTreeView* view)
    : QStyledItemDelegate(view)
    , mAlertsDelegate(std::make_unique<AlertDelegate>())
    , mNotificationsDelegate(std::make_unique<NotificationDelegate>())
    , mProxyModel(qobject_cast<NotificationAlertProxyModel*>(proxyModel))
    , mEditor(std::make_unique<NotificationEditorInfo>())
    , mView(view)
{
}

void NotificationAlertDelegate::paint(QPainter* painter,
                                      const QStyleOptionViewItem& option,
                                      const QModelIndex& index) const
{
    if(mEditor->getWidget() && mEditor->getIndex() == index)
    {
        return;
    }

    if (index.isValid())
    {
        painter->save();
        painter->translate(option.rect.topLeft());

        QWidget* item = getWidget(index);
        if(!item)
        {
            return;
        }

        item->resize(option.rect.width(), option.rect.height());
        item->render(painter, QPoint(0, 0), QRegion(0, 0, option.rect.width(), option.rect.height()));

        painter->restore();
    }
    else
    {
        QStyledItemDelegate::paint(painter, option, index);
    }
}

QSize NotificationAlertDelegate::sizeHint(const QStyleOptionViewItem& option,
                                          const QModelIndex& index) const
{
    QSize result;
    if (index.isValid())
    {
        QWidget* item = getWidget(index);
        if(!item)
        {
            return QSize();
        }
        result = item->sizeHint();
    }
    else
    {
        result = QStyledItemDelegate::sizeHint(option, index);
    }
    return result;
}

QWidget* NotificationAlertDelegate::createEditor(QWidget* parent, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(parent);
    Q_UNUSED(option);

    auto widget = getWidget(index);
    if(widget)
    {
        mEditor->setData(index, widget);
    }

    return mEditor->getWidget();
}

void NotificationAlertDelegate::destroyEditor(QWidget *, const QModelIndex &) const
{
    //Do not destroy it the editor, as it is also used to paint the row and it is saved in a cache
    mEditor->setData(QModelIndex(), nullptr);
}

bool NotificationAlertDelegate::event(QEvent* event)
{
    if(auto hoverEvent = dynamic_cast<MegaDelegateHoverEvent*>(event))
    {
        switch (hoverEvent->type())
        {
            case QEvent::Enter:
            case QEvent::MouseMove:
            {
                onHoverEnter(hoverEvent->index());
                break;
            }
            case QEvent::Leave:
            {
                onHoverLeave(hoverEvent->index());
                break;
            }
            default:
            {
                break;
            }
        }
    }

    return QStyledItemDelegate::event(event);
}

void NotificationAlertDelegate::updateEditorGeometry(QWidget *editor, const QStyleOptionViewItem &option, const QModelIndex &) const
{
    QRect geometry(option.rect);
#ifdef __APPLE__
    auto width = mView->size().width();
    width -= mView->contentsMargins().left();
    width -= mView->contentsMargins().right();
    if(mView->verticalScrollBar() && mView->verticalScrollBar()->isVisible())
    {
        width -= mView->verticalScrollBar()->width();
    }
    geometry.setWidth(std::min(width, option.rect.width()));
#endif
    editor->setGeometry(geometry);
}

void NotificationAlertDelegate::onHoverEnter(const QModelIndex& index)
{
    QModelIndex editorCurrentIndex(mEditor->getIndex());
    if(editorCurrentIndex != index)
    {
        onHoverLeave(index);
        mView->edit(index);
    }
}

void NotificationAlertDelegate::onHoverLeave(const QModelIndex& index)
{
    //It is mandatory to close the editor, as it may be different depending on the row
    if(mEditor->getWidget())
    {
        emit closeEditor(mEditor->getWidget(), QAbstractItemDelegate::EndEditHint::NoHint);

        //Small hack to avoid blinks when changing from editor to delegate paint
        //Set the editor to nullptr and update the view -> Then the delegate paints the base widget
        //before the editor is removed
        mEditor->setData(QModelIndex(), nullptr);
    }

    QTimer::singleShot(50, this, [this, index](){
        mView->update(index);
    });
}

QModelIndex NotificationAlertDelegate::getEditorCurrentIndex() const
{
    if(mEditor)
    {
        return mProxyModel->mapFromSource(mEditor->getIndex());
    }

    return QModelIndex();
}

QWidget* NotificationAlertDelegate::getWidget(const QModelIndex& index) const
{
    QWidget* widget = nullptr;
    if (index.isValid() && index.row() >= 0)
    {
        QModelIndex filteredIndex = ((QSortFilterProxyModel*)index.model())->mapToSource(index);
        if (filteredIndex.isValid() && filteredIndex.row() >= 0)
        {
            NotificationExtBase* item = static_cast<NotificationExtBase*>(filteredIndex.internalPointer());
            if(item)
            {
                switch (item->getType())
                {
                    case NotificationExtBase::Type::ALERT:
                    {
                        MegaUserAlertExt* alert = dynamic_cast<MegaUserAlertExt*>(item);
                        widget = mAlertsDelegate->getWidget(alert, mView->viewport());
                        break;
                    }
                    case NotificationExtBase::Type::NOTIFICATION:
                    {
                        MegaNotificationExt* notification = dynamic_cast<MegaNotificationExt*>(item);
                        widget = mNotificationsDelegate->getWidget(notification, mView->viewport());
                        break;
                    }
                    default:
                    {
                        break;
                    }
                }
            }
        }
    }
    return widget;
}
