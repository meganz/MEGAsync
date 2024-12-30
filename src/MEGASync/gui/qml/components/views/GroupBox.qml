import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml
import QtQuick.Window 2.15

import common 1.0

import components.texts 1.0 as Texts

Qml.GroupBox {
    id: root

    property int borderRadius: 12
    property int borderWidth: 1

    topInset:  20

    background: Rectangle {
        radius: borderRadius
        color: "transparent"
        border {
            width: borderWidth
            color: ColorTheme.borderStrong
        }
    }

    label: Texts.Text {
        x: root.x + borderRadius
        font {
            pixelSize: Texts.Text.Size.SMALL
            weight: Font.DemiBold
        }
        text: title
    }
}
