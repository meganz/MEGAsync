#include "ProxyStatsEventHandler.h"

#include "Utilities.h"
#include "MegaApplication.h"

void ProxyStatsEventHandler::sendEvent(int eventType, const char *message, bool addJourneyId, const char *viewId, mega::MegaRequestListener *listener)
{
    if(canSend())
    {
        MegaSyncApp->getMegaApi()->sendEvent(eventType, message, addJourneyId, viewId, listener);
    }
}

bool ProxyStatsEventHandler::canSend() const
{
/*
* Usage : declare the list of not allowed conditions to send stats.
*/
#if !defined QT_DEBUG
    return true;
#else
    return false;
#endif
}
