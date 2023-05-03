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

    property alias emailTextField: emailTextField
    property alias cancelButton: cancelButton
    property alias resendButton: resendButton

    readonly property int contentMargin: 48

    color: Styles.pageBackground

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

        Text {
            id: title

            Layout.fillWidth: true
            font.pixelSize: 20
            text: OnboardingStrings.changeEmailTitle
            wrapMode: Text.WordWrap
            color: Styles.textPrimary
        }

        Text {
            text: OnboardingStrings.changeEmailBodyText
            font.pixelSize: 14
            wrapMode: Text.WordWrap
            Layout.preferredWidth: layout.width
            color: Styles.textPrimary
        }

        Custom.EmailTextField {
            id: emailTextField

            title: OnboardingStrings.email
            Layout.preferredWidth: layout.width + 2 * focusWidth
            Layout.leftMargin: -focusWidth
        }
    }

    RowLayout {

        anchors {
            bottom: parent.bottom
            right: parent.right
            rightMargin: contentMargin
            bottomMargin: 32
        }
        spacing: 8

        Custom.OutlineButton {
            id: cancelButton

            text: OnboardingStrings.cancel
        }
        Custom.PrimaryButton {
            id: resendButton

            text: OnboardingStrings.resend
            iconSource: Images.mail
        }
    }
}
