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
    [endPoint setProtocolForProxy:@protocol(CommunicationProtocol)];
    MacXLocalSocketPrivate *clientSocketPrivate = new MacXLocalSocketPrivate(endPoint);
    MacXLocalSocket *client = new MacXLocalSocket(_serverSocketPrivate->localServer, clientSocketPrivate);
    _serverSocketPrivate->localServer->appenPendingConnection(client);
    emit _serverSocketPrivate->localServer->newConnection();
    [endPoint registerObject:clientSocketPrivate->client];
}
@end
