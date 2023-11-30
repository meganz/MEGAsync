import QtQuick 2.15
import QtGraphicalEffects 1.15
import QtQuick.Controls 2.15

import common 1.0

Item {
    id: root

    property alias color: iconFill.color
    property alias source: image.source
    property alias sourceSize: image.sourceSize
    property alias imageWidth: image.width
    property alias imageHeight: image.height

    width: image.width
    height: image.height

    onColorChanged: {
        image.visible = false
        opacityMask.visible = true
    }

    Rectangle {
        id: iconFill

        anchors.fill: parent
        visible: false
    }

    Image {
        id: image

        anchors.centerIn: parent
        visible: true
    }

    OpacityMask {
        id: opacityMask

        anchors.fill: iconFill
        source: iconFill
        maskSource: image
        visible: false
    }
}
