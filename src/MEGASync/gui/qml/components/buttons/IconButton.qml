import QtQuick.Layouts 1.15

import common 1.0

Button {
    id: root

    icons {
        colorEnabled: ColorTheme.iconButton
        colorDisabled: ColorTheme.iconButtonDisabled
        colorHovered: ColorTheme.iconButtonHover
        colorPressed: ColorTheme.iconButtonPressed
    }

    colors {
        background: "transparent"
        hover: "transparent"
        pressed: ColorTheme.iconButtonPressedBackground
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
