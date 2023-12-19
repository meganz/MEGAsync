import common 1.0

Button {
    id: root

    icons {
        colorEnabled: Styles.iconSecondary
        colorHovered: Styles.iconSecondary
        colorPressed: Styles.iconSecondary
    }

    colors {
        background: Styles.buttonSecondary
        border: colors.background
        text: Styles.textSecondary
        textPressed: Styles.buttonSecondaryPressed
        textHover: Styles.textSecondary
        textDisabled: Styles.textDisabled
        pressed: Styles.buttonSecondaryPressed
        borderPressed: colors.pressed
        hover: Styles.buttonSecondaryHover
        borderHover: colors.hover
    }
}

