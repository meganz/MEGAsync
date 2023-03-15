import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0
import QtQuick.Controls.Styles 1.2

import Common 1.0

Item {

    /*
     * Enums
     */

    enum ToStates {
        Step1_ComputerName = 0,
        Current = 1,
        Done = 2
    }

    /*
     * Functions
     */

    function next() {
        if(stepsLayout.state === stepsLayout.stateStep1ComputerName) {
            stepsLayout.state = stepsLayout.stateStep2InstallationType;
        } else if(stepsLayout.state === stepsLayout.stateStep2InstallationType) {
            stepsLayout.state = stepsLayout.stateStep31SelectFolders;
        } else if(stepsLayout.state === stepsLayout.stateStep31SelectFolders) {
            stepsLayout.state = stepsLayout.stateStep32Confirm;
        }
    }

    function previous() {
        if(stepsLayout.state === stepsLayout.stateStep2InstallationType) {
            stepsLayout.state = stepsLayout.stateStep1ComputerName;
        } else if(stepsLayout.state === stepsLayout.stateStep31SelectFolders) {
            stepsLayout.state = stepsLayout.stateStep2InstallationType;
        } else if(stepsLayout.state === stepsLayout.stateStep32Confirm) {
            stepsLayout.state = stepsLayout.stateStep31SelectFolders;
        }
    }

    /*
     * Object properties
     */

    height: parent.height

    /*
     * Child objects
     */

    ColumnLayout {
        width: parent.width
        height: parent.height
        spacing: 0

        ColumnLayout {
            id: stepsLayout

            /*
             * Properties
             */
            readonly property string stateStep1ComputerName: "STEP1_COMPUTER_NAME"
            readonly property string stateStep2InstallationType: "STEP2_INSTALLATION_TYPE"
            readonly property string stateStep31SelectFolders: "STEP3_1_SELECT_FOLDERS"
            readonly property string stateStep32Confirm: "STEP3_2_CONFIRM"

            /*
             * Object properties
             */

            width: parent.width
            Layout.alignment: Qt.AlignTop
            spacing: 4

            state: stateStep1ComputerName

            states: [
                State {
                    name: stepsLayout.stateStep1ComputerName
                    PropertyChanges {
                        target: step1_computerName;
                        toState: BackupInfoStep.ToStates.Current
                    }
                    PropertyChanges {
                        target: step2_line;
                        color: Styles.buttonSecondaryPressed
                    }
                    PropertyChanges {
                        target: step2_installationType;
                        toState: BackupInfoStep.ToStates.Disabled
                    }
                },
                State {
                    name: stepsLayout.stateStep2InstallationType
                    PropertyChanges {
                        target: step1_computerName;
                        toState: BackupInfoStep.ToStates.Done
                    }
                    PropertyChanges {
                        target: step2_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step2_installationType;
                        toState: BackupInfoStep.ToStates.Current
                    }
                    PropertyChanges {
                        target: step3_line;
                        color: Styles.buttonSecondaryPressed
                    }
                    PropertyChanges {
                        target: step3_backup;
                        toState: BackupInfoStep.ToStates.Disabled
                    }
                    PropertyChanges {
                        target: step3_1_line;
                        color: Styles.buttonSecondaryPressed
                    }
                    PropertyChanges {
                        target: step3_1_selectFolder;
                        toState: BackupInfoSubStep.ToStates.Disabled
                    }
                },
                State {
                    name: stepsLayout.stateStep31SelectFolders
                    PropertyChanges {
                        target: step1_computerName;
                        toState: BackupInfoStep.ToStates.DoneLight
                    }
                    PropertyChanges {
                        target: step2_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step2_installationType;
                        toState: BackupInfoStep.ToStates.DoneLight
                    }
                    PropertyChanges {
                        target: step3_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step3_backup;
                        toState: BackupInfoStep.ToStates.Current
                    }
                    PropertyChanges {
                        target: step3_1_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step3_1_selectFolder;
                        toState: BackupInfoSubStep.ToStates.Current
                    }
                    PropertyChanges {
                        target: step3_2_line;
                        color: Styles.buttonSecondaryPressed
                    }
                    PropertyChanges {
                        target: step3_2_confirm;
                        toState: BackupInfoSubStep.ToStates.Disabled
                    }
                },
                State {
                    name: stepsLayout.stateStep32Confirm
                    PropertyChanges {
                        target: step1_computerName;
                        toState: BackupInfoStep.ToStates.DoneLight
                    }
                    PropertyChanges {
                        target: step2_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step2_installationType;
                        toState: BackupInfoStep.ToStates.DoneLight
                    }
                    PropertyChanges {
                        target: step3_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step3_backup;
                        toState: BackupInfoStep.ToStates.DoneConfirm
                    }
                    PropertyChanges {
                        target: step3_1_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step3_1_selectFolder;
                        toState: BackupInfoSubStep.ToStates.Done
                    }
                    PropertyChanges {
                        target: step3_2_line;
                        color: Styles.buttonPrimaryPressed
                    }
                    PropertyChanges {
                        target: step3_2_confirm;
                        toState: BackupInfoSubStep.ToStates.Current
                    }
                }
            ] // states

            BackupInfoStep {
                id: step1_computerName

                title: qsTr("Computer name")
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

            BackupInfoStep {
                id: step2_installationType

                title: qsTr("Installation type")
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

            BackupInfoStep {
                id: step3_backup

                title: qsTr("Synchronize")
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

                BackupInfoSubStep {
                    id: step3_1_selectFolder

                    title: qsTr("Select Folders")
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

                BackupInfoSubStep {
                    id: step3_2_confirm

                    title: qsTr("Confirm")
                    Layout.leftMargin: 40
                }

            } // ColumnLayout -> leftSubStepsLayout

        } // ColumnLayout -> stepsLayout

        Image {
            source: "../../../../images/Onboarding/help-circle.svg"
            Layout.leftMargin: 32
            Layout.topMargin: 173
            Layout.alignment: Qt.AlignTop

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: {
                    console.debug("TODO: Help clicked")
                }
            }
        }

    } // ColumnLayout

} // Item
