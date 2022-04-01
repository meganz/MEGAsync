#include "EventHelper.h"
#include <QVariant>

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
            return mData.func();
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
    EventHelper::Data data;
    data.action = act;
    data.eventType = eType;
    data.object = obj;
    addEvent(data);
}

void EventManager::addEvent(QObject *obj, const QEvent::Type& eType, const std::function<bool ()>& func)
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
    return obj->property(eventName.toStdString().c_str()).value<EventHelper*>();
}

void EventManager::addEvent(const EventHelper::Data& data)
{
    if(!data.object || GetHelper(data.object, data.eventType))
    {
        return;
    }

    EventHelper* helper = new EventHelper(data);
    data.object->setProperty(GetStringFromEventType(data.eventType).toStdString().c_str(),
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
        obj->setProperty(eventName.toStdString().c_str(), QVariant());
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
