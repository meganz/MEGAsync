// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    property alias renameTextField: renameTextField
    property alias headerDescription: header.description
    property alias hint: hint

    Column {
        id: mainLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: contentMargin
        }
        spacing: 24

        Header {
            id: header

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: renameTextField.textField.focusBorderWidth
            anchors.rightMargin: renameTextField.textField.focusBorderWidth
            title: OnboardingStrings.renameBackupFolderTitle
            description: OnboardingStrings.renameBackupFolderDescription
        }

        Custom.TextField {
            id: renameTextField

            anchors.left: parent.left
            anchors.right: parent.right
            title: OnboardingStrings.renameBackupFolder
            leftIcon.source: Images.database
            hint.text: OnboardingStrings.bakupFolderExistsError
            hint.icon: Images.alertTriangle
        }

        Custom.NotificationText {
            id: hint

            anchors.left: parent.left
            anchors.right: parent.right
            anchors.leftMargin: renameTextField.textField.focusBorderWidth
            anchors.rightMargin: renameTextField.textField.focusBorderWidth
            visible: true
            type: Custom.NotificationText.Type.Info
            notificationText {
                text: OnboardingStrings.renameBackupFolderHint
                manageMouse: true
            }
        }
    }
}
