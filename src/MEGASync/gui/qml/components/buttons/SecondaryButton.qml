// Local
import Components 1.0 as Custom
import Common 1.0

Custom.Button {
    icons.color: Styles.iconSecondary
    icons.disabledColor: Styles.textDisabled

    colors {
        background: Styles.buttonSecondary
        border: colors.background
        text: Styles.textSecondary
        textPressed: Styles.buttonSecondaryPressed
        textHover: Styles.textSecondary
        pressed: Styles.buttonSecondaryPressed
        borderPressed: colors.pressed
        hover: Styles.buttonSecondaryHover
        borderHover: colors.hover
    }
}

