import QtQuick.Layouts 1.15

import common 1.0

Button {
    id: root

    function getSource() {
        if(!icons.manageSource) {
            return icons.source;
        }

        if(root.pressed || root.checked) {
            return icons.sourcePressed;
        }
        if(root.hovered) {
            return icons.sourceHovered;
        }
        if(!root.enabled) {
            return icons.sourceDisabled;
        }
        return icons.sourceEnabled;
    }

    icons {
        colorEnabled: ColorTheme.buttonPrimary
        colorDisabled: ColorTheme.buttonDisabled
        colorHovered: ColorTheme.buttonPrimaryHover
        colorPressed: ColorTheme.buttonPrimaryPressed
        source: getSource()
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
