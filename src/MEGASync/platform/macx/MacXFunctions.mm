#include "MacXFunctions.h"
#include <Cocoa/Cocoa.h>
#include <QFileInfo>
#include <QCoreApplication>
#include <QWidget>
#include <QProcess>
#include <Preferences/Preferences.h>
#include <QOperatingSystemVersion>

#import <objc/runtime.h>
#import <sys/proc_info.h>
#import <libproc.h>

#include <time.h>
#include <errno.h>
#include <sys/sysctl.h>

// A NSViewController that controls a programatically created view
@interface ProgramaticViewController : NSViewController
@end

@implementation ProgramaticViewController
- (id)initWithView:(NSView *)aView
{
    self = [self initWithNibName:nil bundle:nil];
    self.view = aView;
    return self;
}
@end

void setMacXActivationPolicy()
{
    //application does not appear in the Dock
    [NSApp setActivationPolicy:NSApplicationActivationPolicyAccessory];
}

QString fromNSString(const NSString *str)
{
    if (!str)
    {
        return QString();
    }

    const char *utf8Str = [str UTF8String];
    if (!utf8Str)
    {
        return QString();
    }
    return QString::fromUtf8(utf8Str);
}

QStringList qt_mac_NSArrayToQStringList(void *nsarray)
{
    QStringList result;
    if (!nsarray)
    {
        return result;
    }

    NSArray *array = static_cast<NSArray *>(nsarray);
    for (NSUInteger i = 0; i < [array count]; ++i)
    {
        QString st;
        if ([[array objectAtIndex:i] isKindOfClass:[NSURL class]])
        {
            st = fromNSString([[array objectAtIndex:i] path]);
        }
        else
        {
            st = fromNSString([array objectAtIndex:i]);
        }
       result.append(st);
    }
    return result;
}


void selectorsImpl(QString title, QString defaultDir, bool multiSelection, bool showFiles, bool showFolders, bool createDirectories, QWidget *parent, std::function<void (QStringList)> func)
{
    QStringList uploads;

    if(panel)
    {
        auto panelParentWindow = [panel sheetParent];
        if(panelParentWindow)
        {
            [panelParentWindow endSheet: panel];
        }
        else
        {
            [panel close];
            panel = NULL;
        }
    }

    if (!panel)
    {
        panel = [NSOpenPanel openPanel];
        if(!parent)
        {
            [panel setTitle:[NSString stringWithUTF8String:title.toUtf8().constData()]];
        }
        [panel setCanChooseFiles: showFiles ? YES : NO];
        [panel setCanChooseDirectories:showFolders ? YES : NO];
        [panel setAllowsMultipleSelection:multiSelection ? YES : NO];
        [panel setCanCreateDirectories: createDirectories ? YES : NO];

        if(!defaultDir.isEmpty())
        {
            NSURL *baseURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:defaultDir.toUtf8().constData()]];
            [panel setDirectoryURL:baseURL];
        }

        if(parent)
        {
            NSView* nsview = (__bridge NSView*)reinterpret_cast<void*>(parent->window()->winId());
            NSWindow *nswindow = [nsview window];
            [panel beginSheetModalForWindow: nswindow
                                             completionHandler:^(NSInteger result) {
                QStringList selection;
                if(result  == NSFileHandlingPanelOKButton)
                {
                    selection = qt_mac_NSArrayToQStringList([panel URLs]);
                }
                func(selection);
                panel = NULL;
            }];
        }
        else
        {
            [panel beginWithCompletionHandler:^(NSInteger result){
                QStringList selection;
                if(result  == NSFileHandlingPanelOKButton)
                {
                    selection = qt_mac_NSArrayToQStringList([panel URLs]);
                }
                func(selection);
                panel = NULL;
            }];
        }
        raiseFileSelectionPanels();
    }
}

void raiseFileSelectionPanels()
{
    if(panel)
    {
        [panel orderFrontRegardless];
    }
}

void closeFileSelectionPanels(QWidget* parent)
{
    if(panel)
    {
        NSWindow* panelParentWindow(nullptr);
        NSView* checkParentview = (__bridge NSView*)reinterpret_cast<void*>(parent->window()->winId());
        NSWindow* checkParentWindow = [checkParentview window];
        panelParentWindow = [panel sheetParent];
        if(checkParentWindow == panelParentWindow)
        {
            [panelParentWindow endSheet: panel];
        }
    }
}

bool startAtLogin(bool opt)
{
    if (opt)
    {
        //Enable start at login
        if (!isStartAtLoginActive())
        {
            addLoginItem();
        }
    }
    else
    {
        removeLoginItem();
    }

    return true;
}

