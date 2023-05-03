// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

Rectangle {
    id: root
    property alias email: email.text
    property alias changeEmailLinkText: changeEmailLinkText
    readonly property int contentMargin: 48

    ColumnLayout {
        id: layout
        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            rightMargin: contentMargin
            leftMargin: contentMargin
            topMargin: contentMargin
        }
        spacing: 24

        Custom.RichText {
            id: title

            Layout.fillWidth: true
            font.pixelSize: 20
            text: OnboardingStrings.confirmEmailTitle
            wrapMode: Text.WordWrap
        }

        Text {
            text: OnboardingStrings.confirmEmailBodyText
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            Layout.preferredWidth: layout.width
        }

        RowLayout {
            spacing: 9
            Custom.SvgImage {
                source: Images.mail
                sourceSize: Qt.size(24, 24)
                Layout.alignment: Qt.AlignVCenter
            }

            Text {
                id: email
                Layout.alignment: Qt.AlignVCenter
                font.pixelSize: 14
            }
        }

        Custom.RichText {
            id: changeEmailLinkText
            Layout.preferredWidth: layout.width
            font.pixelSize: 14
            text: OnboardingStrings.confirmEmailChangeText
            wrapMode: Text.WordWrap
            manageMouse: true
        }
    }
}

/*##^##
Designer {
    D{i:0;autoSize:true;height:480;width:640}
}
##^##*/

