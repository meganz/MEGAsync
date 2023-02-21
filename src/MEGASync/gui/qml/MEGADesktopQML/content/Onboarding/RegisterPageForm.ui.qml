

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
    id: registerForm
    color: Styles.backgroundColor

    Rectangle{
        anchors{
            top: parent.top
            right: parent.right
            left: parent.left
        }
    ColumnLayout {
        spacing: 8
        anchors {
            top: parent.top
            verticalCenter: parent.verticalCenter
            left: parent.left
            right: parent.right
            topMargin: 24
            leftMargin: 40
            rightMargin: 40
        }
        Layout.alignment: Qt.AlignTop
        Layout.fillHeight: false
        Custom.RichText {
            Layout.alignment: Qt.AlignCenter | Qt.AlignTop
            font.pixelSize: 20
            text: qsTr("Create your [b]MEGA account[/b]")
        }
        RowLayout {
            Custom.TextField {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                placeholderText: qsTr("First name")
            }
            Custom.TextField {
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                placeholderText: qsTr("Last name")
            }
        }
        Custom.TextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Email")
        }
        Custom.PasswordTextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Password")
        }
        Custom.PasswordTextField {
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Repeat password")
        }
        Custom.CheckBox {
            id: checkBox
            Layout.fillWidth: true
            text: qsTr("I understand that if I lose my password, I may lose my data. Read more about MEGA’s end-to-end encryption.")
            checked:true
            enabled:false
        }

        Custom.CheckBox {
            id: checkBox1
            Layout.fillWidth: true
            text: qsTr("I agree with MEGA Terms of service.")
            checked:true
        }
        Text {
            font.pixelSize: 14
            Layout.alignment: Qt.AlignCenter
            text: qsTr("Forgot password?")
            color: Styles.textColor
        }
    }
    }
    RowLayout {
        spacing: 8
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 32
            bottomMargin: 24
        }
        Custom.Button {
            id: createAccountButton
            text: qsTr("Create account")
        }

        Custom.Button {
            id: loginButton
            primary: true
            text: qsTr("Login")
        }
    }
}


/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

