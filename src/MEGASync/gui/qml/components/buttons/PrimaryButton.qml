import common 1.0

Button {
    id: root

    colors {
        background: Styles.buttonPrimary
        border: colors.background
        pressed: Styles.buttonPrimaryPressed
        borderPressed: colors.pressed
        hover: Styles.buttonPrimaryHover
        borderHover: colors.hover
        text: Styles.textOnColor
        textPressed: Styles.textOnColor
        textHover: Styles.textOnColor
        textDisabled: Styles.textDisabled
    }
}

