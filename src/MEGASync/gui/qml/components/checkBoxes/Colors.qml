import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property color backgroundUnchecked: "transparent"
    property color background: Styles.buttonPrimary
    property color backgroundHover: Styles.buttonPrimaryHover
    property color backgroundPressed: Styles.buttonPrimaryPressed
    property color backgroundDisabled: Styles.iconButtonDisabled
    property color border: Styles.buttonPrimary
    property color borderHover: Styles.buttonPrimaryHover
    property color borderPressed: Styles.buttonPrimaryPressed
    property color borderDisabled: Styles.iconButtonDisabled
    property color icon: Styles.iconInverseAccent
    property color text: Styles.textPrimary
    property color textDisabled: Styles.textDisabled
    property color textHover: Styles.buttonPrimaryHover
    property color textPressed: Styles.buttonPrimaryPressed
}
