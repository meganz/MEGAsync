// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

// QML common
import Components 1.0 as Custom
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    property alias renameTextField: renameTextField
    property alias headerDescription: header.description

    ColumnLayout {
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

            Layout.bottomMargin: mainLayout.spacing
            title: OnboardingStrings.renameBackupFolderTitle
            description: OnboardingStrings.renameBackupFolderDescription
        }

        Custom.TextField {
            id: renameTextField

            Layout.preferredWidth: parent.width + 2 * renameTextField.textField.focusBorderWidth
            Layout.preferredHeight: 48
            Layout.leftMargin: -4
            title: OnboardingStrings.renameBackupFolder
            leftIconSource: Images.database
        }

        Custom.NotificationText {
            id: hint

            visible: true
            Layout.fillWidth: true
            Layout.rightMargin: 4
            Layout.preferredHeight: hint.height
            type: Custom.NotificationText.Type.Info
            notificationText {
                url: Links.backupCentre
                urlColor: Styles.textInfo
                text: OnboardingStrings.renameBackupFolderHint
            }
        }
    }

}
