#ifndef ISTATSEVENTHANDLER_H
#define ISTATSEVENTHANDLER_H

#include "megaapi.h"

#include <QObject>

class IStatsEventHandler
{
public:
    virtual ~IStatsEventHandler() = default;

    virtual void sendEvent(int eventType, const char *message, bool addJourneyId, const char *viewId, mega::MegaRequestListener *listener = nullptr) = 0;
};

#endif // ISTATSEVENTHANDLER_H
