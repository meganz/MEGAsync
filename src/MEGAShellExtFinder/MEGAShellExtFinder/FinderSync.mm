//
//  FinderSync.mm
//  MEGAShellExtFinder
//

#import "FinderSync.h"
#include <string>
#include <unordered_map>
#include <iostream>

static std::unordered_map<std::string , FileState> pathStatus;

void cleanItemsOfFolder(std::string dirPath)
{
    std::unordered_map<std::string, FileState>::iterator it = pathStatus.begin();
    while (it != pathStatus.end())
    {
        std::string tempPath = it->first;
        if (strncmp(dirPath.c_str(),tempPath.c_str(), dirPath.size()) != 0)
        {
            ++it;
            continue;
        }
        
        std::string item = it->first.substr(dirPath.size(), it->first.size() - dirPath.size());
        if (!item.empty())
        {
            it = pathStatus.erase(it);
        }
        else
        {
            ++it;
        }
    }
}

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
    NSString *serverName = @"T9RH74Y7L9.mega.mac.socket";
    _ext = [[ShellExt alloc] initWithServerName:serverName delegate:self];
    _directories = [[NSMutableSet alloc] init];
    _syncNames   = [[NSMutableArray alloc] init];
    _syncPaths   = [[NSMutableArray alloc] init];
    
    NSLog(@"INFO - MEGA Finder Extension launched");
    [_ext start];
    return self;
}

#pragma mark - Primary Finder Sync protocol methods

- (void)beginObservingDirectoryAtURL:(NSURL *)url {
    
    NSLog(@"INFO - beginObservingDirectoryAtURL:%@", url.filePathURL);
    
    std::string path = url.path.precomposedStringWithCanonicalMapping.UTF8String;
    if (path.back() != '/')
    {
        path.push_back('/');
    }
    
    if (pathStatus.find(path) != pathStatus.end())
    {
        return;
    }
    
    pathStatus.emplace(path, FileState::FILE_NONE);
}


- (void)endObservingDirectoryAtURL:(NSURL *)url {

    NSLog(@"INFO - endObservingDirectoryAtURL:%@", url.filePathURL);
    
    std::string path = url.path.precomposedStringWithCanonicalMapping.UTF8String;
    if (path.back() != '/')
    {
        path.push_back('/');
    }
    
    cleanItemsOfFolder(path);
}

- (void)requestBadgeIdentifierForURL:(NSURL *)url {
    
    //NSLog(@"requestBadgeIdentifierForURL:%@", url.filePathURL);
    NSMutableString *path = [[NSMutableString alloc] initWithString:[url path]];
    if ([self isDirectory:url])
    {
        [path appendString:@"/"];
    }
    
    [_ext sendRequest:path type:@"P"];
    pathStatus.emplace(path.precomposedStringWithCanonicalMapping.UTF8String, FileState::FILE_NONE);
}

#pragma mark - Menu and toolbar item support

// Disable menu and toolbar options

- (NSString *)toolbarItemName {
    return NSLocalizedString(@"MEGA Finder Extension", nil);
}

- (NSString *)toolbarItemToolTip {
    return NSLocalizedString(@"Click the toolbar item for a menu.", nil);
}

- (NSImage *)toolbarItemImage {
    return [[NSBundle mainBundle] imageForResource:@"toolbar-icon.icns"];
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
        if ([self isDirectory:url])
        {
            [path appendString:@"/"];
        }
        
        std::unordered_map<std::string, FileState>::iterator it = pathStatus.find(path.precomposedStringWithCanonicalMapping.UTF8String);
        if (it != pathStatus.end() && it->second == FileState::FILE_SYNCED)
        {
            if ([self isDirectory:url])
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
    
    if ((selectedItemURLs.count == 1) && (numFolders + numFiles == 1)) // If only one item is selected and already synced.
    {
        NSMenuItem *viewOnMEGAItem = [[NSMenuItem alloc] initWithTitle:NSLocalizedString(@"View on MEGA", nil) action:nil keyEquivalent:@""];
        [menu setAutoenablesItems:NO];
        [viewOnMEGAItem setEnabled:YES];
        [viewOnMEGAItem setImage:(whichMenu == FIMenuKindContextualMenuForItems) ? icon : nil];
        [viewOnMEGAItem setAction:@selector(viewOnMEGA:)];
        [menu addItem:viewOnMEGAItem];
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
    
    NSURL *target = [[FIFinderSyncController defaultController] targetedURL];
    if (!target)
    {
        return;
    }
    
    [_ext sendRequest:target.path type:@"V"];
}

#pragma mark - Sync notifications

- (void)onSyncAdd:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"INFO - adding sync folder path:%@", path);
    [_directories addObject:[NSURL fileURLWithPath:path]];
    
    if (![_syncNames containsObject:syncName])
    {
        [_syncNames addObject:syncName];
        [_syncPaths addObject:path];
    }
    [FIFinderSyncController defaultController].directoryURLs = _directories;
}

- (void)onSyncDel:(NSString *)path withSyncName:(NSString*)syncName{
    
    NSLog(@"INFO - removing sync folder path:%@", path);
    [_directories removeObject:[NSURL fileURLWithPath:path]];
    [_syncNames removeObject:syncName];
    [_syncPaths removeObject:path];
    [FIFinderSyncController defaultController].directoryURLs = _directories;
}

- (void)onItemChanged:(NSString *)urlPath withState:(int)state {
    
//    NSLog(@"settingBadge: %i for path:%@", state, urlPath);
    
    std::string path = urlPath.precomposedStringWithCanonicalMapping.UTF8String;
    std::unordered_map<std::string, FileState>::iterator it = pathStatus.find(path);
    if (it == pathStatus.end())
    {
        pathStatus.emplace(path, (FileState)state);
    }
    else
    {
        it->second = (FileState)state;
    }
    
    [[FIFinderSyncController defaultController] setBadgeIdentifier:[self badgeIdentifierFromCode:state] forURL:[NSURL fileURLWithPath:urlPath]];
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

- (void) cleanAll {
    
    [_directories removeAllObjects];
    [_syncNames removeAllObjects];
    [_syncPaths removeAllObjects];
    [FIFinderSyncController defaultController].directoryURLs = nil;
}

- (bool) isDirectory:(NSURL *)url
{
    NSNumber *isDirectory;
    BOOL success = [url getResourceValue:&isDirectory forKey:NSURLIsDirectoryKey error:nil];
    if (success && [isDirectory boolValue])
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