void addPathToPlaces(QString path, QString pathName)
{
    if (path.isEmpty() || !QFileInfo(path).exists())
    {
        return;
    }

    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    if (!appPath)
    {
        return;
    }

    NSString *folderPath = [NSString stringWithUTF8String:path.toUtf8().constData()];
    if (!folderPath)
    {
        return;
    }

    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:folderPath];
    if (url == nil)
    {
        return;
    }

    CFURLRef iconURLRef = (CFURLRef)[NSURL fileURLWithPath:[appPath stringByAppendingString:@"/Contents/Resources/app.icns"]];
    if (iconURLRef == nil)
    {
        return;
    }

    FSRef fref;
    if (!CFURLGetFSRef(iconURLRef, &fref))
    {
        return;
    }

    IconRef iconRef;
    if (RegisterIconRefFromFSRef('SSBL', 'ssic', &fref, &iconRef) != noErr)
    {
        return;
    }

    CFStringRef pnString = CFStringCreateWithCString(NULL, pathName.toUtf8().constData(), kCFStringEncodingUTF8);
    if (pnString == nil)
    {
        return;
    }

    // Create a reference to the shared file list.
    LSSharedFileListRef favoriteItems = LSSharedFileListCreate(NULL, kLSSharedFileListFavoriteItems, NULL);
    if (favoriteItems == nil)
    {
        CFRelease(pnString);
        return;
    }

    //Insert an item to the list.
    LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(favoriteItems, kLSSharedFileListItemLast,
                                                                 pnString, iconRef, url, NULL, NULL);
    if (item)
    {
        CFRelease(item);
    }
    CFRelease(favoriteItems);
    CFRelease(pnString);
}

void removePathFromPlaces(QString path)
{
    if (path.isEmpty() || !QFileInfo(path).exists())
    {
        return;
    }

    // Create a reference to the shared file list of favourite items.
    LSSharedFileListRef favoriteItems = LSSharedFileListCreate(NULL, kLSSharedFileListFavoriteItems, NULL);
    if (favoriteItems == nil)
    {
        return;
    }

    NSString *folderPath = [NSString stringWithUTF8String:path.toUtf8().constData()];
    if (!folderPath)
    {
        CFRelease(favoriteItems);
        return;
    }

    // This will get the path for the application
    CFURLRef url = (CFURLRef)[NSURL fileURLWithPath:folderPath];
    if (!url)
    {
        CFRelease(favoriteItems);
        return;
    }

    //Avoid check special volumes (Airdrop)
    UInt32 resolutionFlags = kLSSharedFileListNoUserInteraction | kLSSharedFileListDoNotMountVolumes;

    // loop through the list of startup items and try to find the MEGAsync app
    CFArrayRef listSnapshot = LSSharedFileListCopySnapshot(favoriteItems, NULL);
    if (listSnapshot == nil)
    {
        CFRelease(favoriteItems);
        return;
    }

    for (int i = 0; i < CFArrayGetCount(listSnapshot); i++)
    {
        LSSharedFileListItemRef item = (LSSharedFileListItemRef)CFArrayGetValueAtIndex(listSnapshot, i);
        if (item != nil)
        {
            CFURLRef itemURL = NULL;
            if (LSSharedFileListItemResolve(item, resolutionFlags, &itemURL, NULL) == noErr && itemURL)
            {
                if (CFEqual(itemURL, url))
                {
                    LSSharedFileListItemRemove(favoriteItems, item);
                }
                CFRelease(itemURL);
            }
        }
    }

    CFRelease(listSnapshot);
    CFRelease(favoriteItems);
}

void setFolderIcon(QString path)
{
    if (path.isEmpty() || !QFileInfo(path).exists())
    {
        return;
    }

    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    if (!appPath)
    {
        return;
    }

    NSString *folderPath = [NSString stringWithUTF8String:path.toUtf8().constData()];
    if (!folderPath)
    {
        return;
    }

    NSImage* iconImage = NULL;
    auto osVersion = QOperatingSystemVersion::current();
    if (osVersion >= QOperatingSystemVersion::MacOSBigSur)
    {
        iconImage = [[NSImage alloc] initWithContentsOfFile:[appPath stringByAppendingString:@"/Contents/Resources/folder_bigsur.icns"]];
    }
    else if (osVersion >= QOperatingSystemVersion::OSXYosemite)
    {
        iconImage = [[NSImage alloc] initWithContentsOfFile:[appPath stringByAppendingString:@"/Contents/Resources/folder_yosemite.icns"]];
    }
    else
    {
        iconImage = [[NSImage alloc] initWithContentsOfFile:[appPath stringByAppendingString:@"/Contents/Resources/folder.icns"]];
    }

    if (iconImage)
    {
        [[NSWorkspace sharedWorkspace] setIcon:iconImage forFile:folderPath options:0];
        [iconImage release];
    }
}

