// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Components.Buttons 1.0 as MegaButtons
import Components.Texts 1.0 as MegaTexts
import Components.Views 1.0 as MegaViews
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

StackViewPage {
    id: root
    readonly property int contentHeight: 374

    property RegisterContent registerContent: RegisterContent {
        parent: scrollPanel.flickable.contentItem
    }

    property alias loginButton: loginButton
    property alias nextButton: nextButton

    Column {
        id: mainColumn

        anchors.left: root.left
        anchors.right: root.right
        anchors.top: root.top
        anchors.leftMargin: contentMargin
        anchors.rightMargin: contentMargin
        anchors.topMargin: contentMargin

        spacing: contentMargin / 2

        MegaTexts.RichText {
            id: title

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: registerContent.email.textField.focusBorderWidth
            font.pixelSize: MegaTexts.Text.Size.Large

            text: OnboardingStrings.signUpTitle
        }

        MegaViews.ScrollPanel {
            id: scrollPanel

            anchors.left: parent.left
            anchors.right: parent.right
            height: root.contentHeight
            flickable.contentHeight: registerContent.implicitHeight
            flickable.contentWidth: registerContent.implicitWidth
        }
    }

    RowLayout {
        anchors {
            right: root.right
            bottom: root.bottom
            left: root.left
            leftMargin: contentMargin
            rightMargin: contentMargin
            bottomMargin: bottomMargin
        }
        Layout.fillWidth: true

        MegaButtons.SecondaryButton {
            id: loginButton

            text: OnboardingStrings.login
            Layout.alignment: Qt.AlignLeft
        }

        MegaButtons.PrimaryButton {
            id: nextButton

            enabled: registerContent.dataLossCheckBox.checked
                     && registerContent.termsCheckBox.checked
            icons.source: Images.arrowRight
            text: OnboardingStrings.next

            Layout.alignment: Qt.AlignRight
        }
    }
}
