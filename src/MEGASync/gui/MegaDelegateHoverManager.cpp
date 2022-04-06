#include "MegaDelegateHoverManager.h"

#include <QMouseEvent>
#include <QApplication>

MegaDelegateHoverManager::MegaDelegateHoverManager() : mView(nullptr)
{}

void MegaDelegateHoverManager::setView(QAbstractItemView *view)
{
    if(mView)
    {
        mView->viewport()->removeEventFilter(this);
    }

    mView = view;
    mView->setMouseTracking(true);
    mView->viewport()->installEventFilter(this);
}

bool MegaDelegateHoverManager::eventFilter(QObject *watched, QEvent *event)
{
    if(auto mouseEvent = dynamic_cast<QMouseEvent*>(event))
    {
        auto index = mView->indexAt(mouseEvent->pos());

        if(mCurrentIndex.row() != index.row()
                || mCurrentIndex.parent() != index.parent())
        {
            sendEvent(QEvent::Leave);
            mCurrentIndex = index;
            sendEvent(QEvent::Enter);
        }

        sendEvent(QEvent::MouseMove, mouseEvent->pos());
    }
    else if(event->type() == QEvent::Enter)
    {
        if(auto enterEvent = dynamic_cast<QEnterEvent*>(event))
        {
            mCurrentIndex = mView->indexAt(enterEvent->pos());
            sendEvent(QEvent::Enter);
        }
    }
    else if(event->type() == QEvent::Leave || event->type() == QEvent::Wheel)
    {
        sendEvent(QEvent::Leave);
        mCurrentIndex = QModelIndex();
    }

    return QObject::eventFilter(watched, event);
}

void MegaDelegateHoverManager::sendEvent(QEvent::Type eventType, const QPoint &point)
{
    if(mCurrentIndex.isValid())
    {
        auto delegate = mView->itemDelegateForColumn(mCurrentIndex.column());
        if(!delegate)
        {
            delegate = mView->itemDelegate();
        }
        if(delegate)
        {
            auto hoverEvent = new MegaDelegateHoverEvent(eventType);
            hoverEvent->setIndex(mCurrentIndex);
            hoverEvent->setRect(mView->visualRect(mCurrentIndex));
            if(!point.isNull())
            {
                hoverEvent->setMousePos(point - hoverEvent->rect().topLeft());
            }
            QApplication::postEvent(delegate, hoverEvent);
        }
    }
}
