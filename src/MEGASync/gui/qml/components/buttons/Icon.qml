import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    enum Position {
        LEFT = 0,
        RIGHT,
        BOTH
    }

    property color colorEnabled: colorStyle.textInverseAccent
    property color colorDisabled: colorStyle.textDisabled
    property color colorHovered: colorStyle.textInverseAccent
    property color colorPressed: colorStyle.textInverseAccent
    property string source
    property int position: Icon.Position.RIGHT
    property int busyIndicatorPosition: Icon.Position.RIGHT
    property bool busyIndicatorVisible: false
}

