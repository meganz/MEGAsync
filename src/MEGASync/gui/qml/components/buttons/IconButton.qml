import QtQuick.Layouts 1.15

import common 1.0

Button {
    id: root

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
        disabled: "transparent"
    }

    sizes {
        iconWidth: 24
        verticalPadding: 4
        horizontalPadding: 4
    }
}
