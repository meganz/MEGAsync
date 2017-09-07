#ifndef CLIENTSIDE_H
#define CLIENTSIDE_H

#import <Cocoa/Cocoa.h>
#include "MacXLocalSocketPrivate.h"

@interface ClientSide : NSObject<CommunicationProtocol>

- (instancetype)initWithLocalSocket:(MacXLocalSocketPrivate*)lSocket;

@end

#endif // CLIENTSIDE_H
