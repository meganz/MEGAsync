import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color backgroundUnchecked: "transparent"
    property color background: ColorTheme.buttonPrimary
    property color backgroundHover: ColorTheme.buttonPrimaryHover
    property color backgroundPressed: ColorTheme.buttonPrimaryPressed
    property color backgroundDisabled: ColorTheme.buttonDisabled
    property color border: ColorTheme.buttonPrimary
    property color borderHover: ColorTheme.buttonPrimaryHover
    property color borderPressed: ColorTheme.buttonPrimaryPressed
    property color borderDisabled: ColorTheme.buttonDisabled
    property color icon: ColorTheme.iconInverseAccent
    property color text: ColorTheme.textPrimary
    property color textDisabled: ColorTheme.textDisabled
    property color textHover: ColorTheme.buttonPrimaryHover
    property color textPressed: ColorTheme.buttonPrimaryPressed
}
