#include <stdio.h>
#include <Cocoa/Cocoa.h>

int main(int argc, char *argv[])
{
    NSAlert *alert = [[NSAlert alloc] init];
    [alert addButtonWithTitle:@"OK"];
    [alert setMessageText:@"MEGAsync unsupported macOS Version"];
    [alert setInformativeText:@"We have detected that you are using and old macOS version that is no longer supported. Please update your system to macOS 10.9 or superior"];
    [alert setAlertStyle:NSWarningAlertStyle];
    [alert runModal];
    [alert release];
    return 0;
}
