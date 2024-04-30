import QtQuick.Layouts 1.15

import common 1.0

Button {
    id: root

    icons {
        colorEnabled: ColorTheme.buttonPrimary
        colorDisabled: ColorTheme.ButtonDisabled
        colorHovered: ColorTheme.ButtonHover
        colorPressed: ColorTheme.ButtonPressed
    }

    colors {
        background: "transparent"
        hover: "transparent"
        pressed: ColorTheme.surface2
        border: "transparent"
        borderDisabled: "transparent"
        borderHover: "transparent"
        borderSelected: "transparent"
        borderPressed: "transparent"
        disabled: "transparent"
    }

    sizes {
        iconWidth: 24
        verticalPadding: 4
        horizontalPadding: 4
    }
}
