import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color background
    property color disabled: ColorTheme.buttonDisabled
    property color text
    property color textDisabled: ColorTheme.textDisabled
    property color focus: ColorTheme.focusColor
    property color textHover
    property color textPressed
    property color hover
    property color borderHover
    property color pressed
    property color border
    property color borderDisabled: ColorTheme.buttonDisabled
    property color borderSelected
    property color borderPressed
}

