// System
import QtQuick.Layouts 1.12

// Local
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

MegaButtons.Button {
    sizes.borderWidth: 0
    height: 32 + 2 * sizes.focusBorderWidth
    width: height
    Layout.preferredHeight: height
    Layout.preferredWidth: width

    icons {
        colorEnabled: Styles.iconButton
        colorDisabled: Styles.iconButtonDisabled
        colorHovered: Styles.iconButtonHover
        colorPressed: Styles.iconButtonPressed
    }

    colors {
        background: "transparent"
        hover: "transparent"
        pressed: Styles.iconButtonPressedBackground
        border: "transparent"
        borderDisabled: "transparent"
        borderHover: "transparent"
        borderSelected: "transparent"
        borderPressed: "transparent"
    }

    sizes {
        iconWidth: 24
        verticalPadding: 4
        horizontalPadding: 4
    }
}
