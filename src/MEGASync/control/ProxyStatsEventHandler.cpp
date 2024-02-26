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
#if !defined QT_DEBUG
    return true;
#endif

    return false;
}
