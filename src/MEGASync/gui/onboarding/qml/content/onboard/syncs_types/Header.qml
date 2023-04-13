// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

ColumnLayout {

    property string title: ""
    property string description: ""

    Text {
        text: title
        Layout.preferredHeight: 20
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.DemiBold
        font.pixelSize: 20
        lineHeight: 30
    }

    Text {
        text: description
        Layout.topMargin: 12
        Layout.fillWidth: true
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.Light
        font.pixelSize: 14
        wrapMode: Text.WordWrap
    }

}
