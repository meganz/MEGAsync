#ifndef NODESELECTORVIEWSTYLE_H
#define NODESELECTORVIEWSTYLE_H

#include "MegaProxyStyle.h"

// Forces a fixed gap between an item's icon and its text in the node selector tree views.
// Qt's default item-view layout derives that gap from PM_FocusFrameHMargin (style/platform
// dependent), so it is pinned here at the single point both the standard delegate paint and the
// search delegate (which queries SE_ItemViewItemText) rely on.
class NodeSelectorViewStyle: public MegaProxyStyle
{
public:
    QRect subElementRect(SubElement element,
                         const QStyleOption* option,
                         const QWidget* widget) const override;

    void drawControl(ControlElement element,
                     const QStyleOption* option,
                     QPainter* painter,
                     const QWidget* widget) const override;

private:
    static constexpr int ICON_TEXT_SPACING = 4;
};

#endif // NODESELECTORVIEWSTYLE_H
