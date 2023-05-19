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

StackViewPage {
    id: root

    readonly property int contentMargin: 48
    readonly property int bottomMargin: 32
    readonly property int buttonSpacing: 8
    readonly property int contentHeight: 374

    property RegisterContent registerContent: RegisterContent {
        parent: scrollPanel.flickable.contentItem
    }

    property alias loginButton: loginButton
    property alias nextButton: nextButton

    color: Styles.pageBackground

    Column {
        id: mainColumn

        anchors.left: root.left
        anchors.right: root.right
        anchors.top: root.top
        anchors.leftMargin: contentMargin
        anchors.rightMargin: contentMargin
        anchors.topMargin: contentMargin

        spacing: contentMargin / 2

        Custom.RichText {
            id: title

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: registerContent.email.textField.focusBorderWidth
            font.pixelSize: Custom.Text.Size.Large

            text: OnboardingStrings.signUpTitle
        }

        Custom.ScrollPanel {
            id: scrollPanel

            anchors.left: parent.left
            anchors.right: parent.right
            height: root.contentHeight
            flickable.contentHeight: registerContent.implicitHeight
            flickable.contentWidth: registerContent.implicitWidth
        }
    }

    Row {
        anchors.right: root.right
        anchors.bottom: root.bottom
        anchors.rightMargin: contentMargin
        anchors.bottomMargin: bottomMargin
        spacing: buttonSpacing

        Custom.PrimaryButton {
            id: nextButton

            enabled: registerContent.dataLossCheckBox.checked
                     && registerContent.termsCheckBox.checked
            icons.source: Images.arrowRight
            text: OnboardingStrings.next
        }

        Custom.SecondaryButton {
            id: loginButton

            text: OnboardingStrings.login
        }
    }
}
