import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts

QtObject {
    id: root

    property bool isThin: false

    property size iconSize: Qt.size(iconWidth, iconWidth)

    property real horizontalPadding: 6.0
    property real verticalPadding: 2.0
    property real spacing: 4.0
    property real radius: 6.0
    property real borderWidth: root.isThin ? 1.0 : 2.0
    property real lineHeight: 20.0

    property int iconWidth: 16
    property int pixelSize: Texts.Text.Size.MEDIUM
    property int fontWeight: root.isThin ? Font.Thin : Font.DemiBold

}
