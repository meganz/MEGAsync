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

// C++
import Onboarding 1.0

SyncsPage {

    property alias folderField: folderField
    property alias proxyModel: backupTable.backupsProxyModel

    footerButtons.nextButton {
        text: OnboardingStrings.backup
        icons.source: Images.cloud
        icons.busyIndicatorImage: Images.loader
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
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
            }

            Custom.TextField {
                id: folderField

                Layout.preferredWidth: parent.width
                Layout.leftMargin: -folderField.textField.focusBorderWidth
                title: OnboardingStrings.backupTo
                text: "/Backups"
                leftIcon.source: Images.database
                textField.readOnly: true
                enabled: false
            }
        }
    }
}
