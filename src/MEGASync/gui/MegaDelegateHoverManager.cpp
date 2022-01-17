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

        if(mCurrentIndex.row() != index.row())
        {
            sendLeaveEvent();
            mCurrentIndex = index;
            sendEnterEvent();
        }

        sendMoveEvent(mouseEvent->pos());
    }
    else if(event->type() == QEvent::Enter)
    {
        if(auto enterEvent = dynamic_cast<QEnterEvent*>(event))
        {
            mCurrentIndex = mView->indexAt(enterEvent->pos());
            sendEnterEvent();
        }
    }
    else if(event->type() == QEvent::Leave)
    {
        sendLeaveEvent();
        mCurrentIndex = QModelIndex();
    }
    else if(event->type() == QEvent::Wheel)
    {
        sendLeaveEvent();
        mCurrentIndex = QModelIndex();
    }

    return QObject::eventFilter(watched, event);
}

void MegaDelegateHoverManager::sendEnterEvent()
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
            auto enterEvent = new MegaDelegateHoverEvent(QEvent::Enter);
            enterEvent->setIndex(mCurrentIndex);
            enterEvent->setRect(mView->visualRect(mCurrentIndex));
            QApplication::postEvent(delegate, enterEvent);
        }
    }
}

void MegaDelegateHoverManager::sendLeaveEvent()
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
            auto leaveEvent = new MegaDelegateHoverEvent(QEvent::Leave);
            leaveEvent->setIndex(mCurrentIndex);
            leaveEvent->setRect(mView->visualRect(mCurrentIndex));
            QApplication::postEvent(delegate, leaveEvent);
        }
    }
}

void MegaDelegateHoverManager::sendMoveEvent(const QPoint& point)
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
            auto mouseMoveEvent = new MegaDelegateHoverEvent(QEvent::MouseMove);
            mouseMoveEvent->setIndex(mCurrentIndex);
            mouseMoveEvent->setRect(mView->visualRect(mCurrentIndex));
            mouseMoveEvent->setMousePos(point - mouseMoveEvent->rect().topLeft());
            QApplication::postEvent(delegate, mouseMoveEvent);
        }
    }
}
