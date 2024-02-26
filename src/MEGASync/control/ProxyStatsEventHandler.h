#ifndef PROXYSTATSEVENTHANDLER_H
#define PROXYSTATSEVENTHANDLER_H

#include "IStatsEventHandler.h"
#include <QObject>

class ProxyStatsEventHandler : public IStatsEventHandler
{
    Q_OBJECT

public:
    using IStatsEventHandler::IStatsEventHandler;
    void sendEvent(int eventType, const char *message, bool addJourneyId, const char *viewId, mega::MegaRequestListener *listener = nullptr) override;

protected:
    virtual bool canSend() const;
};

#endif // PROXYSTATSEVENTHANDLER_H
