// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

SyncsPage {

    property alias buttonGroup: buttonGroup

    footerButtons.nextButton.enabled: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: contentMargin
        }
        spacing: 24

        Header {
            title: OnboardingStrings.installationTypeTitle
            description: OnboardingStrings.installationTypeDescription
        }

        ButtonGroup {
            id: buttonGroup
        }

        ColumnLayout {
            spacing: 20

            SyncsHorizontalButton {
                id: syncButton

                title: OnboardingStrings.sync
                description: OnboardingStrings.syncButtonDescription
                imageSource: Images.sync
                type: SyncsType.Sync
                ButtonGroup.group: buttonGroup
            }

            SyncsHorizontalButton {
                id: backupsButton

                title: OnboardingStrings.backup
                description: OnboardingStrings.backupButtonDescription
                imageSource: Images.installationTypeBackups
                type: SyncsType.Backup
                ButtonGroup.group: buttonGroup
            }
        }
    }

}
