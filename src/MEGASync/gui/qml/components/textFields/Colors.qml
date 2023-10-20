// System
import QtQuick 2.15

// Local
import Common 1.0

QtObject {
    id: colorsRoot

    property color selection: Styles.focus
    property color placeholder: Styles.textPlaceholder
    property color text: Styles.textPrimary
    property color textDisabled: Styles.textDisabled
    property color focus: Styles.focus
    property color border: Styles.borderStrong
    property color borderDisabled: Styles.borderDisabled
    property color borderError: Styles.textError
    property color borderFocus: Styles.borderStrongSelected
    property color background: Styles.pageBackground
    property color title: Styles.textPrimary
    property color titleDisabled: Styles.textDisabled
    property color icon: Styles.iconSecondary
    property color iconDisabled: Styles.iconDisabled

}
