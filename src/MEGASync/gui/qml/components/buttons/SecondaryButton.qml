import common 1.0

Button {
    id: root

    icons {
        colorEnabled: colorStyle.iconSecondary
        colorHovered: colorStyle.iconSecondary
        colorPressed: colorStyle.iconSecondary
    }

    colors {
        background: colorStyle.buttonSecondary
        border: colors.background
        text: colorStyle.textSecondary
        textPressed: colorStyle.buttonSecondaryPressed
        textHover: colorStyle.textSecondary
        textDisabled: colorStyle.textDisabled
        pressed: colorStyle.buttonSecondaryPressed
        borderPressed: colors.pressed
        hover: colorStyle.buttonSecondaryHover
        borderHover: colors.hover
    }
}

