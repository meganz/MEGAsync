#include "NativeMacPopoverPrivate.h"

@implementation NativeMacPopoverPrivate

- (void) show:(QWidget *) contentWidget
         withView:(NSView *) view
       sourceRect:(NSRect) position
             size:(NSSize) size
            color:(NativeMacPopover::PopOverColor) selectedColor
    preferredEdge:(NativeMacPopover::PopOverEdge) selectedEdge {

    m_contentWidget = contentWidget;

    m_viewController = [[NSViewController alloc] init];
    m_popover = [[NSPopover alloc] init];
    m_nativeView = [[NSView alloc] init];

    if (selectedColor == NativeMacPopover::PopOverColor::WHITE)
    {
      [m_popover setBackgroundColor:[NSColor whiteColor]];
    }
    else
    {
      [m_popover setBackgroundColor:[NSColor clearColor]];
    }

    [m_popover setContentSize: size];
    [m_popover setBehavior:NSPopoverBehaviorTransient];
    [m_popover setAnimates:YES];
    [m_popover setContentViewController: m_viewController];

    auto contentWidgetView = reinterpret_cast<NSView *>(m_contentWidget->winId());

    [m_nativeView addSubview: contentWidgetView];
    m_contentWidget->show();
    [m_viewController setView: contentWidgetView];

    [m_popover showRelativeToRect: position
                           ofView: view
                    preferredEdge: (NSRectEdge)selectedEdge];

    [[NSNotificationCenter defaultCenter] addObserver:self
                                             selector:@selector(popoverClosedNotification:)
                                                 name:@"NSPopoverDidCloseNotification"
                                               object:m_popover];
}

- (void) popoverClosedNotification:(NSNotification *) notification {
    delete m_contentWidget;
}

@end
