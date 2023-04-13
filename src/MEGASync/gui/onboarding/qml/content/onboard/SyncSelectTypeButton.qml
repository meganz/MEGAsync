// System
import QtQuick 2.12
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

Button {
    id: syncButton

    property string title: ""
    property string description: ""
    property string imageSource: ""

    Layout.preferredWidth: parent.width
    Layout.fillWidth: true
    Layout.preferredHeight: 96
    autoExclusive : true

    background: buttonBackground

    Rectangle {
        id: buttonBackground

        border.width: 2
        radius: 8
        color: Styles.surface1
        border.color: syncButton.checked ? Styles.borderStrongSelected : Styles.borderStrong

        ColumnLayout {
            anchors.fill: parent
            spacing: 16

            Custom.SvgImage {
                color: syncButton.checked ? Styles.iconAccent : Styles.iconSecondary
                Layout.leftMargin: 24
                source: imageSource

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
                    Layout.preferredWidth: 324
                    color: Styles.toastBackground
                    Layout.preferredHeight: 32
                    font.pixelSize: 10
                    font.weight: Font.Light
                    font.family: "Inter"
                    font.styleName: "normal"
                    lineHeight: 16
                }
            }
        }
    }

    MouseArea {
        anchors.fill: syncButton
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onClicked: syncButton.checked
    }

}
