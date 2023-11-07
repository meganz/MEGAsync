// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.Buttons 1.0 as MegaButtons

// C++
import Onboard 1.0

SyncsPage {
    id: root

    property string title
    property string description
    property bool fullSyncDone
    property bool selectiveSyncDone
    property alias buttonGroup: buttonGroupItem

    readonly property string stateFullSync: "FULL"
    readonly property string stateSelectiveSync: "SELECTIVE"
    readonly property string stateBackup: "BACKUP"

    footerButtons {
        leftSecondary.visible: false
        rightSecondary.text: OnboardingStrings.viewInSettings
        rightPrimary {
            text: OnboardingStrings.done
            icons: MegaButtons.Icon {}
        }
    }

    states: [
        State {
            name: stateFullSync
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepSync; }
            PropertyChanges { target: syncButton; visible: false; }
        },
        State {
            name: stateSelectiveSync
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepSync; }
            PropertyChanges {
                target: syncButton;
                type: SyncsType.SelectiveSync;
                visible: true;
            }
        },
        State {
            name: stateBackup
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepBackupTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepBackup; }
            PropertyChanges {
                target: syncButton;
                type: !fullSyncDone && !selectiveSyncDone ? SyncsType.Sync : SyncsType.SelectiveSync;
                visible: !fullSyncDone
                title: !fullSyncDone && !selectiveSyncDone ? OnboardingStrings.sync : OnboardingStrings.selectiveSync
                description: !fullSyncDone && !selectiveSyncDone ? OnboardingStrings.finalPageButtonSync : OnboardingStrings.finalPageButtonSelectiveSync
            }
        }
    ]

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }

        MegaTexts.Text {
            id: titleItem

            Layout.preferredWidth: parent.width
            text: title
            font.pixelSize: MegaTexts.Text.Size.Large
            font.weight: Font.Bold
        }

        MegaTexts.Text {
            id: descriptionItem

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
            text: description
            font.pixelSize: MegaTexts.Text.Size.Medium
        }

        MegaTexts.Text {
            Layout.preferredWidth: parent.width
            Layout.topMargin: 36
            text: OnboardingStrings.finalStepQuestion
            font.pixelSize: MegaTexts.Text.Size.MediumLarge
            font.weight: Font.DemiBold
        }

        Rectangle {
            Layout.preferredWidth: parent.width + 8
            Layout.topMargin: 24
            color: "transparent"

            ButtonGroup {
                id: buttonGroupItem
            }

            RowLayout {
                spacing: 12
                anchors.fill: parent
                anchors.leftMargin: -syncButton.focusBorderWidth
                anchors.rightMargin: backupsButton.focusBorderWidth

                SyncsVerticalButton {
                    id: syncButton

                    title: OnboardingStrings.selectiveSync
                    description: OnboardingStrings.finalPageButtonSelectiveSync
                    imageSource: Images.sync
                    ButtonGroup.group: buttonGroupItem
                    checkable: false
                    width: (parent.width - parent.spacing) / 2
                    height: 195
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    imageSourceSize: Qt.size(32, 32)
                    contentMargin: 24
                    contentSpacing: 8
                    focus: true
                }

                SyncsVerticalButton {
                    id: backupsButton

                    title: OnboardingStrings.backup
                    description: OnboardingStrings.finalPageButtonBackup
                    imageSource: Images.installationTypeBackups
                    ButtonGroup.group: buttonGroupItem
                    type: SyncsType.Backup
                    checkable: false
                    width: !syncButton.visible
                           ? parent.width
                           : (parent.width - parent.spacing) / 2
                    height: !syncButton.visible
                            ? 155
                            : 195
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    imageSourceSize: Qt.size(32, 32)
                    contentMargin: 24
                    contentSpacing: 8
                }
            }
        }
    }
}
