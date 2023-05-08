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

StackViewPage {
    id: root

    property alias loginButton: loginButton
    property alias signUpButton: signUpButton
    property alias twoFAField: twoFAField

    readonly property int contentMargin: 48
    readonly property int bottomMargin: 32
    
    color: Styles.pageBackground

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
            left: root.left
            leftMargin: contentMargin
            rightMargin: contentMargin
            bottomMargin: bottomMargin
        }

        Custom.OutlineButton {
            id: signUpButton

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        Custom.PrimaryButton {
            id: loginButton

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignRight
            busyIndicatorImage: Images.loader
            iconSource: Images.arrowRight
            progressBar: true
        }
    }
}
