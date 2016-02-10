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
    [m_pButton setTitle: @""];
    [m_pButton setBezelStyle: NSHelpButtonBezelStyle];
    [m_pButton setBordered: YES];
    [m_pButton setAlignment: NSCenterTextAlignment];
    [m_pButton sizeToFit];
    NSRect frame = [m_pButton frame];
    frame.size.width += 12; /* Margin */
    [m_pButton setFrame:frame];
    /* We need a target for the click selector */
    NSButtonTarget *bt = [[NSButtonTarget alloc] initWithObject: this];
    [m_pButton setTarget: bt];
    [m_pButton setAction: @selector(clicked:)];

    setCocoaView((NSButton *)m_pButton);
    /* Make sure all is properly resized */
    resize(frame.size.width, frame.size.height);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize CocoaHelpButton::sizeHint() const
{
    NSRect frame = [m_pButton frame];
    return QSize(frame.size.width, frame.size.height);
}

void CocoaHelpButton::onClicked()
{
    emit clicked();
}

