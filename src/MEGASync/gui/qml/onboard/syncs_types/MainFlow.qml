import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import onboard 1.0
import onboard.syncs_types.left_panel 1.0
import onboard.syncs_types.syncs 1.0
import onboard.syncs_types.backups 1.0

import BackupsProxyModel 1.0
import LoginController 1.0
import SettingsDialog 1.0

Rectangle {
    id: syncsPanel

    readonly property string deviceName: "deviceName"
    readonly property string syncType: "syncType"
    readonly property string syncsFlow: "syncs"
    readonly property string backupsFlow: "backups"
    readonly property string resume: "resume"

    readonly property int stepPanelWidth: 304
    readonly property int contentMargin: 48
    readonly property int contentHeight: 464
    readonly property int lineWidth: 2

    property NavigationInfo navInfo: NavigationInfo {}

    color: Styles.surface1
    state: deviceName
    states: [
        State {
            name: deviceName
            StateChangeScript {
                script: rightPanel.replace(deviceNamePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step1DeviceName;
            }
        },
        State {
            name: syncType
            StateChangeScript {
                script: rightPanel.replace(installationTypePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step2InstallationType;
            }
        },
        State {
            name: syncsFlow
            StateChangeScript {
                script: {
                    navInfo.typeSelected = SyncsType.Types.SYNC;
                    rightPanel.replace(syncsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
            }
        },
        State {
            name: backupsFlow
            StateChangeScript {
                script: {
                    navInfo.typeSelected = SyncsType.Types.BACKUP;
                    rightPanel.replace(backupsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
            }
        },
        State {
            name: resume
            StateChangeScript {
                script: {
                    syncsPanel.navInfo.comesFromResumePage = true;

                    var resumePageState = "";
                    var toOpenTabIndex = 0;
                    switch(syncsPanel.navInfo.typeSelected) {
                        case SyncsType.Types.SELECTIVE_SYNC:
                            resumePageState = "SELECTIVE";
                            toOpenTabIndex = SettingsDialog.SYNCS_TAB;
                            break;
                        case SyncsType.Types.FULL_SYNC:
                            resumePageState = "FULL";
                            toOpenTabIndex = SettingsDialog.SYNCS_TAB;
                            break;
                        case SyncsType.Types.BACKUP:
                            resumePageState = "BACKUP";
                            toOpenTabIndex = SettingsDialog.BACKUP_TAB;
                            break;
                        default:
                            console.warn("ResumePage: typeSelected does not exist -> "
                                         + syncsPanel.navInfo.typeSelected);
                            break;
                    }

                    rightPanel.replace(resumePage,
                                       { "state": resumePageState,
                                         "tabToOpen": toOpenTabIndex,
                                         "fullSyncDone": syncsPanel.navInfo.fullSyncDone,
                                         "selectiveSyncDone": syncsPanel.navInfo.selectiveSyncDone
                                       });
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.stepAllDone;
                step3Text: navInfo.typeSelected === SyncsType.Types.BACKUP
                           ? OnboardingStrings.backupSelectFolders
                           : OnboardingStrings.syncChooseType;
                step4Text: navInfo.typeSelected === SyncsType.Types.BACKUP
                           ? OnboardingStrings.backupConfirm
                           : OnboardingStrings.syncSetUp;
            }
        }
    ]

    Rectangle {
        id: leftPanel

        width: stepPanelWidth
        height: parent.height
        color: Styles.surface1
        z: 2

        StepPanel {
            id: stepPanel

            anchors {
                fill: parent
                topMargin: contentMargin
                bottomMargin: contentMargin
                leftMargin: contentMargin
            }
        }

        Rectangle {
            id: separatorLine

            anchors {
                right: leftPanel.right
                top: parent.top
                topMargin: contentMargin
            }
            width: lineWidth
            height: contentHeight
            radius: lineWidth
            color: Styles.borderDisabled
        }
    }

    StackViewBase {
        id: rightPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            margins: contentMargin
        }

        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
        }

        Component {
            id: deviceNamePage

            DeviceNamePage {}
        }

        Component {
            id: installationTypePage

            InstallationTypePage {}
        }

        Component {
            id: syncsFlowPage

            SyncsFlow {}
        }

        Component {
            id: backupsFlowPage

            BackupsFlow {}
        }

        Component {
            id: resumePage

            ResumePage {}
        }
    }

    Connections {
        target: logoutControllerAccess

        function onLogout() {
            onboardingWindow.forceClose();
        }
    }

    /*
     * Navigation connections
     */

    Connections {
        id: deviceNameNavigationConnection

        target: rightPanel.currentItem
        ignoreUnknownSignals: true

        function onDeviceNameMoveToSyncType() {
            syncsPanel.state = syncType
        }
    }

    Connections {
        id: installationTypeNavigationConnection

        target: rightPanel.currentItem
        ignoreUnknownSignals: true

        function onInstallationTypeMoveToBack() {
            syncsPanel.state = deviceName
        }

        function onInstallationTypeMoveToSync() {
            syncsPanel.state = syncsFlow;
        }

        function onInstallationTypeMoveToBackup() {
            syncsPanel.state = backupsFlow;
        }
    }

    Connections {
        id: syncsFlowNavigationConnection

        target: rightPanel.currentItem
        ignoreUnknownSignals: true        

        function onSyncsFlowMoveToFinal() {
            syncsPanel.state = resume;
        }

        function onSyncsFlowMoveToBack() {
            syncsPanel.state = syncType;
        }
    }

    Connections {
        id: backupFlowNavigationConnection

        target: rightPanel.currentItem
        ignoreUnknownSignals: true

        function onBackupFlowMoveToFinal() {
            syncsPanel.state = resume;
        }

        function onBackupFlowMoveToBack() {
            syncsPanel.state = syncType;
        }
    }

    Connections {
        id: resumePageNavigationConnection

        target: rightPanel.currentItem
        ignoreUnknownSignals: true

        function onResumePageMoveToSyncs() {
            syncsPanel.navInfo.previousTypeSelected = syncsPanel.navInfo.typeSelected;
            syncsPanel.state = syncsFlow;
        }

        function onResumePageMoveToSelectiveSyncs() {
            syncsPanel.navInfo.previousTypeSelected = syncsPanel.navInfo.typeSelected;
            syncsPanel.state = syncsFlow;
            syncsPanel.navInfo.typeSelected = SyncsType.Types.SELECTIVE_SYNC;
        }

        function onResumePageMoveToBackup() {
            syncsPanel.navInfo.previousTypeSelected = syncsPanel.navInfo.typeSelected;
            syncsPanel.state = backupsFlow;
        }
    }
}
