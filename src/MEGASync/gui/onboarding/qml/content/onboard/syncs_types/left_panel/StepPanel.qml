// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import Onboard 1.0

Rectangle {

    readonly property string step1ComputerName: "STEP1_COMPUTER_NAME"
    readonly property string step2InstallationType: "STEP2_INSTALLATION_TYPE"
    readonly property string stepBackupsSelectFolders: "STEP_BACKUPS_SELECT_FOLDERS"
    readonly property string stepBackupsConfirm: "STEP_BACKUPS_CONFIRM"
    readonly property string stepSelectSyncType: "STEP_SELECT_SYNC_TYPE"
    readonly property string stepSyncFolder: "STEP_SELECT_SYNC_FOLDER"

    color: Styles.alternateBackgroundColor
    height: parent.height

    state: step1ComputerName

    states: [
        State {
            name: step1ComputerName
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.Current
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonSecondaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Disabled
            }
            PropertyChanges {
                target: step3_1_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                visible: false;
            }
            PropertyChanges {
                target: step3_2_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_2_confirm;
                visible: false;
            }
        },
        State {
            name: step2InstallationType
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.Done
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Current
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.buttonSecondaryPressed
            }
            PropertyChanges {
                target: step3_syncs;
                toState: Step.ToStates.Disabled
            }
            PropertyChanges {
                target: step3_1_line;
                color: Styles.buttonSecondaryPressed
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                toState: SubStep.ToStates.Disabled
            }
            PropertyChanges {
                target: step3_1_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                visible: false;
            }
            PropertyChanges {
                target: step3_2_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_2_confirm;
                visible: false;
            }
        },
        State {
            name: stepBackupsSelectFolders
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step3_syncs;
                toState: Step.ToStates.Current
                title: OnboardingStrings.backup
            }
            PropertyChanges {
                target: step3_1_line;
                color: Styles.buttonPrimaryPressed
                visible: true
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                toState: SubStep.ToStates.Current
                visible: true
                title: OnboardingStrings.selectFolders
            }
            PropertyChanges {
                target: step3_2_line;
                color: Styles.buttonSecondaryPressed
                visible: true
            }
            PropertyChanges {
                target: step3_2_confirm;
                toState: SubStep.ToStates.Disabled
                visible: true
                title: OnboardingStrings.confirm
            }
        },
        State {
            name: stepBackupsConfirm
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step3_syncs;
                toState: Step.ToStates.DoneConfirm
                title: OnboardingStrings.backup
            }
            PropertyChanges {
                target: step3_1_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                toState: SubStep.ToStates.Done
                visible: true;
                title: OnboardingStrings.selectFolders
            }
            PropertyChanges {
                target: step3_2_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_2_confirm;
                toState: SubStep.ToStates.Current
                visible: true;
                title: OnboardingStrings.confirm
            }
        },
        State {
            name: stepSelectSyncType
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step3_syncs;
                toState: Step.ToStates.Current
                title: OnboardingStrings.sync
            }
            PropertyChanges {
                target: step3_1_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                toState: SubStep.ToStates.Current
                visible: true;
                title: OnboardingStrings.syncTitle
            }
            PropertyChanges {
                target: step3_2_line;
                color: Styles.buttonSecondaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_2_confirm;
                toState: SubStep.ToStates.Disabled
                visible: true;
                title: OnboardingStrings.selectFolders
            }
        },
        State {
            name: stepSyncFolder
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.DoneLight
            }
            PropertyChanges {
                target: step3_line;
                color: Styles.buttonPrimaryPressed
            }
            PropertyChanges {
                target: step3_syncs;
                toState: Step.ToStates.DoneConfirm
                title: OnboardingStrings.sync
            }
            PropertyChanges {
                target: step3_1_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_1_selectFolder;
                toState: SubStep.ToStates.Done
                visible: true;
                title: OnboardingStrings.syncTitle
            }
            PropertyChanges {
                target: step3_2_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_2_confirm;
                toState: SubStep.ToStates.Current
                visible: true;
                title: OnboardingStrings.selectFolders
            }
        }
    ]

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        ColumnLayout {
            id: stepsLayout

            width: parent.width
            Layout.alignment: Qt.AlignTop
            spacing: 4

            Step {
                id: step1_computerName

                title: OnboardingStrings.computerName
                Layout.leftMargin: 32
                Layout.topMargin: 40
            }

            Rectangle {
                id: step2_line

                color: Styles.buttonSecondaryPressed
                width: 2
                height: 56
                radius: 1
                Layout.leftMargin: 43
            }

            Step {
                id: step2_installationType

                title: OnboardingStrings.installationType
                Layout.leftMargin: 32
            }

            Rectangle {
                id: step3_line

                color: Styles.buttonSecondaryPressed
                width: 2
                height: 56
                radius: 1
                Layout.leftMargin: 43
            }

            Step {
                id: step3_syncs

                title: OnboardingStrings.synchronize
                Layout.leftMargin: 32
            }

            ColumnLayout {
                id: leftSubStepsLayout

                Layout.preferredWidth: 224
                spacing: 0

                Rectangle {
                    id: step3_1_line

                    color: Styles.buttonSecondaryPressed
                    Layout.preferredWidth: 2
                    Layout.preferredHeight: 12
                    radius: 1
                    Layout.leftMargin: 43
                }

                SubStep {
                    id: step3_1_selectFolder

                    title: OnboardingStrings.selectFolders
                    Layout.leftMargin: 40
                }

                Rectangle {
                    id: step3_2_line

                    color: Styles.buttonSecondaryPressed
                    Layout.preferredWidth: 2
                    Layout.preferredHeight: 12
                    radius: 1
                    Layout.leftMargin: 43
                }

                SubStep {
                    id: step3_2_confirm

                    title: OnboardingStrings.confirm
                    Layout.leftMargin: 40
                }
            }

        }

        Custom.HelpButton {
            Layout.leftMargin: 25
            Layout.bottomMargin: 25
            Layout.alignment: Qt.AlignBottom
            url: Links.desktopSyncApp
        }
    }

}
