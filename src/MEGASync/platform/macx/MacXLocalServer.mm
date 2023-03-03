#include "MacXLocalServer.h"
#include "MacXLocalSocket.h"
#include "MacXLocalServerPrivate.h"
#include "Protocol.h"
#include "megaapi.h"
#import <Cocoa/Cocoa.h>

using namespace mega;

MacXLocalServer::MacXLocalServer(QObject* parent)
    :QObject(parent),
      serverPrivate(new MacXLocalServerPrivate())
{
    listening = false;
    serverPrivate->localServer = this;
}

MacXLocalServer::~MacXLocalServer()
{
    if (listening)
    {
        [serverPrivate->connection registerName:nil];
    }

    delete serverPrivate;
}

bool MacXLocalServer::listen(QString name)
{
    if ([serverPrivate->connection registerName:name.toNSString()] == YES)
    {
        listening = true;
        MegaApi::log(MegaApi::LOG_LEVEL_INFO, "Shell ext server started");
        return true;
    }

    MegaApi::log(MegaApi::LOG_LEVEL_ERROR, "Error opening shell ext server");
    return false;
}
