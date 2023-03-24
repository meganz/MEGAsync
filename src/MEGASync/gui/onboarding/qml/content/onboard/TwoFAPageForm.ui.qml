import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

import Components 1.0 as Custom
import Common 1.0

Rectangle {
    id: loginForm

    property alias cancelButton: cancelButton
    property alias acceptButton: acceptButton
    property string key: twoFAField.key2fa

    color: Styles.backgroundColor

    ColumnLayout {

        spacing: 16
        anchors {
            verticalCenter: loginForm.verticalCenter
            left: loginForm.left
            right: loginForm.right
            leftMargin: 40
            rightMargin: 40
        }

        Custom.RichText {
            Layout.alignment: Qt.AlignLeft
            font.pixelSize: 20
            text: qsTr("Continue with [b]two factor authentication[/b]")
        }

        Text {
            Layout.alignment: Qt.AlignLeft
            text: qsTr("Enter the 6-digit Google Authenticator code for this account")
            color: Styles.textColor
            font.pixelSize: 14
        }

        Custom.TwoFA {
            id: twoFAField

            Layout.preferredWidth: parent.width
        }
    }

    RowLayout {
        spacing: 8
        anchors {
            right: loginForm.right
            bottom: loginForm.bottom
            rightMargin: 32
            bottomMargin: 24
        }

        Custom.Button {
            id: cancelButton

            text: qsTr("Close")
            Layout.alignment: Qt.AlignRight
        }

        Custom.Button {
            id: acceptButton

            primary: true
            text: qsTr("Accept")
            Layout.alignment: Qt.AlignRight
        }
    }
}
