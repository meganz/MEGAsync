import QtQuick 2.15
import QtQuick.Controls 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.views 1.0

import onboard 1.0
import onboard.syncs_types.backups 1.0

import LoginController 1.0
import SettingsDialog 1.0

Rectangle {
    id: root

    readonly property string deviceName: "deviceName"
    readonly property string syncType: "syncType"
    readonly property string syncsFlow: "syncs"
    readonly property string backupsFlow: "backups"
    readonly property string resume: "resume"

    readonly property int stepPanelWidth: 304
    readonly property int lineWidth: 2

    property NavigationInfo navInfo: NavigationInfo {}

    color: ColorTheme.surface1
    state: root.deviceName

    states: [
        State {
            name: root.deviceName
            StateChangeScript {
                script: rightPanel.replace(deviceNamePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step1DeviceName;
                helpButtonLink: Links.installAppsDesktop;
            }
        },
        State {
            name: root.syncType
            StateChangeScript {
                script: rightPanel.replace(installationTypePage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step2InstallationType;
                helpButtonLink: Links.installAppsDesktop;
            }
        },
        State {
            name: root.syncsFlow
            StateChangeScript {
                script: {
                    root.navInfo.typeSelected = Constants.SyncType.SYNC;
                    rightPanel.replace(syncsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
                helpButtonLink: Links.setUpSyncs;
            }
        },
        State {
            name: root.backupsFlow
            StateChangeScript {
                script: {
                    root.navInfo.typeSelected = Constants.SyncType.BACKUP;
                    rightPanel.replace(backupsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
                helpButtonLink: Links.createBackup;
            }
        },
        State {
            name: root.resume
            StateChangeScript {
                script: {
                    root.navInfo.comesFromResumePage = true;

                    var resumePageState = "";
                    var toOpenTabIndex = 0;
                    switch(root.navInfo.typeSelected) {
                        case Constants.SyncType.SELECTIVE_SYNC:
                            resumePageState = "stateSelectiveSync";
                            toOpenTabIndex = SettingsDialog.SYNCS_TAB;
                            break;
                        case Constants.SyncType.FULL_SYNC:
                            resumePageState = "stateFullSync";
                            toOpenTabIndex = SettingsDialog.SYNCS_TAB;
                            break;
                        case Constants.SyncType.BACKUP:
                            resumePageState = "stateBackup";
                            toOpenTabIndex = SettingsDialog.BACKUP_TAB;
                            break;
                        default:
                            console.warn("ResumePage: typeSelected does not exist -> "
                                         + root.navInfo.typeSelected);
                            break;
                    }

                    rightPanel.replace(resumePage,
                                       { "state": resumePageState,
                                         "tabToOpen": toOpenTabIndex,
                                         "fullSyncDone": root.navInfo.fullSyncDone,
                                         "selectiveSyncDone": root.navInfo.selectiveSyncDone
                                       });
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.stepAllDone;
                helpButtonLink: {
                    switch(root.navInfo.typeSelected) {
                        case Constants.SyncType.SELECTIVE_SYNC:
                        case Constants.SyncType.FULL_SYNC:
                            return Links.setUpSyncs;
                        case Constants.SyncType.BACKUP:
                            return Links.createBackup;
                        default:
                            return Links.installAppsDesktop;
                    }
                }
            }
        }
    ]

    Item {
        id: leftPanel

        width: root.stepPanelWidth
        height: parent.height
        z: 2

        StepPanel {
            id: stepPanel

            anchors {
                fill: parent
                margins: Constants.defaultWindowMargin
            }
        }

        Rectangle {
            id: separatorLine

            anchors {
                right: leftPanel.right
                top: parent.top
                bottom: parent.bottom
                topMargin: Constants.defaultWindowMargin
                bottomMargin: Constants.defaultWindowMargin
            }

            width: root.lineWidth
            radius: root.lineWidth
            height: root.contentHeight
            color: ColorTheme.borderDisabled
        }
    }

    StackViewBase {
        id: rightPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            margins: Constants.defaultWindowMargin
        }

        onCurrentItemChanged: {
            currentItem.setInitialFocusPosition();
        }

        Component {
            id: deviceNamePage

            DeviceNamePage {
                id: deviceNamePageItem

                onDeviceNameMoveToSyncType: {
                    root.state = root.syncType;
                }
            }
        }

        Component {
            id: installationTypePage

            InstallationTypePage {
                id: installationTypePageItem

                onInstallationTypeMoveToBack: {
                    root.state = root.deviceName;
                }

                onInstallationTypeMoveToSync: {
                    root.state = root.syncsFlow;
                }

                onInstallationTypeMoveToBackup: {
                    root.state = root.backupsFlow;
                }
            }
        }

        Component {
            id: syncsFlowPage

            SyncsPage {
                id: syncsFlowPageItem

                stepPanelRef: stepPanel
                navInfoRef: root.navInfo

                onSyncsFlowMoveToFinal: (syncType) => {
                    if(syncType === Constants.SyncType.FULL_SYNC) {
                        root.navInfo.fullSyncDone = true;
                    }
                    else if(syncType === Constants.SyncType.SELECTIVE_SYNC) {
                        root.navInfo.selectiveSyncDone = true;
                    }
                    root.state = root.resume;
                }

                onSyncsFlowMoveToBack: (fromSelectType) => {
                    if(root.navInfo.comesFromResumePage) {
                        root.navInfo.typeSelected = root.navInfo.previousTypeSelected;
                        root.state = root.resume;
                    }
                    else if(fromSelectType) {
                        root.state = root.syncType;
                    }
                    else {
                        syncsFlowPageItem.state = syncsFlowPageItem.syncType;
                    }
                }
            }
        }

        Component {
            id: backupsFlowPage

            BackupsPage {
                id: backupsFlowPageItem

                stepPanelRef: stepPanel

                onBackupFlowMoveToFinal: (success) => {
                    if(success) {
                        root.state = root.resume;
                    }
                }

                onBackupFlowMoveToBack: {
                    if(root.navInfo.comesFromResumePage) {
                        root.navInfo.typeSelected = root.navInfo.previousTypeSelected;
                        root.state = root.resume;
                    }
                    else {
                        root.state = root.syncType;
                    }
                }
            }
        }

        Component {
            id: resumePage

            ResumePage {
                id: resumePageItem

                stepPanelRef: stepPanel

                onResumePageMoveToSyncs: {
                    root.navInfo.previousTypeSelected = root.navInfo.typeSelected;
                    root.state = root.syncsFlow;
                }

                onResumePageMoveToSelectiveSyncs: {
                    root.navInfo.previousTypeSelected = root.navInfo.typeSelected;
                    root.state = root.syncsFlow;
                    root.navInfo.typeSelected = Constants.SyncType.SELECTIVE_SYNC;
                }

                onResumePageMoveToBackup: {
                    root.navInfo.previousTypeSelected = root.navInfo.typeSelected;
                    root.state = root.backupsFlow;
                }
            }
        }

    }

    Connections {
        id: logoutControllerAccessConnection

        target: logoutControllerAccess

        function onLogout() {
            window.forceClose();
        }
    }

}
