import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

Button {
    id: resumeButton

    enum Type {
        SelectiveSync = 0,
        FullSync
    }

    property string title: ""
    property string description: ""
    property string imageSource: ""
    property int type: Type.Selective
    property size imageSourceSize: imageSourceSize

    Layout.preferredWidth: 230
    Layout.preferredHeight: parent.height
    checkable: true
    checked: false
    autoExclusive : true

    background: buttonBackground

    Rectangle {
        id: buttonBackground

        border.width: 2
        radius: 8
        color: Styles.surface1
        border.color: resumeButton.checked ? Styles.borderStrongSelected : Styles.borderStrong

        ColumnLayout {

            anchors{
                fill: buttonBackground
                leftMargin: 16
                topMargin: 24
                bottomMargin: 24
            }
            spacing: 16
            Layout.alignment: Qt.AlignCenter

            Custom.SvgImage {
                id: icon

                color: resumeButton.checked ? Styles.iconAccent : Styles.iconSecondary
                source: imageSource
                sourceSize: imageSourceSize
            }

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

    MouseArea {
        anchors.fill: resumeButton
        hoverEnabled: true
        cursorShape: Qt.PointingHandCursor
        onPressed:  mouse.accepted = false;
    }

}


