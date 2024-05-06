import common 1.0

Button {
    id: root

    colors {
        background: ColorTheme.buttonPrimary
        border: colors.background        
        pressed: ColorTheme.buttonPrimaryPressed
        borderPressed: colors.pressed
        hover: ColorTheme.buttonPrimaryHover
        borderHover: colors.hover
        text: ColorTheme.textInverse
        textPressed: ColorTheme.textInverse
        textHover: ColorTheme.textInverse
        textDisabled: ColorTheme.textDisabled
    }
}

