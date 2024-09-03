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
        auto length(static_cast<int>([msg length]));
        emit _socketPrivate->socket->dataReady(QByteArray((const char *)[msg bytes], length));
    }
}

@end
