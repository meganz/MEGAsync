import common 1.0

Button {
    id: root

    colors {
        background: ColorTheme.buttonPrimary
        //background: CategoryTheme.primaryButton.background
        border: colors.background        
        pressed: ColorTheme.buttonPrimaryPressed
        borderPressed: colors.pressed
        hover: ColorTheme.buttonPrimaryHover
        borderHover: colors.hover
        text: ColorTheme.textOnColor
        textPressed: ColorTheme.textOnColor
        textHover: ColorTheme.textOnColor
        textDisabled: ColorTheme.textDisabled
    }
}

