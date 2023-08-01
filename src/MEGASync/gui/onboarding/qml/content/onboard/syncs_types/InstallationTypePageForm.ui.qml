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

    footerButtons.rightPrimary.enabled: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        Header {
            Layout.leftMargin: 3
            title: OnboardingStrings.welcomeToMEGA
            description: OnboardingStrings.choose
            spacing: 36
            descriptionWeight: Font.DemiBold
        }

        ButtonGroup {
            id: buttonGroup
        }

        ColumnLayout {
            spacing: 12

            SyncsHorizontalButton {
                id: syncButton

                title: OnboardingStrings.sync
                description: OnboardingStrings.syncButtonDescription
                imageSource: Images.sync
                type: SyncsType.Types.Sync
                ButtonGroup.group: buttonGroup
            }

            SyncsHorizontalButton {
                id: backupsButton

                title: OnboardingStrings.backUp
                description: OnboardingStrings.backupButtonDescription
                imageSource: Images.installationTypeBackups
                type: SyncsType.Backup
                ButtonGroup.group: buttonGroup
            }
        }
    }

}
