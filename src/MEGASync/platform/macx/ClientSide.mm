#include "ClientSide.h"

@implementation ClientSide
{
    MacXLocalSocketPrivate *socketPrivate;
}

- (instancetype)initWithLocalSocket:(MacXLocalSocketPrivate*)lSocket
{
    self = [super init];
    socketPrivate = lSocket;
    return self;
}

- (void)send:(NSData *)msg
{
    if (socketPrivate)
    {
        socketPrivate->buf += QByteArray((const char *)[msg bytes], [msg length]);
        emit socketPrivate->socket->dataReady();
    }
}

- (void)connectionHasDied:(NSNotification*)notification
{
    if (socketPrivate)
    {
        emit socketPrivate->socket->disconnected();
    }
}

@end
