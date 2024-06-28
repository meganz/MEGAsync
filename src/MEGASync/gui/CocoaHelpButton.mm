#include "CocoaHelpButton.h"
#include <Cocoa/Cocoa.h>

@interface NSButtonTarget: NSObject
{
    CocoaHelpButton *m_pTarget;
}
-(id)initWithObject:(CocoaHelpButton*)object;
-(IBAction)clicked:(id)sender;
@end
@implementation NSButtonTarget
-(id)initWithObject:(CocoaHelpButton*)object
{
    self = [super init];
    m_pTarget = object;
    return self;
}
-(IBAction)clicked:(id)sender;
{
    m_pTarget->onClicked();
}
@end

CocoaHelpButton::CocoaHelpButton(QWidget *pParent /* = 0 */)
  :QMacCocoaViewContainer(0, pParent)
{
    m_pButton = [[NSButton alloc] init];
    [(__bridge NSButton *) m_pButton setTitle: @""];
    [(__bridge NSButton *) m_pButton setBezelStyle: NSHelpButtonBezelStyle];
    [(__bridge NSButton *) m_pButton setBordered: YES];
    [(__bridge NSButton *) m_pButton setAlignment: NSCenterTextAlignment];
    [(__bridge NSButton *) m_pButton sizeToFit];
    NSRect frame = [(__bridge NSButton *) m_pButton frame];
    frame.size.width += 12; /* Margin */
    [(__bridge NSButton *) m_pButton setFrame:frame];
    /* We need a target for the click selector */
    NSButtonTarget *bt = [[NSButtonTarget alloc] initWithObject: this];
    [(__bridge NSButton *) m_pButton setTarget: bt];
    [(__bridge NSButton *) m_pButton setAction: @selector(clicked:)];

    setCocoaView((__bridge NSButton *) m_pButton);
    /* Make sure all is properly resized */
    resize(frame.size.width, frame.size.height);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize CocoaHelpButton::sizeHint() const
{
    NSRect frame = [(__bridge NSButton *) m_pButton frame];
    return QSize(frame.size.width, frame.size.height);
}

void CocoaHelpButton::onClicked()
{
    emit clicked();
}

