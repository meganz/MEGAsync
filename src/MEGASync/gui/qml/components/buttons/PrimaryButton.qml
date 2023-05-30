// Local
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

MegaButtons.Button {
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
        textPressed: Styles.textOnColor
        textHover: Styles.textOnColor
        textDisabled: Styles.textDisabled
    }
}

