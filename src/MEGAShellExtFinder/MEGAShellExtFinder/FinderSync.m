//
//  FinderSync.m
//  MEGAShellExtFinder
//

#import "FinderSync.h"

@implementation FinderSync

- (instancetype)init {
    
    self = [super init];

    NSImage * iconSYNCED    = [[NSBundle mainBundle] imageForResource:@"ok.icns" ];
    NSImage * iconERROR = [[NSBundle mainBundle] imageForResource:@"error.icns"];
    NSImage * iconSYNCING  = [[NSBundle mainBundle] imageForResource:@"sync.icns"];

    // Set up images for our badge identifiers.
    [[FIFinderSyncController defaultController] setBadgeImage:iconSYNCED label:@"" forBadgeIdentifier:@"synced"];
    [[FIFinderSyncController defaultController] setBadgeImage:iconERROR label:@"" forBadgeIdentifier:@"pending"];
    [[FIFinderSyncController defaultController] setBadgeImage:iconSYNCING label:@"" forBadgeIdentifier:@"syncing"];
    
    // Server name should contain developer id
    NSString *serverName = @"T9RH74Y7L9.mega.mac.megasync.socket";
    _ext = [[ShellExt alloc] initWithServerName:serverName delegate:self];
    _directories = [[NSMutableSet alloc] init];
    _syncNames   = [[NSMutableArray alloc] init];
    _syncPaths   = [[NSMutableArray alloc] init];
    
    [_ext start];
    return self;
}

#pragma mark - Primary Finder Sync protocol methods

- (void)beginObservingDirectoryAtURL:(NSURL *)url {
    
    NSLog(@"beginObservingDirectoryAtURL:%@", url.filePathURL);
}


- (void)endObservingDirectoryAtURL:(NSURL *)url {

    NSLog(@"endObservingDirectoryAtURL:%@", url.filePathURL);
}

- (void)requestBadgeIdentifierForURL:(NSURL *)url {
    
    NSLog(@"requestBadgeIdentifierForURL:%@", url.filePathURL);
    [_ext sendRequest:[url path] type:@"P"];
}

#pragma mark - Menu and toolbar item support

- (NSString *)toolbarItemName {
    return @"MEGAsync Finder Extension";
}

- (NSString *)toolbarItemToolTip {
    return @"MEGAsync Finder Extension: Click the toolbar item for a menu.";
}

- (NSImage *)toolbarItemImage {
    return [[NSBundle mainBundle] imageForResource:@"app.icns"];
}

- (NSMenu *)menuForMenuKind:(FIMenuKind)whichMenu {
    
    if (whichMenu == FIMenuKindContextualMenuForSidebar)
    {
        return nil;
    }
    
    NSLog(@"Context menu viewed directory: %@", [[FIFinderSyncController defaultController] targetedURL]);
    NSLog(@"Context menu selected items: %@", [[FIFinderSyncController defaultController] selectedItemURLs]);
    
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@""];
    
    if (![_syncNames count])
    {
        NSMenuItem *defaultitem = [[NSMenuItem alloc] initWithTitle:@"No options available" action:nil keyEquivalent:@""];
        [menu setAutoenablesItems:NO];
        [defaultitem setEnabled:NO];
        [menu addItem:defaultitem];
        return menu;
    }
        
    [_syncNames enumerateObjectsUsingBlock:^(id obj, NSUInteger idx, BOOL *stop) {
            
        NSMenuItem *item = [[NSMenuItem alloc] init];
        [item setTitle:[NSString stringWithFormat:@"Open %@",obj]];
        [item setTag:idx];
        [item setAction:@selector(openFinderWithPath:)];
        [menu addItem:item];
        }];

    return menu;
}


- (void)openFinderWithPath:(id)sender{
    
    NSInteger idx = [sender tag];
    if (idx < [_syncPaths count]) {
        [_ext sendRequest:[_syncPaths objectAtIndex:idx] type:@"O"];
    }
}

- (IBAction)sampleAction:(id)sender {
    
    NSURL* target = [[FIFinderSyncController defaultController] targetedURL];
    NSArray* items = [[FIFinderSyncController defaultController] selectedItemURLs];

    NSLog(@"sampleAction: menu item: %@, target = %@, items = ", [sender title], [target filePathURL]);
    [items enumerateObjectsUsingBlock: ^(id obj, NSUInteger idx, BOOL *stop) {
        NSLog(@"    %@", [obj filePathURL]);
    }];
}

#pragma mark - Sync notifications

- (void)onSyncAdd:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"adding sync folder path:%@", path);
    [_directories addObject:[NSURL fileURLWithPath:path]];
    
    if (![_syncNames containsObject:syncName])
    {
        [_syncNames addObject:syncName];
        [_syncPaths addObject:path];
    }
    [FIFinderSyncController defaultController].directoryURLs = _directories;
}

- (void)onSyncDel:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"removing sync folder path:%@", path);
    [_directories removeObject:[NSURL fileURLWithPath:path]];
    [_syncNames removeObject:syncName];
    [_syncPaths removeObject:path];
    [FIFinderSyncController defaultController].directoryURLs = _directories;
}

- (void)onItemChanged:(NSString *)path withState:(int)state {
    
    NSLog(@"settingBadge:%i for path:%@", state, path);
    [[FIFinderSyncController defaultController] setBadgeIdentifier:[self badgeIdentifierFromCode:state] forURL:[NSURL fileURLWithPath:path]];
}

#pragma mark - Private methods

- (NSString *)badgeIdentifierFromCode:(int)code {
    
    switch (code) {
        case FILE_SYNCED:
            return @"synced";
            break;
        case FILE_PENDING:
            return @"pending";
            break;
        case FILE_SYNCING:
            return @"syncing";
            break;
        default:
            break;
    }
    return @"default";
}

- (void) cleanAll
{
    [_directories removeAllObjects];
    [_syncNames removeAllObjects];
    [_syncPaths removeAllObjects];
    [FIFinderSyncController defaultController].directoryURLs = nil;
}

@end

