// System
import QtQuick 2.15

// Local
import Common 1.0

QtObject {
    id: root

    enum Position {
        LEFT = 0,
        RIGHT,
        BOTH
    }

    property color colorEnabled: Styles.textInverseAccent
    property color colorDisabled: Styles.textDisabled
    property color colorHovered: Styles.textInverseAccent
    property color colorPressed: Styles.textInverseAccent
    property string source
    property int position: Icon.Position.RIGHT
    property int busyIndicatorPosition: Icon.Position.RIGHT
    property bool busyIndicatorVisible: false
}

