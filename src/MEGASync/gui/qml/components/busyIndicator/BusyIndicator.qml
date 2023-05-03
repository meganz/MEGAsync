// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Controls.Styles 1.2

import Components 1.0 as Custom
import Common 1.0

Qml.BusyIndicator {
    id: root
    property alias color: iconImage.color
    property alias imageSource: iconImage.source
    property alias imageSize: iconImage.sourceSize

    width: imageSize.width
    height: imageSize.height

    contentItem: Custom.SvgImage {
        id: iconImage

        anchors.fill: parent
        anchors.centerIn: parent
        visible: root.visible

        RotationAnimator on rotation {
            running: root.visible
            loops: Animation.Infinite
            duration: 2000
            from: 0 ; to: 360
        }
    }
}
