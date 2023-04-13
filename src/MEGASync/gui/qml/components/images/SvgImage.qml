// System
import QtQuick 2.12
import QtGraphicalEffects 1.0
import QtQuick.Controls 2.12

// Local
import Common 1.0

Item {
    id: root

    property alias color: overlay.color
    property alias source: image.source
    property alias sourceSize: image.sourceSize
    property alias imageWidth: image.width
    property alias imageHeight: image.height

    width: image.width
    height: image.height

    Image {
        id: image

        anchors.centerIn: parent
    }

    ColorOverlay {
        id: overlay

        source: image
        color: color
        anchors.fill: image
    }
}
