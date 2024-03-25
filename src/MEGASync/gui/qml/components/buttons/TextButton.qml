import QtQuick 2.15

import common 1.0

Button {
    id: root

    sizes.borderLess: true

    icons {
        colorEnabled: ColorTheme.buttonOutline
        colorHovered: ColorTheme.buttonOutlineHover
        colorPressed: ColorTheme.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: "transparent"
        border: "transparent"
        borderHover: "transparent"
        borderDisabled: "transparent"
        text: ColorTheme.buttonOutline
        textHover: ColorTheme.buttonOutlineHover
        textPressed: ColorTheme.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: "transparent"
    }
}
