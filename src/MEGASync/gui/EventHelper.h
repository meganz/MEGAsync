#ifndef EVENTHELPER_H
#define EVENTHELPER_H

#include <QObject>
#include <QEvent>
#include <QPointer>

#include <functional>


class EventHelper : public QObject
{
    Q_OBJECT
public:

    enum Action{
        BLOCK = 0,
        ACCEPT,
    };

public:
    virtual ~EventHelper() {}
    EventHelper(const EventHelper&) = delete;
    EventHelper& operator=(const EventHelper&) = delete;

private:
    struct Data{
      Action action = ACCEPT;
      std::function<bool()> func = nullptr;
      QPointer<QObject> object;
      QEvent::Type eventType;

      bool operator==(const Data &data);
    };

    EventHelper(const Data& data);
    friend class EventManager;
    bool eventFilter(QObject* obj, QEvent* e) override;
    Data mData;
};


class EventManager
{
public:
    static const QString EVENT_PROPERTY;
    static void addEvent(QObject *obj, const QEvent::Type& eType, const EventHelper::Action& act);
    static void addEvent(QObject *obj, const QEvent::Type& eType, const std::function<bool()> &func);
    static void removeEvent(QObject* obj, const QEvent::Type &eType);
    static void removeAllEvents(QObject* obj);

private:
    static void addEvent(const EventHelper::Data& data);
    static QString GetStringFromEventType(const QEvent::Type& eType);
    static EventHelper *GetHelper(QObject* obj, const QEvent::Type& eType);
};

#endif // EVENTHELPER_H
