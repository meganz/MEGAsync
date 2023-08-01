#ifndef NATIVEMACPOPOVER_H
#define NATIVEMACPOPOVER_H

#include <QObject>

Q_FORWARD_DECLARE_OBJC_CLASS(NativeMacPopoverPrivate);

class NativeMacPopover
{
public:
    enum  class PopOverEdge {
        EdgeMinX = 0,
        EdgeMinY,
        EdgeMaxX,
        EdgeMaxY
    };

    enum class PopOverColor {
        TRANSPARENT = 0,
        WHITE
    };

    NativeMacPopover();

    void show(QWidget *widget, QWidget *contentWidget, QPointF showingPoint, PopOverColor color, PopOverEdge edge);

private:
    NativeMacPopoverPrivate *mPopOver;
};

#endif // NATIVEMACPOPOVER_H
