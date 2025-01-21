import QtQuick 2.15 as Qml

import common 1.0

Qml.Text {
    id: root

    enum Size {
        SMALL = 10,
        NORMAL = 12,
        MEDIUM = 14,
        MEDIUM_LARGE = 16,
        LARGE_SMALL = 18,
        LARGE = 20,
        EXTRA_LARGE = 24,
        HUGE = 48
    }

    property alias textMouseArea: mouseArea

    property bool handlePress: false

    font {
        family: FontStyles.fontFamily
        styleName: FontStyles.fontStyleName
        pixelSize: Text.Size.NORMAL
    }
    color: enabled ? ColorTheme.textPrimary : ColorTheme.textDisabled
    wrapMode: Text.WordWrap

    Qml.MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: { mouse.accepted = handlePress; }
    }

}
