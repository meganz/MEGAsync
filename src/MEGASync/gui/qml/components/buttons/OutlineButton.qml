import common 1.0

Button {
    id: root

    icons {
        colorEnabled: colorStyle.buttonOutline
        colorHovered: colorStyle.buttonOutlineHover
        colorPressed: colorStyle.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: colorStyle.buttonOutlineBackgroundHover
        border: colorStyle.buttonOutline
        borderHover: colorStyle.buttonOutlineHover
        borderDisabled: colorStyle.buttonDisabled
        text: colorStyle.buttonOutline
        textHover: colorStyle.buttonOutlineHover
        textPressed: colorStyle.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: colorStyle.buttonOutlinePressed
    }
}

