//
//  ShellExt.h
//  MEGAFinderSync
//

#import <Foundation/Foundation.h>
#import <FinderSync/FinderSync.h>
#import "Interfaces.h"


@interface ShellExt : NSObject <CommunicationProtocol>
{
    NSString *_serverName;
}

- (instancetype)initWithServerName:(NSString*)serverName delegate:(FIFinderSync <ShellProtocolDelegate>*)delegate;
- (void)start;
- (void)sendRequest:(NSString*)command type:(NSString*)type;
- (void) connectionDidDie;

@end
