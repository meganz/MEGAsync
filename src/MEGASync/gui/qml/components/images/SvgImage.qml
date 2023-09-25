// System
import QtQuick 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.12

// Local
import Common 1.0

Item {
    id: root

    property alias color: iconFill.color
    property alias source: image.source
    property alias sourceSize: image.sourceSize
    property alias imageWidth: image.width
    property alias imageHeight: image.height

    width: image.width
    height: image.height

    Rectangle {
        id: iconFill
        anchors.fill: parent
        visible: false
    }

    Image {
        id: image
        anchors.centerIn: parent
        visible: false
    }

    OpacityMask {
        anchors.fill: iconFill
        source: iconFill
        maskSource: image
    }
}
