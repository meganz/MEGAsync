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
    readonly property string stepBackupsRename: "STEP_BACKUPS_RENAME"
    readonly property string stepSelectSyncType: "STEP_SELECT_SYNC_TYPE"
    readonly property string stepSyncFolder: "STEP_SELECT_SYNC_FOLDER"

    readonly property int lineLeftMargin: 42
    readonly property int stepLeftMargin: 32
    readonly property int subStepLeftMargin: 39
    readonly property int layoutTopMargin: 40
    readonly property int lineWidth: 2
    readonly property int lineMainStepHeight: 56
    readonly property int lineSubStepHeight: 12
    readonly property int lineRadius: 1
    readonly property int helpButtonMargin: 25

    color: Styles.pageBackground
    height: parent.height

    state: step1ComputerName

    states: [
        State {
            name: step1ComputerName
            PropertyChanges {
                target: step1_computerName;
                toState: Step.ToStates.Current;
            }
            PropertyChanges {
                target: step2_line;
                color: Styles.buttonSecondaryPressed;
            }
            PropertyChanges {
                target: step2_installationType;
                toState: Step.ToStates.Disabled;
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
                visible: false;
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
                visible: false;
            }
        },
        State {
            name: stepBackupsRename
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
                toState: SubStep.ToStates.Done
                visible: true;
                title: OnboardingStrings.confirm
            }
            PropertyChanges {
                target: step3_3_line;
                color: Styles.buttonPrimaryPressed
                visible: true;
            }
            PropertyChanges {
                target: step3_3_rename;
                toState: SubStep.ToStates.Current
                visible: true;
                title: OnboardingStrings.rename
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
                visible: false;
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
            PropertyChanges {
                target: step3_3_line;
                visible: false;
            }
            PropertyChanges {
                target: step3_3_rename;
                visible: false;
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
                Layout.leftMargin: stepLeftMargin
                Layout.topMargin: layoutTopMargin
            }

            Rectangle {
                id: step2_line

                color: Styles.buttonSecondaryPressed
                Layout.preferredWidth: lineWidth
                Layout.preferredHeight: lineMainStepHeight
                radius: lineRadius
                Layout.leftMargin: lineLeftMargin
            }

            Step {
                id: step2_installationType

                title: OnboardingStrings.installationType
                Layout.leftMargin: stepLeftMargin
            }

            Rectangle {
                id: step3_line

                color: Styles.buttonSecondaryPressed
                Layout.preferredWidth: lineWidth
                Layout.preferredHeight: lineMainStepHeight
                radius: lineRadius
                Layout.leftMargin: lineLeftMargin
            }

            Step {
                id: step3_syncs

                title: OnboardingStrings.synchronize
                Layout.leftMargin: stepLeftMargin
            }

            ColumnLayout {
                id: leftSubStepsLayout

                Layout.preferredWidth: parent.width
                spacing: 0

                Rectangle {
                    id: step3_1_line

                    color: Styles.buttonSecondaryPressed
                    Layout.preferredWidth: lineWidth
                    Layout.preferredHeight: lineSubStepHeight
                    radius: lineRadius
                    Layout.leftMargin: lineLeftMargin
                }

                SubStep {
                    id: step3_1_selectFolder

                    title: OnboardingStrings.selectFolders
                    Layout.leftMargin: subStepLeftMargin
                }

                Rectangle {
                    id: step3_2_line

                    color: Styles.buttonSecondaryPressed
                    Layout.preferredWidth: lineWidth
                    Layout.preferredHeight: lineSubStepHeight
                    radius: lineRadius
                    Layout.leftMargin: lineLeftMargin
                }

                SubStep {
                    id: step3_2_confirm

                    title: OnboardingStrings.confirm
                    Layout.leftMargin: subStepLeftMargin
                }

                Rectangle {
                    id: step3_3_line

                    color: Styles.buttonSecondaryPressed
                    Layout.preferredWidth: lineWidth
                    Layout.preferredHeight: lineSubStepHeight
                    radius: lineRadius
                    Layout.leftMargin: lineLeftMargin
                }

                SubStep {
                    id: step3_3_rename

                    title: OnboardingStrings.rename
                    Layout.leftMargin: subStepLeftMargin
                }
            }

        }
    }

    Custom.HelpButton {
        anchors.bottom: parent.bottom
        anchors.left: parent.left
        anchors.leftMargin: helpButtonMargin
        anchors.bottomMargin: helpButtonMargin
        url: Links.desktopSyncApp
    }

}
