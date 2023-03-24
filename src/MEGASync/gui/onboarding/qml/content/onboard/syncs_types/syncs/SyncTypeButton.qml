import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

Button {
    property string title: ""
    property string description: ""
    property string imageSource: ""

    id: btn
    //Layout.preferredHeight: 96
    checkable: true
    checked: false
    autoExclusive : true
    Layout.preferredWidth: parent.width
    Layout.fillWidth: true
    Layout.preferredHeight: parent.height
    width: 100
    height: 200
    background:
        Rectangle {
        id: box

        /*
         * Object properties
         */
        width: parent.width
        height: parent.height
        border.width: 2
        radius: 8
        color: Styles.surface1
        border.color: btn.checked ? Styles.borderStrongSelected : Styles.borderStrong
        anchors.fill: btn

        /*
         * Child objects
         */

        ColumnLayout {
            spacing: 16
            Custom.SvgImage {
                id: icon
                color: btn.checked ? Styles.iconAccent : Styles.iconSecondary
                Layout.leftMargin: 24
                source: imageSource
            }


            Text {
                text: title
                color: Styles.buttonPrimaryHover
                font.pixelSize: 16
                font.weight: Font.Bold
                font.family: "Inter"
                font.styleName: "normal"
                lineHeight: 24
                wrapMode: Text.WordWrap
                width: 100
            }

            Text {
                text: description
                wrapMode: Text.WordWrap
                lineHeightMode: Text.FixedHeight
                color: Styles.toastBackground
                font.pixelSize: 10
                font.weight: Font.Light
                font.family: "Inter"
                font.styleName: "normal"
                lineHeight: 16
                width: 100
            }

        }
    } // Rectangle

    MouseArea {
        anchors.fill: parent
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: {
            btn.checked = !btn.checked;
        }
    }

} // Button


