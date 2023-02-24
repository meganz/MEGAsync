

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
    property alias registerButton: registerButton
    property var formInfoMap: {
        "firstName": firstName.textField.text,
        "lastName": lastName.textField.text,
        "email": email.textField.text,
        "password": password.textField.text,
        "rePassword": repeatPassword.textField.text
    }
    property bool passwordError: false
    id: registerForm
    color: Styles.backgroundColor

    ColumnLayout {
        spacing: 12
        anchors {
            left: parent.left
            right: parent.right
            top: parent.top
            topMargin: 24
            leftMargin: 40
            rightMargin: 40
        }
        Custom.RichText {
            Layout.alignment: Qt.AlignCenter | Qt.AlignTop
            Layout.bottomMargin: 4
            font.pixelSize: 20
            text: qsTr("Create your [b]MEGA account[/b]")
        }
        RowLayout {
            Custom.TextField {
                id: firstName
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                placeholderText: qsTr("First name")
            }
            Custom.TextField {
                id: lastName
                Layout.fillWidth: true
                Layout.alignment: Qt.AlignTop
                placeholderText: qsTr("Last name")
            }
        }
        Custom.TextField {
            id: email
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Email")
        }
        Custom.PasswordTextField {
            id: password
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Password")
            error: passwordError
        }
        Custom.PasswordTextField {
            id: repeatPassword
            Layout.fillWidth: true
            Layout.alignment: Qt.AlignTop
            placeholderText: qsTr("Repeat password")
            error: passwordError
            showInformativeText: passwordError
            informativeText: qsTr("The password entered don’t match. Please try again.")
            informativeTextIcon: "images/eye.svg"
        }

        Custom.CheckBox {
            id: dataLossCheckBox
            Layout.fillWidth: true
            Layout.topMargin: 24
            richText.url: "http://www.stackoverflow.com/" //TODO: CHANGE LINK
            text: qsTr("I understand that if [b]I lose my password, I may lose my data[/b].
Read more about [a]MEGA’s end-to-end encryption.[/a]")
        }

        Custom.CheckBox {
            id: termsCheckBox
            Layout.fillWidth: true
            Layout.topMargin: 4
            richText.url: "http://www.stackoverflow.com/" //TODO: CHANGE LINK
            text: qsTr("I agree with MEGA [a]Terms of service.[/a]")
        }
    }
    RowLayout {
        spacing: 8
        id: buttonLayout
        anchors {
            right: parent.right
            bottom: parent.bottom
            rightMargin: 32
            bottomMargin: 24
        }
        Custom.Button {
            id: cancelButton
            text: qsTr("Cancel")
        }
        Custom.Button {
            id: loginButton
            text: qsTr("Login")
        }
        Custom.Button {
            id: registerButton
            primary: true
            iconRight: true
            iconSource: "../../../../../images/onboarding/arrow_right.svg"
            text: qsTr("Next")
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