void unSetFolderIcon(QString path)
{
    if (path.isEmpty() || !QFileInfo(path).exists())
    {
        return;
    }

    NSString *folderPath = [NSString stringWithUTF8String:path.toUtf8().constData()];
    if (!folderPath)
    {
        return;
    }

    [[NSWorkspace sharedWorkspace] setIcon:nil forFile:folderPath options:0];
}

QString defaultOpenApp(QString extension)
{
    CFURLRef appURL = NULL;
    CFStringRef ext;
    CFStringRef info;
    char *buffer;

    ext = CFStringCreateWithCString(NULL, extension.toUtf8().constData(), kCFStringEncodingUTF8);
    if (ext == nil)
    {
        return QString();
    }

    LSGetApplicationForInfo(kLSUnknownType, kLSUnknownCreator, ext, kLSRolesAll, NULL, &appURL);
    if (appURL == NULL)
    {
        CFRelease(ext);
        return QString();
    }
    
    info = CFURLCopyFileSystemPath(appURL, kCFURLPOSIXPathStyle);
    if (info == nil)
    {
        CFRelease(ext);
        CFRelease(appURL);
        return QString();
    }

    CFIndex size = CFStringGetMaximumSizeOfFileSystemRepresentation(info);
    buffer = new char[size];
    CFStringGetCString (info, buffer, size, kCFStringEncodingUTF8);
    QString defaultAppPath = QString::fromUtf8(buffer);
    delete [] buffer;
    CFRelease(info);
    CFRelease(appURL);
    CFRelease(ext);
    return defaultAppPath;
}

void enableBlurForWindow(QWidget *window)
{
    NSView *nsview = (NSView *)window->winId();
    NSWindow *nswindow = [nsview window];

    Class vibrantClass = NSClassFromString(@"NSVisualEffectView");
    if (vibrantClass)
    {
        static const NSRect frameRect = {
            { 0.0, 0.0 },
            { static_cast<float>(window->width()), static_cast<float>(window->height()) }
        };

        auto vibrant = [[vibrantClass alloc] initWithFrame:frameRect];
        [vibrant setAutoresizingMask:NSViewWidthSizable|NSViewHeightSizable];
        if ([vibrant respondsToSelector:@selector(setBlendingMode:)])
        {
            [vibrant setBlendingMode:(NSVisualEffectBlendingMode)0];
        }

        //[self addSubview:vibrant positioned:NSWindowBelow relativeTo:nil];
        [nsview addSubview:vibrant positioned:NSWindowBelow relativeTo:nil];
    }
}

bool registerUpdateDaemon()
{
    NSDictionary *plistd = @{
            @"Label": @"mega.mac.megaupdater",
            @"ProgramArguments": @[@"/Applications/MEGAsync.app/Contents/MacOS/MEGAupdater"],
            @"StartInterval": @7200,
            @"RunAtLoad": @true,
            @"StandardErrorPath": @"/dev/null",
            @"StandardOutPath": @"/dev/null",
     };

    const char* home = getenv("HOME");
    if (!home)
    {
        return false;
    }

    NSString *homepath = [NSString stringWithUTF8String:home];
    if (!homepath)
    {
        return false;
    }

    NSString *fullpath = [homepath stringByAppendingString:@"/Library/LaunchAgents/mega.mac.megaupdater.plist"];
    if ([plistd writeToFile:fullpath atomically:YES] == NO)
    {
        return false;
    }

    QString path = QString::fromUtf8([fullpath UTF8String]);
    QFile(path).setPermissions(QFileDevice::ReadOwner | QFileDevice::WriteOwner | QFileDevice::ReadGroup | QFileDevice::ReadOther);

    QStringList scriptArgs;
    scriptArgs << QString::fromUtf8("-c")
               << QString::fromUtf8("launchctl unload %1 && launchctl load %1").arg(path);

    QProcess p;
    p.start(QString::fromAscii("bash"), scriptArgs);
    if (!p.waitForFinished(2000))
    {
        return false;
    }

    return p.exitCode() ? false : true;
}

