// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

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
        spacing: 32

        Header {
            title: OnboardingStrings.syncTitle
            description: OnboardingStrings.syncDescription
        }

        Rectangle {
            Layout.preferredHeight: 208
            Layout.preferredWidth: parent.width
            Layout.alignment: Qt.AlignLeft
            color: Styles.surface1

            ButtonGroup {
                id: buttonGroup
            }

            RowLayout {
                spacing: 8
                anchors.fill: parent

                SyncTypeButton {
                    id: fullSyncButton

                    title: OnboardingStrings.fullSync
                    syncType: SyncsType.SyncTypes.FullSync
                    description: OnboardingStrings.fullSyncButtonDescription
                    imageSource: Images.fullSync
                    ButtonGroup.group: buttonGroup
                }

                SyncTypeButton {
                    id: backupsButton

                    title: OnboardingStrings.selectiveSync
                    syncType: SyncsType.SyncTypes.SelectiveSync
                    description: OnboardingStrings.selectiveSyncButtonDescription
                    imageSource: Images.selectiveSync
                    ButtonGroup.group: buttonGroup
                }
            }
        }
    }
}
