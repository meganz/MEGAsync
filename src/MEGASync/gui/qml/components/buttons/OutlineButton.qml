import common 1.0

Button {
    id: root

    icons {
        colorEnabled: ColorTheme.buttonOutline
        colorHovered: ColorTheme.buttonOutlineHover
        colorPressed: ColorTheme.buttonOutlinePressed
    }

    colors {
        background: "transparent"
        disabled: "transparent"
        hover: ColorTheme.buttonOutlineBackgroundHover
        border: ColorTheme.buttonOutline
        borderHover: ColorTheme.buttonOutlineHover
        borderDisabled: ColorTheme.buttonDisabled
        text: ColorTheme.buttonOutline
        textHover: ColorTheme.buttonOutlineHover
        textPressed: ColorTheme.buttonOutlinePressed
        pressed: "transparent"
        borderPressed: ColorTheme.buttonOutlinePressed
    }
}

