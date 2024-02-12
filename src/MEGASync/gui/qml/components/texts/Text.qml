import QtQuick 2.15 as Qml

import common 1.0

Qml.Text {
    id: root

    enum Size {
        SMALL = 10,
        NORMAL = 12,
        MEDIUM = 14,
        MEDIUM_LARGE = 16,
        LARGE = 20,
        HUGE = 48
    }

    color: enabled ? colorStyle.textPrimary : colorStyle.textDisabled
    wrapMode: Text.WordWrap
    font {
        family: FontStyles.fontFamily
        styleName: FontStyles.fontStyleName
        pixelSize: Text.Size.NORMAL
    }
}
