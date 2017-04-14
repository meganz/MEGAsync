#ifndef SERVERSIDE_H
#define SERVERSIDE_H

#import <Cocoa/Cocoa.h>
#include "MacXLocalServerPrivate.h"

@interface ServerSide : NSObject<RegistrationProtocol>

@property MacXLocalServerPrivate *serverSocketPrivate;

- (instancetype)initWithLocalServer:(MacXLocalServerPrivate*)lServer;

@end

#endif // SERVERSIDE_H
