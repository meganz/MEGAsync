#include "ServerSide.h"
#include "MacXLocalSocketPrivate.h"

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
        MacXLocalSocket *client = new MacXLocalSocket(_serverSocketPrivate->localServer, clientSocketPrivate);
        _serverSocketPrivate->localServer->appendPendingConnection(client);
        emit _serverSocketPrivate->localServer->newConnection();
        [endPoint registerObject:clientSocketPrivate->client];
    }
}

@end
