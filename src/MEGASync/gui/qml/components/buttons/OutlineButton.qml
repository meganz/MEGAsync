// Local
import Components 1.0 as Custom
import Common 1.0

Custom.Button {
    icons.color: Styles.buttonPrimary
    colors {
        background: "transparent"
        disabled: "transparent"
        hover: Styles.buttonOutlineBackgroundHover
        border: Styles.buttonPrimary
        borderHover: Styles.buttonOutlineHover
        borderDisabled: Styles.buttonDisabled
        text: Styles.buttonPrimary
        disabledText: Styles.textDisabled
        pressed: "transparent"
        borderPressed: Styles.buttonOutlinePressed
    }
}

