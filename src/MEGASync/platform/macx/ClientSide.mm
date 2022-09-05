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
        _socketPrivate->appendToBuffer(QByteArray((const char *)[msg bytes], [msg length]));
        emit _socketPrivate->socket->dataReady();
    }
}

@end
