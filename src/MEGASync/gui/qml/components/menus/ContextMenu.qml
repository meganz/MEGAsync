import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtGraphicalEffects 1.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0

Qml.Menu {
    id: root

    readonly property int defaultPadding: 4
    readonly property int defaultMenuWidth: 240
    readonly property int borderRadius: 8
    property Colors colors: Colors {}

    modal: false
    Qml.Overlay.modal: Rectangle {
        color: "transparent"
    }
    topPadding: defaultPadding
    bottomPadding: defaultPadding

    background: Rectangle {
        implicitWidth: defaultMenuWidth
        border.width: 1
        border.color: colors.menuBorder
        color: colors.menuBackground
        radius: borderRadius
        layer.enabled: true
        layer.effect: DropShadow {
            transparentBorder: true
            horizontalOffset: 0
            verticalOffset: 8
            radius: 8.0
            samples: 16
            cached: true
            color: colors.shadowColor
        }
    }

    onVisibleChanged: {
        if (visible && count > 0) {
            itemAt(0).forceActiveFocus();
        }
    }
}
