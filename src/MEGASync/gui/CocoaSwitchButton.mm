#include "CocoaSwitchButton.h"
#include <Cocoa/Cocoa.h>

@interface NSSwitchTarget: NSObject
{
    CocoaSwitchButton *m_pTarget;
}
-(id)initWithObject:(CocoaSwitchButton*)object;
-(IBAction)clicked:(id)sender;
@end
@implementation NSSwitchTarget
-(id)initWithObject:(CocoaSwitchButton*)object
{
    self = [super init];
    m_pTarget = object;
    return self;
}
-(IBAction)clicked:(id)sender
{
    m_pTarget->onClicked();
}
@end

CocoaSwitchButton::CocoaSwitchButton(QWidget *pParent /* = 0 */)
  :QMacCocoaViewContainer(0, pParent),
    mChecked(false)
{
    m_pButton = [[NSSwitch alloc] init];
    [m_pButton sizeToFit];
    NSRect frame = [m_pButton frame];

    /* We need a target for the click selector */
    NSSwitchTarget *bt = [[NSSwitchTarget alloc] initWithObject: this];
    [m_pButton setTarget: bt];
    [m_pButton setAction: @selector(clicked:)];    
    [m_pButton setState:NSControlStateValueOff];
    setCocoaView((NSSwitch *)m_pButton);

    /* Make sure all is properly resized */
    resize(frame.size.width, frame.size.height);
    setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Fixed);
}

QSize CocoaSwitchButton::sizeHint() const
{
    [m_pButton sizeToFit];
    NSRect frame = [m_pButton frame];
    return QSize(frame.size.width, frame.size.height);
}

void CocoaSwitchButton::onClicked()
{
    NSControlStateValue value = [m_pButton state];
    mChecked = value == NSControlStateValueOn ? true : false;
    emit toggled(mChecked);
}

bool CocoaSwitchButton::isChecked()
{
    return mChecked;
}

void CocoaSwitchButton::setChecked(bool state)
{
    if(mChecked != state)
    {
        mChecked = state;
        [m_pButton setState:mChecked ? NSControlStateValueOn : NSControlStateValueOff];
        emit toggled(mChecked);
    }
}

