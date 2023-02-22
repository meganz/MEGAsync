

/*
This is a UI file (.ui.qml) that is intended to be edited in Qt Design Studio only.
It is supposed to be strictly declarative and only uses a subset of QML. If you edit
this file manually, you might introduce QML code that is not supported by Qt Design Studio.
Check out https://doc.qt.io/qtcreator/creator-quick-ui-forms.html for details on .ui.qml files.
*/
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12
import Components 1.0 as Custom
import Styles 1.0

Rectangle {
    property alias createAccountButton: createAccountButton

    id: loginForm
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
            Layout.alignment: Qt.AlignCenter
            font.pixelSize: 20
            text: qsTr("Login into your [b]MEGA account[/b]")
        }
        Custom.TextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Email")
        }
        Custom.PasswordTextField {
            Layout.fillWidth: true
            placeholderText: qsTr("Password")
        }
        Text {
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Forgot password?")
            color: Styles.textColor
            font.pixelSize: 14
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

