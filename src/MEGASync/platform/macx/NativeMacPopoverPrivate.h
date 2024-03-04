#ifndef NATIVEMACPOPOVERPRIVATE_H
#define NATIVEMACPOPOVERPRIVATE_H

#import <AppKit/AppKit.h>
#include <QWidget>
#include "NativeMacPopover.h"

@interface NativeMacPopoverPrivate : NSObject {
    QWidget *m_contentWidget;
    NSPopover *m_popover;
    NSView *m_nativeView;
    NSViewController *m_viewController;
}

- (void) show:(QWidget *) contentWidget
         withView:(NSView *) view
       sourceRect:(NSRect) position
             size:(NSSize) size
            color:(NativeMacPopover::PopOverColor) color
    preferredEdge:(NativeMacPopover::PopOverEdge) preferredEdge;

@end


#endif // NATIVEMACPOPOVERPRIVATE_H
