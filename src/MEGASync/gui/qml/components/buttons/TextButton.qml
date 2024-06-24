import QtQuick 2.15

import common 1.0

Button {
    id: root

    sizes.borderLess: true

    icons {
        colorEnabled: colorStyle.buttonOutline
        colorHovered: colorStyle.buttonOutlineHover
        colorPressed: colorStyle.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: colorStyle.buttonOutline
        textHover: colorStyle.buttonOutlineHover
        textPressed: colorStyle.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: "transparent"
    }
}
