// System
import QtQuick 2.12

//Local
import Common 1.0

QtObject {
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
