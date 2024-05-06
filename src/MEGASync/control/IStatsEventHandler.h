#ifndef ISTATSEVENTHANDLER_H
#define ISTATSEVENTHANDLER_H

#include "AppStatsEvents.h"

#include "megaapi.h"

#include <QObject>

class IStatsEventHandler
{
public:
    virtual ~IStatsEventHandler() = default;

    virtual void sendEvent(AppStatsEvents::EventTypes type) = 0;
    virtual void sendEvent(AppStatsEvents::EventTypes type, const char* message) = 0;

};

#endif // ISTATSEVENTHANDLER_H
