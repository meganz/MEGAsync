// Local
import Components.Buttons 1.0 as MegaButtons
import Common 1.0

MegaButtons.Button {

    icons {
        colorEnabled: Styles.buttonPrimary
        colorHovered: Styles.buttonPrimary
        colorPressed: Styles.buttonPrimary
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: Styles.buttonOutlineBackgroundHover
        border: Styles.buttonPrimary
        borderHover: Styles.buttonOutlineHover
        borderDisabled: Styles.buttonDisabled
        text: Styles.buttonPrimary
        textDisabled: Styles.textDisabled
        textHover: Styles.buttonOutlineHover
        textPressed: Styles.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: Styles.buttonOutlinePressed
    }
}

