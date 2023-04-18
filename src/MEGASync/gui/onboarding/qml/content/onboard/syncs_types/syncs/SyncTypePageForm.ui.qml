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

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 32
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

                ResumeButton {
                    id: fullSyncButton

                    title: OnboardingStrings.fullSyncTitle
                    type: ResumeButton.Type.FullSync
                    description: OnboardingStrings.fullSyncButtonDescription
                    imageSource: Images.fullSync
                    ButtonGroup.group: buttonGroup
                    imageSourceSize: Qt.size(200, 96)
                }

                ResumeButton {
                    id: backupsButton

                    title: OnboardingStrings.selectiveSyncTitle
                    type: ResumeButton.Type.SelectiveSync
                    description: OnboardingStrings.selectiveSyncButtonDescription
                    imageSource: Images.selectiveSync
                    ButtonGroup.group: buttonGroup
                    imageSourceSize: Qt.size(200, 96)
                }
            }
        }
    }
}
