import common 1.0

Button {
    id: root

    colors {
        background: colorStyle.buttonPrimary
        border: colors.background
        pressed: colorStyle.buttonPrimaryPressed
        borderPressed: colors.pressed
        hover: colorStyle.buttonPrimaryHover
        borderHover: colors.hover
        text: colorStyle.textOnColor
        textPressed: colorStyle.textOnColor
        textHover: colorStyle.textOnColor
        textDisabled: colorStyle.textDisabled
    }
}

