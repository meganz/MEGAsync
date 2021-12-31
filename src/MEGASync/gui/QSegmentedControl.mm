#include "QSegmentedControl.h"
#include <Cocoa/Cocoa.h>
#include <AppKit/AppKit.h>
#include <QHBoxLayout>

@interface QSegmentedControlTarget : NSObject
{
@public
    QPointer<QSegmentedControl> pimpl;
}
-(void)clicked;
@end

@implementation QSegmentedControlTarget
-(void)clicked: (id)sender {
    Q_ASSERT(pimpl);
    if (pimpl)
        pimpl->clicked([sender selectedSegment]);
}
@end

QSegmentedControl::QSegmentedControl(QWidget *parent)
    : QWidget(parent)
{
    segmentType = -1;
    cocoaContainer = NULL;
    parentLayout = NULL;
}

void QSegmentedControl::configureTableSegment()
{
    segmentType = TYPE_TABLE;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSSegmentedControl *segControl = [[NSSegmentedControl alloc] init];

    [segControl setSegmentCount:3];
    [segControl setSegmentStyle:NSSegmentStyleSmallSquare];
    [segControl setTrackingMode:NSSegmentSwitchTrackingMomentary];
    [segControl setImage:[NSImage imageNamed:NSImageNameAddTemplate] forSegment:0];
    [segControl setImage:[NSImage imageNamed:NSImageNameRemoveTemplate] forSegment:1];
    [segControl setEnabled:NO forSegment:2];
    [segControl setWidth:31 forSegment:0];
    [segControl setWidth:31 forSegment:1];

    //Remaining space witouth + and - buttons and borders
    CGFloat remainingSpace = [segControl widthForSegment:0] * 2 + 4;
    QSegmentedControlTarget *proxy = [[QSegmentedControlTarget alloc] init];
    proxy->pimpl = this;

    [segControl setTarget:proxy];
    [segControl setAction:@selector(clicked:)];

    setupView((NSView *)segControl, this);

    [segControl release];
    [pool release];
}

void QSegmentedControl::configureTabSegment(QStringList options)
{
    segmentType = TYPE_TAB;

    NSAutoreleasePool *pool = [[NSAutoreleasePool alloc] init];
    NSSegmentedControl *segControl = [[NSSegmentedControl alloc] init];

    NSRect frame = [segControl frame];
    [segControl setFrame:frame];

    [segControl setSegmentCount:options.size()];

    for (int i = 0; i < options.size(); i++)
    {
        [segControl setLabel:options[i].toNSString() forSegment:i];
        [segControl setWidth:0 forSegment:i];
    }

    QSegmentedControlTarget *proxy = [[QSegmentedControlTarget alloc] init];
    proxy->pimpl = this;

    [segControl setTarget:proxy];
    [segControl setAction:@selector(clicked:)];

    setupView((NSView *)segControl, this);

    [segControl release];
    [pool release];
}

void QSegmentedControl::clicked(int segment)
{
    switch (segmentType)
    {
        case TYPE_TABLE:
            if (segment == ADD_BUTTON) // + button clicked
            {
                emit addButtonClicked();
            }
            else if (segment == REMOVE_BUTTON) // - button clicked
            {
                emit removeButtonClicked();
            }
            break;
        case TYPE_TAB:
            if (segment == UNDEFINED)
            {
                return;
            }
            emit segmentClicked(segment);
            break;
        default:
            break;
    }
}

QSegmentedControl::~QSegmentedControl()
{
    delete cocoaContainer;
}

void QSegmentedControl::setupView(NSView *cocoaView, QWidget *parent)
{
    if (cocoaContainer)
    {
        delete cocoaContainer;
        cocoaContainer = NULL;
        clearLayout(this);
    }

    parent->setAttribute(Qt::WA_NativeWindow);
    QHBoxLayout *layout = new QHBoxLayout(parent);
    layout->setMargin(0);

    cocoaContainer = new QMacCocoaViewContainer(cocoaView, parent);
    layout->addWidget(cocoaContainer);
}

void QSegmentedControl::clearLayout(QWidget *widget)
{
    QLayout* layout = widget->layout();
    if (layout != 0)
    {
        QLayoutItem *item;
        while ((item = layout->takeAt(0)) != 0)
        {
            layout->removeItem(item);
        }
        delete layout;
    }
}

