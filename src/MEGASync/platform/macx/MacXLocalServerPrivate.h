#ifndef MACXLOCALSERVERPRIVATE_H
#define MACXLOCALSERVERPRIVATE_H

#include "Protocol.h"
#include "MacXLocalServer.h"

class MacXLocalServerPrivate
{
public:
    MacXLocalServer *localServer;
    NSConnection *connection;
    id server;

    MacXLocalServerPrivate();
    ~MacXLocalServerPrivate();
};

#endif // MACXLOCALSERVERPRIVATE_H
