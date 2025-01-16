import common 1.0

Button {
    id: root

    icons {
        colorEnabled: ColorTheme.iconSecondary
        colorHovered: ColorTheme.iconSecondary
        colorPressed: ColorTheme.iconSecondary
    }

    colors {
        background: ColorTheme.buttonSecondary
        border: colors.background
        text: ColorTheme.textSecondary
        textPressed: ColorTheme.textSecondary
        textHover: ColorTheme.textSecondary
        textDisabled: ColorTheme.textDisabled
        pressed: ColorTheme.buttonSecondaryPressed
        borderPressed: colors.pressed
        hover: ColorTheme.buttonSecondaryHover
        borderHover: colors.hover
    }
}

