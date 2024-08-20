import QtQuick 2.15

import common 1.0

QtObject {
    id: root
    // menu
    property color menuBackground: colorStyle.pageBackground
    property color menuBorder: colorStyle.borderDisabled
    // property color disabled: colorStyle.buttonDisabled
    // property color text
    // property color textDisabled: colorStyle.textDisabled
    // item
    property color focus: colorStyle.focus
    property color itemBackground: "transparent"
    property color itemBackgroundHover: colorStyle.surface1
    property color itemBackgroundPressed: colorStyle.surface2
    property color text: colorStyle.textPrimary
    property color icon: colorStyle.iconPrimary
    // property color borderHover
    // property color pressed
    property color border
    // property color borderDisabled: colorStyle.buttonDisabled
    // property color borderSelected
    // property color borderPressed
}

