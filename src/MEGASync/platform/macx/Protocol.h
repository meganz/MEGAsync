#ifndef MACXPROTOCOL_H
#define MACXPROTOCOL_H

#import <Cocoa/Cocoa.h>

@protocol CommunicationProtocol <NSObject>
- (void)send:(NSData *)msg;
@end

@protocol RegistrationProtocol <NSObject>
- (void)registerObject:(id)endPoint;
@end

#endif
