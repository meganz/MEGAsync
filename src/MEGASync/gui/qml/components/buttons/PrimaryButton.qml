// Local
import Components 1.0 as Custom
import Common 1.0

Custom.Button {
    icons.color: Styles.textInverseAccent
    icons.disabledColor: Styles.textDisabled

    colors {
        background: Styles.buttonPrimary
        border: colors.background
        pressed: Styles.buttonPrimaryPressed
        borderPressed: colors.pressed
        hover: Styles.buttonPrimaryHover
        borderHover: colors.hover
        text: Styles.textOnColor

    }
}

