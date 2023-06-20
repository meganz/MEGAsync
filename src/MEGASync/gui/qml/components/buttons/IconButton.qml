// System
import QtQuick.Layouts 1.12

// Local
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

MegaButtons.Button {
    borderWidth: 0
    leftPadding: 5 + focusBorderWidth
    rightPadding: 5 + focusBorderWidth
    topPadding: 5 + focusBorderWidth
    bottomPadding: 5 + focusBorderWidth
    height: 32 + 2 * focusBorderWidth
    width: height
    Layout.preferredHeight: height
    Layout.preferredWidth: width

    icons {
        colorEnabled: Styles.iconButton
        colorDisabled: Styles.iconButtonDisabled
        colorHovered: Styles.iconButtonHover
        colorPressed: Styles.iconButtonPressed
        size: Qt.size(24, 24)
    }

    colors {
        background: "transparent"
        hover: "transparent"
        pressed: Styles.iconButtonPressedBackground
    }

}
