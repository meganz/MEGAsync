//
//  FinderSync.mm
//  MEGAShellExtFinder
//

#import "FinderSync.h"
#include <string>
#include <map>
#include <iostream>

static std::map<std::string , FileState> pathStatus;

// dirPath is the path of a folder, with a trailing '/'
void cleanItemsOfFolder(std::string dirPath)
{
    std::map<std::string, FileState>::iterator it = pathStatus.lower_bound(dirPath);
    while (it != pathStatus.end())
    {
        const std::string &tempPath = it->first;
        if (tempPath.size() < dirPath.size() || strncmp(dirPath.c_str(), tempPath.c_str(), dirPath.size()))
        {
            return;
        }

        size_t pos;
        if (tempPath.size() == dirPath.size()
                || (((pos = tempPath.find('/', dirPath.size())) != std::string::npos)
                    && (pos != (tempPath.size() - 1))))
        {
            it++;
        }
        else
        {
            pathStatus.erase(it++);
        }
    }
}

@implementation FinderSync

- (instancetype)init {
    
    self = [super init];

    NSImage *iconSYNCED = [[NSBundle mainBundle] imageForResource:@"ok.icns" ];
    NSImage *iconERROR = [[NSBundle mainBundle] imageForResource:@"error.icns"];
    NSImage *iconSYNCING = [[NSBundle mainBundle] imageForResource:@"sync.icns"];

    // Set up images for our badge identifiers.
    [[FIFinderSyncController defaultController] setBadgeImage:iconSYNCED label:@"" forBadgeIdentifier:@"synced"];
    [[FIFinderSyncController defaultController] setBadgeImage:iconERROR label:@"" forBadgeIdentifier:@"pending"];
    [[FIFinderSyncController defaultController] setBadgeImage:iconSYNCING label:@"" forBadgeIdentifier:@"syncing"];
      
    // Server name should contain developer id
    NSString *serverName = @"T9RH74Y7L9.mega.mac.socket";
    _ext = [[ShellExt alloc] initWithServerName:serverName delegate:self];
    _syncPaths = [[NSMutableSet alloc] init];
    _syncNames = [[NSMutableArray alloc] init];
    _directories = [[NSMutableArray alloc] init];
    
    NSLog(@"INFO - MEGA Finder Extension launched");
    [_ext start];
    return self;
}

#pragma mark - Primary Finder Sync protocol methods

- (void)beginObservingDirectoryAtURL:(NSURL *)url {
    
    NSLog(@"INFO - beginObservingDirectoryAtURL:%@", url.filePathURL);
    
    NSString *path = url.path;
    if (![_directories containsObject:path])
    {
        [_directories addObject:path];
    }
}


- (void)endObservingDirectoryAtURL:(NSURL *)url {

    NSLog(@"INFO - endObservingDirectoryAtURL:%@", url.filePathURL);
    
    NSString *path = url.path;
    if ([_directories containsObject:path])
    {
        [_directories removeObject:path];
    }
    
    std::string localPath = path.precomposedStringWithCanonicalMapping.UTF8String;
    if (localPath.back() != '/')
    {
        localPath.push_back('/');
    }
    
    cleanItemsOfFolder(localPath);
}

- (void)requestBadgeIdentifierForURL:(NSURL *)url {
    
    //NSLog(@"requestBadgeIdentifierForURL:%@", url.filePathURL);
    NSString *path = [[NSString alloc] initWithString:[url path]];
    if ([self isDirectory:url])
    {
        NSString* folderPath = [path stringByAppendingString:@"/"];
        pathStatus.emplace(folderPath.precomposedStringWithCanonicalMapping.UTF8String, FileState::FILE_NONE);
    }
    else
    {
        pathStatus.emplace(path.precomposedStringWithCanonicalMapping.UTF8String, FileState::FILE_NONE);
    }
    
    [_ext sendRequest:path type:@"P"];
}

#pragma mark - Menu and toolbar item support

// Disable menu and toolbar options

- (NSString *)toolbarItemName {
    return NSLocalizedString(@"MEGA", nil);
}

- (NSString *)toolbarItemToolTip {
    return NSLocalizedString(@"Click the toolbar item for a menu.", nil);
}

- (NSImage *)toolbarItemImage {
    NSImage *im = [[NSBundle mainBundle] imageForResource:@"toolbar-mega-icon"];
    [im setTemplate:YES];
    return im;
}

