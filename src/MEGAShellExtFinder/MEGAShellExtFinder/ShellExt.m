//
//  ShellExt.m
//  MEGAFinderSync
//

#import "ShellExt.h"

@implementation ShellExt {    
    NSDistantObject <CommunicationProtocol> *_serverConnection;
    FIFinderSync <ShellProtocolDelegate>*_delegate;
    NSTimer* _timeout;
}

- (instancetype)initWithServerName:(NSString*)serverName delegate:delegate{
    
    self = [super init];
    _serverName = serverName;
    _delegate = delegate;
    _timeout = nil;
    _serverConnection = nil;
    return self;
}


#pragma mark - Launch Shell Extension

- (void)start {
    
    NSConnection *_conn = [NSConnection  connectionWithRegisteredName:_serverName host:nil];
    if (!_conn || _conn.isValid == NO)
    {
        NSLog(@"ERROR - Unable to stablish connection with server");
        [self startTimer];
        return;
    }
    
    NSLog(@"INFO - Connection stablished with server");
    [self stopTimer];
    
    @try {
        [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(connectionDidDie)
                                                 name:NSConnectionDidDieNotification
                                               object:_conn];
    
        NSDistantObject <RegistrationProtocol> *server = (NSDistantObject <RegistrationProtocol> *)[_conn rootProxy];
        [server setProtocolForProxy:@protocol(RegistrationProtocol)];
        NSLog(@"INFO - Server object received");
    
        [server registerObject: self];
        NSLog(@"INFO - Sending shell object to server");
    }
    @catch (NSException *exception) {
        NSLog(@"Caught exception %@", exception);
        [self connectionDidDie];
    }
}

#pragma mark - Implement CommunicationProtocol protocol
- (void)send:(NSData *)msg
{
    NSString *command = [[NSString alloc] initWithData:msg encoding:NSUTF8StringEncoding];
    NSLog(@"Command received: %@", command);
    
    NSArray *strings = [command componentsSeparatedByString:@":"];
    if (strings == nil || ![strings count]) {
        return;
    }
    
    NSString *firstChar = [strings objectAtIndex:0];
    if ([firstChar isEqualToString:@"P"]) //Format: <op-code>:<path-file>:<state-code>
    {
        if ([strings count] < 3)
        {
            return;
        }
        [_delegate onItemChanged:[strings objectAtIndex:1]  withState:[[strings objectAtIndex:2] intValue]];
    }
    else if ([firstChar isEqualToString:@"A"]) //Format: <op-code>:<path-file>:<sync-name>
    {
        if ([strings count] < 3)
        {
            return;
        }
        [_delegate onSyncAdd:[strings objectAtIndex:1] withSyncName:[strings objectAtIndex:2]];
    }
    else if ([firstChar isEqualToString:@"D"]) //Format: <op-code>:<path-file>:<sync-name>
    {
        if ([strings count] < 3)
        {
            return;
        }
        [_delegate onSyncDel:[strings objectAtIndex:1] withSyncName:[strings objectAtIndex:2]];
    }
}

- (void)registerObject:(id)endPoint {
   
    _serverConnection = (NSDistantObject <CommunicationProtocol> *)endPoint;
    [_serverConnection setProtocolForProxy:@protocol(CommunicationProtocol)];
}

- (void)sendRequest:(NSString*)command type:(NSString*)type {
    
    char separator = ':';
    int commandLength = (int)[command length];
    NSData *headerLength = [NSData dataWithBytes:&commandLength length:sizeof(int)];
    
    NSMutableData *query = [NSMutableData data];
    [query appendData:[type dataUsingEncoding:NSUTF8StringEncoding]];
    [query appendBytes:&separator length:sizeof(char)];
    [query appendData:headerLength];
    [query appendBytes:&separator length:sizeof(char)];
    [query appendData:[command dataUsingEncoding:NSUTF8StringEncoding]];
     
    @try {
        [_serverConnection send:query];
    }
    @catch (NSException *exception) {
        NSLog(@"Caught exception %@", exception);
        [self connectionDidDie];
    }
}

#pragma mark - Private methods

- (void) startTimer {
    if (!_timeout) {
        _timeout = [NSTimer scheduledTimerWithTimeInterval:5.0f
                                                 target:self
                                                 selector:@selector(start)
                                                 userInfo:nil
                                                 repeats:YES];
    }
}

- (void) stopTimer {
    if (_timeout && [_timeout isValid]) {
        [_timeout invalidate];
    }
    _timeout = nil;
}

- (void) connectionDidDie
{
    _serverConnection = nil;
    [_delegate cleanAll];
    [self startTimer];
}

@end
