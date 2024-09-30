import QtQuick 2.0

import components.buttons 1.0

IconButton {
    id: root

    colors {
        text: colorStyle.iconButton
        textHover: colorStyle.iconButtonHover
        textPressed: colorStyle.iconButtonPressed
    }
    icons {
        position: Icon.Position.LEFT
        colorHovered: colorStyle.iconButtonHover
        colorPressed: colorStyle.iconButtonPressed
    }

    sizes: SmallSizes {
        verticalPadding: 4
        horizontalPadding: 4
        spacing: 8
    }

}
