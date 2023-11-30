import common 1.0

Button {
    id: root

    icons {
        colorEnabled: Styles.buttonOutline
        colorHovered: Styles.buttonOutlineHover
        colorPressed: Styles.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: Styles.buttonOutlineBackgroundHover
        border: Styles.buttonOutline
        borderHover: Styles.buttonOutlineHover
        borderDisabled: Styles.buttonDisabled
        text: Styles.buttonOutline
        textHover: Styles.buttonOutlineHover
        textPressed: Styles.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: Styles.buttonOutlinePressed
    }
}

