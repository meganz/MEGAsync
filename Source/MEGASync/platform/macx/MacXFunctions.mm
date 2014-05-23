#include "MacXFunctions.h"
#include <Cocoa/Cocoa.h>

void setMacXActivationPolicy()
{
    //application does not appear in the Dock
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

char *runWithRootPrivileges(char *command)
{
    OSStatus status;
    AuthorizationRef authorizationRef;
    char *result = NULL;

    char* args[2];
    args [0] = "-c";
    args [1] = command;
    args [2] = NULL;

    FILE *pipe = NULL;
    status = AuthorizationCreate(NULL, kAuthorizationEmptyEnvironment,
                                     kAuthorizationFlagDefaults, &authorizationRef);
    if (status != errAuthorizationSuccess)
        return NULL;

    status = AuthorizationExecuteWithPrivileges(authorizationRef, "/bin/sh",
                                                kAuthorizationFlagDefaults, args, &pipe);
    AuthorizationFree(authorizationRef, kAuthorizationFlagDestroyRights);
    if (status == errAuthorizationSuccess)
    {
        result = new char[1024];
        fread(result, 1024, 1, pipe);
        fclose(pipe);
    }

    return result;
}


bool startAtLogin(bool opt)
{
    NSString * appPath = [[NSBundle mainBundle] bundlePath];

    // Get the path for the MEGAsync application and access login items list
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:appPath];
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL,kLSSharedFileListSessionLoginItems, NULL);

    //Enable start at login time
    if(opt)
    {
        if (loginItems) {
            //Insert an item to the login list.
            LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(loginItems,
                                                                         kLSSharedFileListItemLast, NULL, NULL,
                                                                         url, NULL, NULL);
            if (item){
                CFRelease(item);
                CFRelease(loginItems);
                return true;
            }
        }

    }else //disable start at login time
    {
        if (loginItems)
        {
            UInt32 seed = 0U;
            NSArray *currentLoginItems = [NSMakeCollectable(LSSharedFileListCopySnapshot(loginItems, &seed)) autorelease];
            for (id itemObject in currentLoginItems)
            {
                LSSharedFileListItemRef item = (LSSharedFileListItemRef)itemObject;

                //Resolve the item with URL
                if (LSSharedFileListItemResolve(item, 0, (CFURLRef*) &url, NULL) == noErr)
                {
                    NSString * urlPath = [(__bridge NSURL*)url path];
                    if ([urlPath compare:appPath] == NSOrderedSame)
                    {
                        LSSharedFileListItemRemove(loginItems,item);

                        CFRelease(loginItems);
                        return true;
                    }
                }
            }

        }

    }

    CFRelease(loginItems);
    return false;
}

bool isStartAtLoginActive()
{

    Boolean foundIt=false;
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL,kLSSharedFileListSessionLoginItems, NULL);

    if (loginItems)
    {
            // This will get the path for the application
            NSURL *itemURL=[NSURL fileURLWithPath:[[NSBundle mainBundle] bundlePath]];
            UInt32 seed = 0U;
            NSArray *currentLoginItems = [NSMakeCollectable(LSSharedFileListCopySnapshot(loginItems, &seed)) autorelease];
            for (id itemObject in currentLoginItems)
            {
                LSSharedFileListItemRef item = (LSSharedFileListItemRef)itemObject;

                UInt32 resolutionFlags = kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes;
                CFURLRef URL = NULL;
                OSStatus err = LSSharedFileListItemResolve(item, resolutionFlags, &URL, /*outRef*/ NULL);
                if (err == noErr)
                {
                    foundIt = CFEqual(URL, itemURL);
                    CFRelease(URL);

                    if (foundIt)
                        break;
                }
            }
        }

    CFRelease(loginItems);
    return (BOOL)foundIt;
}

void addPathToPlaces(QString path, QString pathName)
{
    IconRef iconRef;
    FSRef fref;

    NSString *folderPath = [[NSString alloc] initWithUTF8String:path.toUtf8().constData()];
    NSString * appPath = [[NSBundle mainBundle] bundlePath];

    //Does not work propertly
    CFStringRef pnString = CFStringCreateWithCharacters(0, (UniChar *)pathName.unicode(), pathName.length());

    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:folderPath];

    CFURLRef iconURLRef = (CFURLRef)[NSURL fileURLWithPath:[appPath stringByAppendingString:@"/Contents/Resources/app.icns"]];
    CFURLGetFSRef(iconURLRef, &fref);
    RegisterIconRefFromFSRef('SSBL', 'ssic', &fref, &iconRef);

    // Create a reference to the shared file list.
    LSSharedFileListRef favoriteItems = LSSharedFileListCreate(NULL,
                                                            kLSSharedFileListFavoriteItems, NULL);
    if (favoriteItems) {
        //Insert an item to the list.
        LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(favoriteItems,
                                                                     kLSSharedFileListItemLast, pnString, iconRef/*NULL*/,
                                                                     url, NULL, NULL);
        if (item){
            CFRelease(item);
        }
    }

    CFRelease(favoriteItems);
    CFRelease(pnString);
    [folderPath release];

}

void removePathFromPlaces(QString path)
{
    NSString *folderPath = [[NSString alloc] initWithUTF8String:path.toUtf8().constData()];

    // This will get the path for the application
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:folderPath];

    // Create a reference to the shared file list of favourite items.
    LSSharedFileListRef favoriteItems = LSSharedFileListCreate(NULL,
                                                            kLSSharedFileListFavoriteItems, NULL);

    // loop through the list of startup items and try to find the MEGAsync app
    CFArrayRef listSnapshot = LSSharedFileListCopySnapshot(favoriteItems, NULL);
    for(int i = 0; i < CFArrayGetCount(listSnapshot); i++)
    {
        LSSharedFileListItemRef item = (LSSharedFileListItemRef)CFArrayGetValueAtIndex(listSnapshot, i);
        //Avoid check special volumes (Airdrop)
        UInt32 resolutionFlags = kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes;
        CFURLRef currentItemURL = NULL;
        LSSharedFileListItemResolve(item, resolutionFlags, &currentItemURL, NULL);
        if(currentItemURL && CFEqual(currentItemURL, url))
        {

            CFRelease(currentItemURL);
            LSSharedFileListItemRemove(favoriteItems,item);
        }
        if(currentItemURL)
        {
            CFRelease(currentItemURL);
        }
    }


    CFRelease(favoriteItems);
    [folderPath release];


}

void setFolderIcon(QString path)
{

    NSString *folderPath = [[NSString alloc] initWithUTF8String:path.toUtf8().constData()];

    NSString * appPath = [[NSBundle mainBundle] bundlePath];
    NSImage* iconImage = [[NSImage alloc] initWithContentsOfFile:[appPath stringByAppendingString:@"/Contents/Resources/folder.icns"]];

    BOOL didSetIcon = [[NSWorkspace sharedWorkspace] setIcon:iconImage forFile:folderPath options:0];

    if(!didSetIcon)
    {
        //[iconImage release];
        [folderPath release];
        return;
    }

    //[iconImage release];
    [folderPath release];
}

void unSetFolderIcon(QString path)
{

    NSString *folderPath = [[NSString alloc] initWithUTF8String:path.toUtf8().constData()];
    BOOL didUnsetIcon = [[NSWorkspace sharedWorkspace] setIcon:nil forFile:folderPath options:0];

    if(!didUnsetIcon)
    {
        [folderPath release];
        return;
    }

    [folderPath release];
}


