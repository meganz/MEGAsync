#include "MacXLocalSocketPrivate.h"
#include "ClientSide.h"

MacXLocalSocketPrivate::MacXLocalSocketPrivate(NSDistantObject<CommunicationProtocol> *extClient)
{
    this->extClient = extClient;
    client = [[ClientSide alloc] initWithLocalSocket:this];
    [extClient retain];

    [[NSNotificationCenter defaultCenter] addObserver:client
            selector:@selector(connectionHasDied:)
            name:NSConnectionDidDieNotification
            object:[extClient connectionForProxy]];
}

MacXLocalSocketPrivate::~MacXLocalSocketPrivate()
{
    [[NSNotificationCenter defaultCenter] removeObserver:client];
    [extClient release];
    [client setSocketPrivate:nil];
    [client release];
}

