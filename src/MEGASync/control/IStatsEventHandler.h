#ifndef ISTATSEVENTHANDLER_H
#define ISTATSEVENTHANDLER_H

#include "AppStatsEvents.h"

#include "megaapi.h"

class IStatsEventHandler
{
public:
    IStatsEventHandler(mega::MegaApi* megaApi)
        : mMegaApi(megaApi) {;}

    virtual ~IStatsEventHandler() = default;

    virtual void sendEvent(AppStatsEvents::EventTypes type) = 0;
    virtual void sendEvent(AppStatsEvents::EventTypes type,
                           const QStringList& args,
                           bool encode = false) = 0;

protected:
    mega::MegaApi* mMegaApi;

    virtual void sendEvent(AppStatsEvents::EventTypes type, const char* message) = 0;

};

#endif // ISTATSEVENTHANDLER_H
