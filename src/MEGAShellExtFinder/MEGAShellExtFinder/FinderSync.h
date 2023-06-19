//
//  FinderSync.h
//  MEGAShellExtFinder
//

#import <Cocoa/Cocoa.h>
#import <FinderSync/FinderSync.h>
#import "Interfaces.h"
#import "ShellExt.h"

typedef enum {
    FILE_NONE    = 0,
    FILE_SYNCED   = 1,
    FILE_PENDING  = 2,
    FILE_SYNCING  = 3,
    FILE_IGNORED = 4,
} FileState;

struct NodeInfo
{
    FileState state;
    bool isIncomingShare;
};

const NSString* OP_PATH_STATE = @"P";
const NSString* OP_UPLOAD     = @"F";
const NSString* OP_LINK       = @"L";
const NSString* OP_END        = @"E";
const NSString* OP_STRING     = @"T";

@interface FinderSync : FIFinderSync <ShellProtocolDelegate>

@property NSMutableSet *syncPaths;
@property NSMutableArray *syncNames;
@property NSMutableArray *directories;
@property ShellExt *ext;

@end
