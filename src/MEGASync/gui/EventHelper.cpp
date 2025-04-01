#include "EventHelper.h"

#include <QVariant>

/*
 * HOW TO USE THIS CLASS?
 */

/*
 * If you want to block a type of event on a QWidget -> Example: addEvent(mMenu, QEvent::Wheel,
 * Action::BLOCKED) -> This will block all mouse wheel events
 */

/* If you want to block a type of event
 * depending on a more complex logic -> Example: addEvent(mMenu, QEvent::Wheel, const
 * std::function<bool(QEvent*)>& func) -> This will block mouse wheel events depending on the
 * function return, return true when the event is blocked and false when it is accepted
 */

/* If you want to run some logic when an event is received without blocking any event just use the
 * previous example but returning always false in the function provided
 * -> Example: addEvent(mMenu, QEvent::Wheel, const
 * std::function<bool(QEvent*)>& func) -> This will run the function every time a wheel event is
 * received, but as you are returning false from the function, no event is blocked
 */

/* If you want to clear the state and keep it as default:
 * -> Example: addEvent(mMenu, QEvent::Wheel, Action::ACCEPT) -> This will remove the existing state
 * for the object
 */

const QString EventManager::EVENT_PROPERTY = QString::fromUtf8("EVENTMANAGER_EVENT_%1");

EventHelper::EventHelper(const Data &data) : QObject(data.object)
{
    mData.action = data.action;
    mData.eventType = data.eventType;
    mData.func = data.func;
    mData.object = data.object;
    mData.object->installEventFilter(this);
}

bool EventHelper::eventFilter(QObject *obj, QEvent *e)
{
    if(mData.eventType == e->type())
    {
        if(mData.func != nullptr)
        {
            return mData.func(e);
        }
        else if(mData.action == BLOCK)
        {
            return true;
        }
    }
    return QObject::eventFilter(obj, e);
}

bool EventHelper::Data::operator==(const Data& data)
{
    if(data.object == object && data.eventType == eventType)
    {
        return true;
    }

    return false;
}

void EventManager::addEvent(QObject *obj, const QEvent::Type& eType, const EventHelper::Action& act)
{
    if (act == EventHelper::ACCEPT)
    {
        removeEvent(obj, eType);
    }
    else
    {
        EventHelper::Data data;
        data.action = act;
        data.eventType = eType;
        data.object = obj;
        addEvent(data);
    }
}

void EventManager::addEvent(QObject* obj,
                            const QEvent::Type& eType,
                            const std::function<bool(QEvent*)>& func)
{
    EventHelper::Data data;
    data.func = func;
    data.eventType = eType;
    data.object = obj;
    addEvent(data);
}

QString EventManager::GetStringFromEventType(const QEvent::Type& eType)
{
    QString ret = (EVENT_PROPERTY).arg(QString::number(eType));
    return ret;
}

EventHelper* EventManager::GetHelper(QObject *obj, const QEvent::Type &eType)
{
    QString eventName = GetStringFromEventType(eType);
    return obj->property(eventName.toUtf8().constData()).value<EventHelper*>();
}

void EventManager::addEvent(const EventHelper::Data& data)
{
    if(!data.object)
    {
        return;
    }

    if(GetHelper(data.object, data.eventType))
    {
        return;
    }

    EventHelper* helper = new EventHelper(data);
    data.object->setProperty(GetStringFromEventType(data.eventType).toUtf8().constData(),
                             QVariant::fromValue(helper));
}

void EventManager::removeEvent(QObject* obj, const QEvent::Type& eType)
{
    if(!obj)
    {
        return;
    }

    QString eventName = GetStringFromEventType(eType);
    if(auto helper = GetHelper(obj, eType))
    {
        helper->deleteLater();
        obj->setProperty(eventName.toUtf8().constData(), QVariant());
    }
}

void EventManager::removeAllEvents(QObject *obj)
{
    if(!obj)
    {
        return;
    }

    for(int i=0;i<=QEvent::Type::HoverMove;i++)
    {
        removeEvent(obj, QEvent::Type(i));
    }
}
