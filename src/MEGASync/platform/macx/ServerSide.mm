#include "ServerSide.h"
#include "MacXLocalSocketPrivate.h"

#include <QThread>

@implementation ServerSide
- (instancetype)initWithLocalServer:(MacXLocalServerPrivate*)lServer
{
    self = [super init];
    _serverSocketPrivate = lServer;
    return self;
}

- (void)registerObject:(NSDistantObject <CommunicationProtocol> *)endPoint
{
    if (_serverSocketPrivate)
    {
        [endPoint setProtocolForProxy:@protocol(CommunicationProtocol)];
        MacXLocalSocketPrivate *clientSocketPrivate = new MacXLocalSocketPrivate(endPoint);
        QPointer<MacXLocalSocket> client = new MacXLocalSocket(clientSocketPrivate);
        client->moveToThread(_serverSocketPrivate->localServer->thread());
        emit _serverSocketPrivate->localServer->newConnection(client);
        [endPoint registerObject:clientSocketPrivate->client];
    }
}

@end
