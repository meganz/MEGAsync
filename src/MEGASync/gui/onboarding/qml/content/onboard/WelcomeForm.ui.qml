import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom

Item {
    id: onboardingWindow

    property alias continueButton: continueButton

    anchors.fill: parent

    Image {
        source: "../../../../images/Onboarding/onboard_step1.png"
        anchors.bottom: bottomRect.top
        anchors.horizontalCenter: parent.horizontalCenter
    }

    Rectangle {
        id: bottomRect

        width: parent.width
        height: parent.height * 0.46
        anchors.bottom: parent.bottom
        color: "#04101E"

        ColumnLayout {
            id: clayout

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.top: parent.top
            spacing: 8

            Text {
                id: title

                text: qsTr("Welcome")
                color: "#FFFFFF"
                lineHeightMode: Text.FixedHeight
                lineHeight: 36
                font.pixelSize: 32
                Layout.topMargin: 24
                font.weight: Font.DemiBold
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignCenter
                horizontalAlignment: Text.AlignHCenter
            }

            Text {
                id: description

                text: qsTr("MEGA keeps all your files and folders in the cloud so you can easily access and share them on any device.")
                font.pixelSize: 14
                lineHeightMode: Text.FixedHeight
                lineHeight: 20
                horizontalAlignment: Text.AlignHCenter
                font.weight: Font.DemiBold
                color: "white"
                wrapMode: Text.WordWrap
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter
                Layout.maximumWidth: bottomRect.width / 1.5
            }

            Custom.Button {
                id: continueButton

                text: qsTr("Lets jump into it")
                font.pixelSize: 16
                Layout.topMargin: 36
                Layout.alignment: Qt.AlignHCenter | Qt.AlignVCenter

                contentItem: Text {
                    color: "white"
                    text: continueButton.text
                }
                background: Rectangle {
                    color: "#F0373A"
                    radius: 8
                }
            }
        }
    }

}
