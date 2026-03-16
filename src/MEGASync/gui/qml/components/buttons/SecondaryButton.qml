import common 1.0

Button {
    id: root

    icons {
        colorEnabled: ColorTheme.iconPrimary
        colorHovered: ColorTheme.iconPrimary
        colorPressed: ColorTheme.iconPrimary
    }

    colors {
        background: ColorTheme.buttonSecondary
        border: colors.background
        text: ColorTheme.textPrimary
        textPressed: ColorTheme.textPrimary
        textHover: ColorTheme.textPrimary
        textDisabled: ColorTheme.textDisabled
        pressed: ColorTheme.buttonSecondaryPressed
        borderPressed: colors.pressed
        hover: ColorTheme.buttonSecondaryHover
        borderHover: colors.hover
    }
}

