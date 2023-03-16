import Components 1.0 as Custom
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Common 1.0
import Onboarding 1.0

Rectangle {
    property alias createAccountButton: createAccountButton
    property alias loginButton: loginButton
    property alias forgotPasswordHyperlinkArea: forgotPasswordHyperlinkArea

    property string email: email.textField.text
    property string password: password.textField.text

    id: loginForm
    color: Styles.backgroundColor
    Connections {
        target: Onboarding
        onUserPassFailed: {
            email.error = true
            password.error = true
        }
    }
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
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            text: qsTr("Login into your [b]MEGA account[/b]")
        }

        Custom.TextField {
            id: email
            Layout.fillWidth: true
            placeholderText: qsTr("Email")
        }

        Custom.PasswordTextField {
            id: password
            Layout.fillWidth: true
            placeholderText: qsTr("Password")
        }

        Text {
            id: forgotPassword
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Forgot password?")
            color: Styles.textColor
            font.pixelSize: 14

            MouseArea {
                id: forgotPasswordHyperlinkArea
                anchors.fill: parent
                hoverEnabled: true
                cursorShape: Qt.PointingHandCursor
            }
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
            id: createAccountButton
            text: qsTr("Create account")
            Layout.alignment: Qt.AlignRight
        }

        Custom.Button {
            id: loginButton
            primary: true
            text: qsTr("Login")
            Layout.alignment: Qt.AlignRight
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/
