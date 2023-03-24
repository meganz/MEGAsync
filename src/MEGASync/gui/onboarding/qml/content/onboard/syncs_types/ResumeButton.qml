import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

Button {
    id: btn

    property string title: ""
    property string description: ""
    property string imageSource: ""
    property int type: InstallationTypeButton.Type.Sync

    Layout.preferredWidth: 230
    Layout.preferredHeight: parent.height
    checkable: true
    checked: false
    autoExclusive : true

    background: Rectangle {
        id: box

        border.width: 2
        radius: 8
        color: Styles.surface1
        border.color: btn.checked ? Styles.borderStrongSelected : Styles.borderStrong

        ColumnLayout {
            anchors.fill: parent

            Custom.SvgImage {
                id: icon

                color: btn.checked ? Styles.iconAccent : Styles.iconSecondary
                Layout.leftMargin: 16
                Layout.topMargin: 40
                source: imageSource
            }

            ColumnLayout {
                Layout.leftMargin: 24

                Text {
                    text: title
                    color: Styles.buttonPrimaryHover
                    Layout.preferredHeight: 24
                    font.pixelSize: 16
                    font.weight: Font.Bold
                    font.family: "Inter"
                    font.styleName: "normal"
                    lineHeight: 24
                }

                Text {
                    text: description
                    wrapMode: Text.WordWrap
                    lineHeightMode: Text.FixedHeight
                    Layout.preferredWidth: 182
                    color: Styles.toastBackground
                    Layout.preferredHeight: 64
                    font.pixelSize: 12
                    font.weight: Font.Light
                    font.family: "Inter"
                    font.styleName: "normal"
                    lineHeight: 16
                }
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


