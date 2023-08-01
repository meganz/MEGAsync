// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.Buttons 1.0 as MegaButtons

// C++
import Onboard 1.0

SyncsPage {
    id: finalPageRoot

    property alias buttonGroup: buttonGroup

    readonly property string stateFullSync: "FULL"
    readonly property string stateSelectiveSync: "SELECTIVE"
    readonly property string stateBackup: "BACKUP"

    property string title
    property string description

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
            name: finalPageRoot.stateFullSync
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepSync; }
        },
        State {
            name: finalPageRoot.stateSelectiveSync
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepSyncTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepSync; }
        },
        State {
            name: finalPageRoot.stateBackup
            PropertyChanges { target: titleItem; text: OnboardingStrings.finalStepBackupTitle; }
            PropertyChanges { target: descriptionItem; text: OnboardingStrings.finalStepBackup; }
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
            font.weight: Font.Light
        }

        MegaTexts.Text {
            Layout.preferredWidth: parent.width
            Layout.topMargin: 36
            text: OnboardingStrings.finalStepQuestion
            font.pixelSize: MegaTexts.Text.Size.MediumLarge
            font.weight: Font.DemiBold
        }

        Rectangle {
            Layout.preferredWidth: parent.width
            Layout.topMargin: 24
            color: "transparent"

            ButtonGroup {
                id: buttonGroup
            }

            RowLayout {
                spacing: 12
                anchors.fill: parent

                SyncsVerticalButton {
                    id: syncButton

                    title: OnboardingStrings.sync
                    description: OnboardingStrings.finalPageButtonSync
                    imageSource: Images.sync
                    ButtonGroup.group: buttonGroup
                    type: SyncsType.Sync
                    checkable: false
                    width: (finalPageRoot.state === finalPageRoot.stateFullSync)
                           ? parent.width
                           : (parent.width - parent.spacing) / 2
                    height: (finalPageRoot.state === finalPageRoot.stateFullSync)
                            ? 148
                            : 196
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    imageSourceSize: Qt.size(32, 32)
                    contentMargin: 24
                    contentSpacing: 8
                }

                SyncsVerticalButton {
                    id: backupsButton

                    title: OnboardingStrings.backup
                    description: OnboardingStrings.finalPageButtonBackup
                    imageSource: Images.installationTypeBackups
                    ButtonGroup.group: buttonGroup
                    type: SyncsType.Backup
                    checkable: false
                    width: (parent.width - parent.spacing) / 2
                    height: 196
                    Layout.preferredWidth: width
                    Layout.preferredHeight: height
                    imageSourceSize: Qt.size(32, 32)
                    visible: finalPageRoot.state !== finalPageRoot.stateFullSync
                    contentMargin: 24
                    contentSpacing: 8
                }
            }
        }
    }

}
