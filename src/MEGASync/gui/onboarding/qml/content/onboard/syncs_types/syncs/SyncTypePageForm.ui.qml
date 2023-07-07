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

    footerButtons.rightPrimary.enabled: false

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
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
                    type: SyncsType.Types.FullSync
                    description: OnboardingStrings.fullSyncButtonDescription
                    imageSource: Images.fullSync
                    imageSourceSize: Qt.size(172, 100)
                    ButtonGroup.group: buttonGroup
                    textHorizontalExtraMargin: 4
                }

                SyncTypeButton {
                    id: backupsButton

                    title: OnboardingStrings.selectiveSync
                    type: SyncsType.Types.SelectiveSync
                    description: OnboardingStrings.selectiveSyncButtonDescription
                    imageSource: Images.selectiveSync
                    imageSourceSize: Qt.size(172, 100)
                    ButtonGroup.group: buttonGroup
                    textHorizontalExtraMargin: 4
                }
            }
        }
    }
}
