#include "NativeMacPopover.h"
#include "NativeMacPopoverPrivate.h"

NativeMacPopover::NativeMacPopover()
{

}

void NativeMacPopover::show(QWidget *widget, QWidget *contentWidget, QPointF showingPoint, PopOverColor color, PopOverEdge edge)
{
    mPopOver = [[NativeMacPopoverPrivate alloc] init];

    NSRect position = CGRectMake(showingPoint.x(), showingPoint.y(), 1, 1);

    [mPopOver show:  contentWidget
           withView: reinterpret_cast<NSView *>(widget->winId())
         sourceRect: position
               size: contentWidget->size().toCGSize()
              color: color
      preferredEdge: edge];
}
