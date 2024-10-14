import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    // menu
    property color menuBackground: ColorTheme.pageBackground
    property color menuBorder: ColorTheme.borderDisabled
    property color shadowColor: "#10000000"

    // item
    property color focus: ColorTheme.focusColor
    property color itemBackground: "transparent"
    property color itemBackgroundHover: ColorTheme.surface1
    property color itemBackgroundPressed: ColorTheme.surface2
    property color text: ColorTheme.textPrimary
    property color icon: ColorTheme.iconPrimary
}

