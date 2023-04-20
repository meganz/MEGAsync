// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

Rectangle {
    id: root

    property alias loginButton: loginButton
    property alias signUpButton: signUpButton
    property alias twoFAField: twoFAField

    color: Styles.backgroundColor

    ColumnLayout {

        spacing: 24
        anchors {
            verticalCenter: root.verticalCenter
            left: root.left
            right: root.right
            leftMargin: 48
            rightMargin: 48
        }

        Header {
            title: OnboardingStrings.twoFATitle
            description: OnboardingStrings.twoFASubtitle
        }

        Custom.TwoFA {
            id: twoFAField

            Layout.leftMargin: -3
            Layout.preferredWidth: parent.width
            Layout.fillHeight: true
        }

        Custom.HelpButton {
            text: OnboardingStrings.twoFANeedHelp
            url: Links.recovery
        }
    }

    RowLayout {
        spacing: 8
        anchors {
            right: root.right
            bottom: root.bottom
            rightMargin: 48
            bottomMargin: 24
        }

        Custom.Button {
            id: loginButton

            primary: true
            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
        }

        Custom.Button {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignRight
        }
    }
}