- (NSMenu *)menuForMenuKind:(FIMenuKind)whichMenu {
    
    int numFiles = 0;
    int numFolders = 0;
    
    if (whichMenu != FIMenuKindContextualMenuForItems && whichMenu != FIMenuKindToolbarItemMenu)
    {
        return nil;
    }
    
//    NSLog(@"Context menu viewed directory: %@", [[[FIFinderSyncController defaultController] targetedURL] path]);
//    NSLog(@"Context menu selected items: %@", [[FIFinderSyncController defaultController] selectedItemURLs]);
    
    NSMenu *menu = [[NSMenu alloc] initWithTitle:@""];
    NSArray *selectedItemURLs = [[FIFinderSyncController defaultController] selectedItemURLs];
    
    if (selectedItemURLs == nil || [selectedItemURLs count] == 0)
    {
        selectedItemURLs = [NSArray arrayWithObjects:[[FIFinderSyncController defaultController] targetedURL], nil];
    }
    
    for (NSURL *url in selectedItemURLs)
    {
        NSMutableString *path = [[NSMutableString alloc] initWithString:[url path]];
        bool urlIsDirectory = [self isDirectory:url];
        if (urlIsDirectory)
        {
            [path appendString:@"/"];
        }
        
        std::map<std::string, FileState>::iterator it = pathStatus.find(path.precomposedStringWithCanonicalMapping.UTF8String);
        if (it != pathStatus.end() && it->second == FileState::FILE_SYNCED)
        {
            if (urlIsDirectory)
            {
                numFolders++;
            }
            else
            {
                numFiles++;
            }
        }
    }
    
    if (!numFolders && !numFiles)
    {
        if (whichMenu == FIMenuKindToolbarItemMenu)
        {
            NSMenuItem *defaultitem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"No options available", nil) action:nil keyEquivalent:@""];
            [menu setAutoenablesItems:NO];
            [defaultitem setEnabled:NO];
            [menu addItem:defaultitem];
            return menu;
        }
        return nil;
    }
    
    NSImage *icon = [NSImage imageNamed:@"context-ico.icns"];
    NSMenuItem *getLinkItem = [[NSMenuItem alloc] initWithTitle:[self createContextStringWithFiles:numFiles folders:numFolders] action:nil keyEquivalent:@""];
    [menu setAutoenablesItems:NO];
    [getLinkItem setEnabled:YES];
    [getLinkItem setImage:(whichMenu == FIMenuKindContextualMenuForItems) ? icon : nil];
    [getLinkItem setAction:@selector(getMEGAlinks:)];
    [menu addItem:getLinkItem];
    
    if (selectedItemURLs.count == 1)
    {
        if (numFolders  == 1) // If only one item is selected and is a folder synced.
        {
            NSMenuItem *viewOnMEGAItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"View on MEGA", nil) action:nil keyEquivalent:@""];
            [menu setAutoenablesItems:NO];
            [viewOnMEGAItem setEnabled:YES];
            [viewOnMEGAItem setImage:(whichMenu == FIMenuKindContextualMenuForItems) ? icon : nil];
            [viewOnMEGAItem setAction:@selector(viewOnMEGA:)];
            [menu addItem:viewOnMEGAItem];
        }
        
        if (numFiles == 1) // If only one file is selected and already synced.
        {
            NSMenuItem *viewPrevVersions = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"View previous versions", nil) action:nil keyEquivalent:@""];
            [menu setAutoenablesItems:NO];
            [viewPrevVersions setEnabled:YES];
            [viewPrevVersions setImage:(whichMenu == FIMenuKindContextualMenuForItems) ? icon : nil];
            [viewPrevVersions setAction:@selector(viewPrevVersions:)];
            [menu addItem:viewPrevVersions];
        }
    }
    
    return menu;
}

#pragma mark - Context menu commands

- (void)getMEGAlinks:(id)sender {

    NSArray *items = [[FIFinderSyncController defaultController] selectedItemURLs];
    if (!items)
    {
        return;
    }
    
    for (NSURL *url in items) {
        [_ext sendRequest:url.path type:@"L"];
    }
    
    [_ext sendRequest:@" " type:@"E"];
}

- (void)viewOnMEGA:(id)sender {
    
    NSArray *items = [[FIFinderSyncController defaultController] selectedItemURLs];
    if (!items || [items count] == 0)
    {
        return;
    }
    
    [_ext sendRequest:[[items firstObject] path] type:@"V"];
}

