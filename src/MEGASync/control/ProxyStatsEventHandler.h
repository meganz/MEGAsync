#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"

class ProxyStatsEventHandler : public IStatsEventHandler
{
public:
    void sendEvent(AppStatsEvents::EventTypes type) override;
    void sendEvent(AppStatsEvents::EventTypes type, const char* message) override;

private:
    bool canSend() const;
};

#endif // PROXYSTATSEVENTHANDLER_H
