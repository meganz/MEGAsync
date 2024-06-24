import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color background
    property color disabled: colorStyle.buttonDisabled
    property color text
    property color textDisabled: colorStyle.textDisabled
    property color focus: colorStyle.focus
    property color textHover
    property color textPressed
    property color hover
    property color borderHover
    property color pressed
    property color border
    property color borderDisabled: colorStyle.buttonDisabled
    property color borderSelected
    property color borderPressed
}

