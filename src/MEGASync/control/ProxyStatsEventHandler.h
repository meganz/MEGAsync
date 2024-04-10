#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"

class ProxyStatsEventHandler : public IStatsEventHandler
{
public:
    void sendEvent(int eventType, const char* message, bool addJourneyId, const char* viewId, mega::MegaRequestListener* listener = nullptr) override;

private:
    bool canSend() const;
};

#endif
