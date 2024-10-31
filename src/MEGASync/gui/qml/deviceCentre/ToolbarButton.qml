import QtQuick 2.0

import common 1.0
import components.buttons 1.0

IconButton {
    id: root

    colors {
        text: ColorTheme.buttonPrimary
        textHover: ColorTheme.buttonPrimaryHover
        textPressed: ColorTheme.buttonPrimaryPressed
    }
    icons {
        position: Icon.Position.LEFT
    }

    sizes: SmallSizes {
        verticalPadding: 4
        horizontalPadding: 4
        spacing: 8
    }
}
