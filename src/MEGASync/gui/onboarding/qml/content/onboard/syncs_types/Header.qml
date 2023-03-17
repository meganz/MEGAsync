import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

ColumnLayout {

    /*
     * Properties
     */

    property string title: ""
    property string description: ""

    /*
     * Child objects
     */

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
        Layout.preferredHeight: 40
        Layout.fillWidth: true
        font.family: "Inter"
        font.styleName: "normal"
        font.weight: Font.Light
        font.pixelSize: 14
        lineHeight: 20
        lineHeightMode: Text.FixedHeight
        wrapMode: Text.WordWrap
        maximumLineCount: 2
        Layout.fillHeight: false
    }

} // RowLayout -> mainLayout