- (void)viewPrevVersions:(id)sender {
    
    NSArray *items = [[FIFinderSyncController defaultController] selectedItemURLs];
    if (!items || [items count] == 0)
    {
        return;
    }
    
    [_ext sendRequest:[[items firstObject] path] type:@"R"];
}

#pragma mark - Sync notifications

- (void)onSyncAdd:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"INFO - adding sync folder path:%@", path);
    [_syncPaths addObject:[NSURL fileURLWithPath:path isDirectory:true]];
    
    if (![_syncNames containsObject:syncName])
    {
        [_syncNames addObject:syncName];
    }
    [FIFinderSyncController defaultController].directoryURLs = _syncPaths;
}

- (void)onSyncDel:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"INFO - removing sync folder path:%@", path);
    [_syncPaths removeObject:[NSURL fileURLWithPath:path isDirectory:true]];
    [_syncNames removeObject:syncName];
    [FIFinderSyncController defaultController].directoryURLs = _syncPaths;
}

- (void)onItemChanged:(NSString *)urlPath withState:(int)state shouldShowBadges:(int)badge{
    
    //NSLog(@"state: %i for path: %@ showBadge: %i", state, urlPath, badge);
    
    if ([self isDirectory:[NSURL fileURLWithPath:urlPath]])
    {
        urlPath = [urlPath stringByAppendingString:@"/"];
    }
    
    std::string path = urlPath.precomposedStringWithCanonicalMapping.UTF8String;
    std::map<std::string, FileState>::iterator it = pathStatus.find(path);
    if (it != pathStatus.end())
    {
        it->second = (FileState)state;
        if (badge)
        {
            [[FIFinderSyncController defaultController] setBadgeIdentifier:[self badgeIdentifierFromCode:state] forURL:[NSURL fileURLWithPath:urlPath]];
        }
        else
        {
            [[FIFinderSyncController defaultController] setBadgeIdentifier:[self badgeIdentifierFromCode:FILE_NONE] forURL:[NSURL fileURLWithPath:urlPath]];
        }
        
        return;
    }
    
    NSString *basePath = [urlPath stringByDeletingLastPathComponent];
    if ([_directories containsObject:basePath])
    {
        pathStatus.emplace(path, (FileState)state);
        if (badge)
        {
            [[FIFinderSyncController defaultController] setBadgeIdentifier:[self badgeIdentifierFromCode:state] forURL:[NSURL fileURLWithPath:urlPath]];
        }
    }
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
    return @"";
}

- (void) cleanAll {
    
    [_syncPaths removeAllObjects];
    [_syncNames removeAllObjects];
    [FIFinderSyncController defaultController].directoryURLs = nil;
}

- (bool) isDirectory:(NSURL *)url
{
    BOOL isDir = NO;
    if([[NSFileManager defaultManager]fileExistsAtPath:url.path isDirectory:&isDir] && isDir)
    {
        return true;
    }
    
    return false;
}

- (NSString *) createContextStringWithFiles:(int)nFiles folders:(int)nFolders
{
    if (!nFiles && !nFolders)
    {
        return @"";
    }
    
    NSMutableString *cFiles = [[NSMutableString alloc] init];
    if (nFiles == 1)
    {
        [cFiles appendString:NSLocalizedString(@"1 file", nil)];
    }
    else if (nFiles > 1)
    {
        [cFiles appendFormat:NSLocalizedString(@"%i files", nil), nFiles];
    }
    
    NSMutableString *cFolders = [[NSMutableString alloc] init];
    if (nFolders == 1)
    {
        [cFolders appendString:NSLocalizedString(@"1 folder", nil)];
    }
    else if (nFolders > 1)
    {
        [cFolders appendFormat:NSLocalizedString(@"%i folders", nil), nFolders];
    }
    
   NSMutableString *fullString = [[NSMutableString alloc] initWithString:NSLocalizedString(@"Get MEGA link", nil)];
    if (nFiles && nFolders)
    {
        [fullString appendFormat:@" (%@, %@)",cFiles, cFolders];
    }
    else if (nFiles && !nFolders)
    {
        [fullString appendFormat:@" (%@)", cFiles];
    }
    else if (!nFiles && nFolders)
    {
        [fullString appendFormat:@" (%@)", cFolders];
    }
    
    return fullString;
}

@end

