// System
import QtQuick 2.15

// QML common
import common 1.0
//import components.texts 1.0
//import components.images 1.0

Row {
    id: root

    readonly property int iconWidth: 16

    property alias text: condition.text

    property bool checked: false

    width: parent.width
    spacing: iconWidth / 2

    /*
    SvgImage {
        id: image

        color: checked ? Styles.indicatorGreen : Styles.textSecondary
        source: checked ? Images.check : Images.smallCircle
        sourceSize: Qt.size(iconWidth, iconWidth)
    }
    */

    Text {
        id: condition

        width: parent.width - iconWidth
        font.strikeout: checked
        color: Styles.textSecondary
    }

}
