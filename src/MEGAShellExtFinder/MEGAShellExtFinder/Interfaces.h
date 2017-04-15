//
//  Interfaces.h
//  MEGAFinderSync
//

@protocol ShellProtocolDelegate <NSObject>
- (void)onSyncAdd:(NSString *)path withSyncName:(NSString*)syncName;
- (void)onSyncDel:(NSString *)path withSyncName:(NSString*)syncName;
- (void)onItemChanged:(NSString *)path withState:(int)state;
- (void)cleanAll;
@end

@protocol CommunicationProtocol <NSObject>
- (void)send:(NSData *)msg;
@end

@protocol RegistrationProtocol <NSObject>
- (void)registerObject:(id)endPoint;
@end