// Check if it's needed to start the local HTTP server
// for communications with the webclient
bool runHttpServer()
{   
    int nProcesses = proc_listpids(PROC_ALL_PIDS, 0, NULL, 0);
    int pidBufSize = nProcesses * sizeof(pid_t);
    pid_t *pids = new pid_t[nProcesses];
    memset(pids, 0, pidBufSize);
    proc_listpids(PROC_ALL_PIDS, 0, pids, pidBufSize);

    for (int i = 0; i < nProcesses; ++i)
    {
        if (pids[i] == 0)
        {
            continue;
        }

        char processPath[PROC_PIDPATHINFO_MAXSIZE];
        memset(processPath, 0, PROC_PIDPATHINFO_MAXSIZE);
        if (proc_pidpath(pids[i], processPath, PROC_PIDPATHINFO_MAXSIZE) <= 0)
        {
            continue;
        }

        int position = strlen(processPath);
        if (position > 0)
        {
            while (position >= 0 && processPath[position] != '/')
            {
                position--;
            }

            // The MEGA webclient sends request to MEGAsync to improve the
            // user experience. We check if web browsers are running because
            // otherwise it isn't needed to run the local web server for this purpose.
            // Here is the list or web browsers that allow HTTP communications
            // with 127.0.0.1 inside HTTPS webs.
            QString processName = QString::fromUtf8(processPath + position + 1);
            if (!processName.compare(QString::fromUtf8("Google Chrome"), Qt::CaseInsensitive)
                || !processName.compare(QString::fromUtf8("firefox"), Qt::CaseInsensitive))
            {
                delete [] pids;
                return true;
            }
        }
    }

    delete [] pids;
    return false;
}

bool userActive()
{
    CFTimeInterval secondsSinceLastEvent = CGEventSourceSecondsSinceLastEventType(kCGEventSourceStateHIDSystemState, kCGAnyInputEventType);
    if (secondsSinceLastEvent > (Preferences::USER_INACTIVITY_MS / 1000))
    {
         return false;
    }

    return true;
}

double uptime()
{
    struct timeval boottime;
    size_t len = sizeof(boottime);
    int mib[2] = { CTL_KERN, KERN_BOOTTIME };
    if( sysctl(mib, 2, &boottime, &len, NULL, 0) < 0 )
    {
        return -1.0;
    }
    time_t bsec = boottime.tv_sec, csec = time(NULL);

    return difftime(csec, bsec);
}

QString appBundlePath()
{
    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    if (appPath == nil)
    {
        return QString();
    }

    return fromNSString(appPath);
}


bool isStartAtLoginActive()
{
    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath:appPath];

    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
    if (loginItems)
    {
        // Look for app login item
        UInt32 seed = 0U;
        NSArray *items = (NSArray *)CFBridgingRelease(LSSharedFileListCopySnapshot(loginItems, &seed));
        if (items)
        {
            for (id item in items)
            {
                LSSharedFileListItemRef itemRef = (__bridge LSSharedFileListItemRef)item;
                if (itemRef)
                {
                    // Resolve the item with URL
                    if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef *)&url, NULL) == noErr)
                    {
                        NSString *urlPath = [(__bridge NSURL *)url path];
                        if ([urlPath compare:appPath] == NSOrderedSame)
                        {
                            return true;
                        }
                    }
                }
            }
        }
    }

    return false;
}

void addLoginItem()
{
    NSString *appPath = [[NSBundle mainBundle] bundlePath];

    // This will get the path for the application
    // For example, /Applications/MEGAsync.app
    CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath:appPath];

    // Create a reference to the shared file list.
    // Adding it to the current user only (kLSSharedFileListSessionLoginItems)
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
    if (loginItems)
    {
        // Insert an item to the login item list.
        LSSharedFileListItemRef item = LSSharedFileListInsertItemURL(loginItems, kLSSharedFileListItemLast, NULL, NULL, url, NULL, NULL);
        if (item)
        {
            CFRelease(item);
        }

        CFRelease(loginItems);
    }    
}

void removeLoginItem()
{
    NSString *appPath = [[NSBundle mainBundle] bundlePath];
    CFURLRef url = (__bridge CFURLRef)[NSURL fileURLWithPath:appPath];

    // Create a reference to the shared file list.
    LSSharedFileListRef loginItems = LSSharedFileListCreate(NULL, kLSSharedFileListSessionLoginItems, NULL);
    if (loginItems)
    {
        UInt32 seedValue = 0U;
        NSArray  *items = (NSArray *)CFBridgingRelease(LSSharedFileListCopySnapshot(loginItems, &seedValue));
        for (id item in items)
        {
            LSSharedFileListItemRef itemRef = (__bridge LSSharedFileListItemRef)item;
            if (itemRef)
            {
                if (LSSharedFileListItemResolve(itemRef, 0, (CFURLRef *)&url, NULL) == noErr)
                {
                    NSString *urlPath = [(__bridge NSURL *)url path];
                    if ([urlPath compare:appPath] == NSOrderedSame)
                    {
                        // Remove an item to the login item list.
                        LSSharedFileListItemRemove(loginItems, itemRef);
                    }
                }
            }
        }
    }
}
