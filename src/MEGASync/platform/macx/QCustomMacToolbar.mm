#include "QCustomMacToolbar.h"
#include <Cocoa/Cocoa.h>
#include <QWindow>

void QCustomMacToolbar::setSelectableItems(bool selectable)
{
    for (auto toolBarItem : this->items())
    {
        toolBarItem->setSelectable(selectable);
    }
}

void QCustomMacToolbar::setAllowsUserCustomization(bool custom)
{
    NSToolbar *tb = nativeToolbar();
    [tb setAllowsUserCustomization:custom];
}

void QCustomMacToolbar::setSelectedItem(QMacToolBarItem *defaultItem)
{
    if (defaultItem)
    {
        NSToolbar *tb = nativeToolbar();
        [tb setSelectedItemIdentifier:[defaultItem->nativeToolBarItem() itemIdentifier]];
    }
}

void QCustomMacToolbar::setEnableToolbarItems(bool isEnabled)
{
    for (auto toolBarItem : this->items())
    {
        [toolBarItem->nativeToolBarItem() setEnabled:isEnabled];
    }
}

void QCustomMacToolbar::attachToWindowWithStyle(QWindow *window, QCustomMacToolbar::BigSurToolbarStyle style)
{
    NSView* nsview = (__bridge NSView*)reinterpret_cast<void*>(window->winId());
    NSWindow* nsWindow = [nsview window];
    if (@available(macOS 10.16, *)) {// macOS 11 is not properly detected when building for other targets
           nsWindow.toolbarStyle = (NSWindowToolbarStyle)(style);
        }

    QMacToolBar::attachToWindow(window);
}

void QCustomMacToolbar::customizeIconToolBarItem(QMacToolBarItem *toolbarItem, QString iconName)
{
    if (!toolbarItem)
    {
        return;
    }

    if (@available(macOS 10.16, *))
    {// macOS 11 is not properly detected when building for other targets

        //Load icon from images.xcassets, needed to load SF symbol (svg image)
        [toolbarItem->nativeToolBarItem() setImage:[NSImage imageNamed:iconName.toNSString()]];
    }
    else
    {
        QIcon icon(QString::fromUtf8("://images/%1-macx.png").arg((iconName)));
        toolbarItem->setIcon(icon);
    }
}
