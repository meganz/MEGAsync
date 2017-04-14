#include "MacXLocalServer.h"
#include "MacXLocalSocket.h"
#include "MacXLocalServerPrivate.h"
#include "Protocol.h"
#include "megaapi.h"
#import <Cocoa/Cocoa.h>

MacXLocalServer::MacXLocalServer()
    :serverPrivate(new MacXLocalServerPrivate())
{
    serverPrivate->localServer = this;
}

MacXLocalServer::~MacXLocalServer()
{
}

bool MacXLocalServer::listen(QString name)
{
    if ([serverPrivate->connection registerName:name.toNSString()] == YES)
    {
        mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_INFO, "Shell ext server started");
        return true;
    }

    mega::MegaApi::log(mega::MegaApi::LOG_LEVEL_ERROR, "Error opening shell ext server");
    return false;
}

MacXLocalSocket* MacXLocalServer::nextPendingConnection()
{
    if (pendingConnections.isEmpty())
    {
        return NULL;
    }

    return pendingConnections.takeFirst();
}

bool MacXLocalServer::hasPendingConnections()
{
    return !pendingConnections.isEmpty();
}

void MacXLocalServer::appenPendingConnection(MacXLocalSocket *client)
{
    pendingConnections.append(client);
}
