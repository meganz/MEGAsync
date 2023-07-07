// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Components.TextFields 1.0 as MegaTextFields
import Components.Texts 1.0 as MegaTexts
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

// C++
import Onboarding 1.0
import BackupsModel 1.0

SyncsPage {

    property alias folderField: folderField

    footerButtons.rightPrimary {
        text: OnboardingStrings.backup
        icons.source: Images.cloud
        enabled: !BackupsModel.mExistConflicts
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            title: OnboardingStrings.confirmBackupFoldersTitle
            description: OnboardingStrings.confirmBackupFoldersDescription
        }

        ColumnLayout {
            id: mainLayout

            Layout.preferredWidth: parent.width
            spacing: 24

            FoldersTable {
                id: backupTable

                Layout.preferredWidth: parent.width
                Layout.preferredHeight: 176
                model: backupsProxyModel
            }

            MegaTextFields.TextField {
                id: folderField

                Layout.preferredWidth: parent.width + 2 * folderField.sizes.focusBorderWidth
                Layout.leftMargin: -folderField.sizes.focusBorderWidth
                title: OnboardingStrings.backupTo
                text: "/Backups"
                leftIcon.source: Images.database
                textField.readOnly: true
                enabled: false
                visible: !BackupsModel.mExistConflicts
            }

            MegaTexts.NotificationText {
                Layout.preferredWidth: parent.width
                attributes.type: MegaTexts.NotificationInfo.Type.Warning
                attributes.icon.source: ""
                attributes.icon.visible: false
                text: BackupsModel.mConflictsNotificationText
                visible: BackupsModel.mExistConflicts
            }
        }
    }
}
