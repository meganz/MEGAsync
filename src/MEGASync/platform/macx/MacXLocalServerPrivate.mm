#include "MacXLocalServerPrivate.h"
#include "ServerSide.h"

MacXLocalServerPrivate::MacXLocalServerPrivate()
{
    connection = [[NSConnection alloc] init];
    server = [[ServerSide alloc] initWithLocalServer:this];
    [connection setRootObject:server];
}

MacXLocalServerPrivate::~MacXLocalServerPrivate()
{
    [server setServerSocketPrivate:nil];
    [server release];
    [connection release];
}
