import common 1.0

Button {
    id: root

    colors {
        background: Style.getCategoryValue("primaryButton", "buttonPrimary")

        border: colors.background
        //pressed: colorStyle.buttonPrimaryPressed
        pressed: Style.getColorValue("buttonPrimaryPressed");
        borderPressed: colors.pressed
        hover: colorStyle.buttonPrimaryHover
        borderHover: colors.hover
        text: colorStyle.textOnColor
        textPressed: colorStyle.textOnColor
        textHover: colorStyle.textOnColor
        textDisabled: colorStyle.textDisabled
    }
}

