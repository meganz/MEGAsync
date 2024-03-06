import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.buttons 1.0
import components.textFields 1.0
import components.texts 1.0 as Texts
import components.pages 1.0

import LoginController 1.0

StackViewPage {
    id: root

    property alias loginButton: loginButtonItem
    property alias signUpButton: signUpButtonItem
    property alias twoFAField: twoFAItem

    ColumnLayout {
        id: mainColumn

        anchors {
            left: root.left
            right: root.right
            top: root.top
            topMargin: 110
        }
        spacing: contentSpacing

        HeaderTexts {
            id: headerItem

            spacing: contentSpacing / 2
            title: OnboardingStrings.twoFATitle
            titleWrapMode: Text.Wrap
            description: OnboardingStrings.twoFASubtitle
            titleWeight: Font.Normal
            descriptionFontSize: Texts.Text.Size.NORMAL
        }

        TwoFA {
            id: twoFAItem

            Layout.fillWidth: true
            focus: true
        }
    }

    RowLayout {
        id: buttonLayout

        anchors {
            left: root.left
            right: root.right
            bottom: root.bottom
            leftMargin: -signUpButtonItem.sizes.focusBorderWidth
            rightMargin: -signUpButtonItem.sizes.focusBorderWidth
            bottomMargin: Constants.defaultWindowMargin - signUpButtonItem.sizes.focusBorderWidth
        }

        OutlineButton {
            id: signUpButtonItem

            text: OnboardingStrings.signUp
            Layout.alignment: Qt.AlignLeft
        }

        PrimaryButton {
            id: loginButtonItem

            text: OnboardingStrings.login
            progressValue: loginControllerAccess.progress
            Layout.alignment: Qt.AlignRight
        }
    }

}
