

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

Item {
    id: root

    Image {
        id: image
        source: "../../../../images/Onboarding/login_folder.png"
        anchors.left: root.left
        anchors.verticalCenter: root.verticalCenter
    }

    Rectangle {
        id: rightRect
        anchors {
            left: image.right
            top: root.top
            bottom: root.bottom
            right: root.right
        }
        color: "#F6F6F6"

        ColumnLayout {
            spacing: 16
            anchors {
                verticalCenter: rightRect.verticalCenter
                left: rightRect.left
                right: rightRect.right
                leftMargin: 40
                rightMargin: 40
            }
            Custom.RichText {
                Layout.alignment: Qt.AlignCenter
                font.pixelSize: 20
                text: qsTr("Login into your [b]MEGA account[/b]")
                color: "#04101E"
            }
            Custom.TextField {
                Layout.fillWidth: true
                font.pixelSize: 14
                placeholderText: qsTr("Email")
            }
            Custom.PasswordTextField {
                Layout.fillWidth: true
                font.pixelSize: 14
                placeholderText: qsTr("Password")
            }
            Text {
                font.pixelSize: 14
                Layout.alignment: Qt.AlignCenter
                text: qsTr("Forgot password?")
            }
        }

        RowLayout {
            spacing: 8
            anchors {
                right: rightRect.right
                bottom: rightRect.bottom
                rightMargin: 32
                bottomMargin: 24
            }

            Custom.Button {
                id: createAccountButton
                text: qsTr("Create account")
                Layout.alignment: Qt.AlignRight
                palette {
                    button: "transparent"
                    windowText: "red"
                    buttonText: "white"
                }
            }

            Custom.Button {
                id: loginButton
                text: qsTr("Login")
                Layout.alignment: Qt.AlignRight
                palette {
                    button: "black"
                    buttonText: "white"
                }
            }
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

