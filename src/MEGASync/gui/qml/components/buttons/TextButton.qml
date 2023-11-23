import QtQuick 2.15

import common 1.0

Button {
    id: root

    sizes.isLinkOrTextButton: true

    icons {
        colorEnabled: Styles.buttonOutline
        colorHovered: Styles.buttonOutlineHover
        colorPressed: Styles.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: Styles.buttonOutline
        textHover: Styles.buttonOutlineHover
        textPressed: Styles.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: "transparent"
    }
}
