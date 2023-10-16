// System
import QtQuick 2.12
import QtQuick.Controls 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types.Left_panel 1.0
import Onboard.Syncs_types.Syncs 1.0
import Onboard.Syncs_types.Backups 1.0

// C++
import BackupsProxyModel 1.0
import LoginController 1.0

Rectangle {
    id: syncsPanel

    readonly property string deviceName: "deviceName"
    readonly property string syncType: "syncType"
    readonly property string syncsFlow: "syncs"
    readonly property string backupsFlow: "backups"
    readonly property string finalState: "finalState"

    readonly property int stepPanelWidth: 304
    readonly property int contentMargin: 48
    readonly property int contentHeight: 464
    readonly property int lineWidth: 2

    property NavigationInfo navInfo: NavigationInfo{}

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
                    navInfo.typeSelected = SyncsType.Types.Sync;
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
                    navInfo.typeSelected = SyncsType.Types.Backup;
                    rightPanel.replace(backupsFlowPage);
                }
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.step3;
            }
        },
        State {
            name: finalState
            StateChangeScript {
                script: rightPanel.replace(finalPage);
            }
            PropertyChanges {
                target: stepPanel;
                state: stepPanel.stepAllDone;
                step3Text: navInfo.typeSelected === SyncsType.Types.Backup
                           ? OnboardingStrings.backupSelectFolders
                           : OnboardingStrings.syncChooseType;
                step4Text: navInfo.typeSelected === SyncsType.Types.Backup
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

            anchors.fill: parent
            anchors.topMargin: contentMargin
            anchors.bottomMargin: contentMargin
            anchors.leftMargin: contentMargin
        }

        Rectangle {
            id: separatorLine

            width: lineWidth
            radius: lineWidth
            color: Styles.borderDisabled
            height: contentHeight
            anchors.right: leftPanel.right
            anchors.top: parent.top
            anchors.topMargin: contentMargin
        }
    }

    StackView {
        id: rightPanel

        anchors {
            left: leftPanel.right
            right: parent.right
            top: parent.top
            bottom: parent.bottom
            margins: contentMargin
        }

        replaceEnter: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 0
                to:1
                duration: 100
                easing.type: Easing.OutQuad
            }
        }
        replaceExit: Transition {
            PropertyAnimation {
                property: "opacity"
                from: 1
                to:0
                duration: 100
                easing.type: Easing.InQuad
            }
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
            id: finalPage

            ResumePage {}
        }
    }

    Connections {
        target: LogoutControllerAccess

        function onLogout() {
            onboardingWindow.forceClose();
        }
    }
}
