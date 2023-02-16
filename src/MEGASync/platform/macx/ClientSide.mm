#include "ClientSide.h"

@implementation ClientSide

- (instancetype)initWithLocalSocket:(MacXLocalSocketPrivate*)lSocket
{
    self = [super init];
    _socketPrivate = lSocket;
    return self;
}

- (void)send:(NSData *)msg
{
    if (_socketPrivate)
    {
        emit _socketPrivate->socket->dataReady(QByteArray((const char *)[msg bytes], [msg length]));
    }
}

@end
