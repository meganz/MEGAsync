import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color pressed: ColorTheme.buttonPrimaryPressed
    property color hover: ColorTheme.buttonPrimaryHover
    property color disabled: ColorTheme.buttonDisabled
    property color enabled: ColorTheme.buttonPrimary
    property color focus: ColorTheme.focusColor

}

