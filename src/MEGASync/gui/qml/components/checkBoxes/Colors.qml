import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color backgroundUnchecked: "transparent"
    property color background: colorStyle.buttonPrimary
    property color backgroundHover: colorStyle.buttonPrimaryHover
    property color backgroundPressed: colorStyle.buttonPrimaryPressed
    property color backgroundDisabled: colorStyle.iconButtonDisabled
    property color border: colorStyle.buttonPrimary
    property color borderHover: colorStyle.buttonPrimaryHover
    property color borderPressed: colorStyle.buttonPrimaryPressed
    property color borderDisabled: colorStyle.iconButtonDisabled
    property color icon: colorStyle.iconInverseAccent
    property color text: colorStyle.textPrimary
    property color textDisabled: colorStyle.textDisabled
    property color textHover: colorStyle.buttonPrimaryHover
    property color textPressed: colorStyle.buttonPrimaryPressed
}
