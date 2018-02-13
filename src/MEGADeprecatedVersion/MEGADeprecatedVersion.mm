#include <stdio.h>
#include <Cocoa/Cocoa.h>

@interface AppDelegate : NSObject <NSApplicationDelegate> {
        NSAlert *alert;
    }
    - (AppDelegate *) initWithArgc:(int)argc argv:(const char **)argv;
    - (void) applicationWillFinishLaunching: (NSNotification *)notification;
@end

@implementation AppDelegate
    - (AppDelegate *) initWithArgc:(int)argc argv:(const char **)argv
    {
        return self;
    }

    - (void) applicationWillFinishLaunching: (NSNotification *)notification
    {
        NSAlert *alert = [[NSAlert alloc] init];
        [alert addButtonWithTitle:@"OK"];
        [alert setMessageText:@"Unsupported macOS Version"];
        [alert setInformativeText:@"We have detected that you are using and old macOS version that is no longer supported. Please update your system to macOS 10.9+ or download the latest compatible installer of MEGAsync"];

        NSTextView *view = [[NSTextView alloc] initWithFrame:NSMakeRect(0,0,400,20)];
        [view setTextContainerInset:NSMakeSize(0.0f, 0.0f)];

        NSFont *font = [NSFont systemFontOfSize:[NSFont systemFontSize]];
        NSDictionary *textAttributes = [NSDictionary dictionaryWithObject:font forKey:NSFontAttributeName];

        NSMutableAttributedString* str = [[NSMutableAttributedString alloc] initWithString:@"Click here to download last compatible installer of MEGAsync"];
        [str addAttribute: NSLinkAttributeName value: @"https://mega.nz/MEGAsyncSetupLegacy.dmg" range: NSMakeRange(0, str.length)];

        [view insertText:str];
        [view setEditable:NO];
        [view setDrawsBackground:NO];
        [alert setAccessoryView:view];

        [alert setAlertStyle:NSWarningAlertStyle];

        if ([alert runModal] == NSAlertFirstButtonReturn)
        {
            [alert release];
            [view release];
            [str release];
            exit(0);
        }
    }
 @end

int main(int argc, char *argv[])
{
    NSApplication *app =[NSApplication sharedApplication];
    app.delegate = [[AppDelegate alloc] initWithArgc:argc argv:(const char **)argv];
    return NSApplicationMain (argc, (const char **)argv);
}
