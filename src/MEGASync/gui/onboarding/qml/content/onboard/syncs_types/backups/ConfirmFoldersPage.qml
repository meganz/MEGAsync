// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0
import BackupsController 1.0
import BackupsModel 1.0

ConfirmFoldersPageForm {
    id: root

    property bool success: false

    footerButtons {

        rightSecondary.onClicked: {
            backupsProxyModel.selectedFilterEnabled = false;
            backupsFlow.state = backupsFlow.selectBackup;
        }

        rightPrimary.onClicked: {
            success = false;
            root.enabled = false;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Component.onCompleted: {
        Onboarding.getComputerName();
    }

    Connections {
        target: Onboarding

        onDeviceNameReady: (deviceName) => {
            folderField.textField.text = "/" + deviceName;
            folderField.enabled = true;
        }
    }

    Connections {
        target: BackupsModel

        onNoneSelected: {
            footerButtons.rightSecondary.clicked();
        }

        onExistConflictsChanged: {
            if(BackupsModel.mConflictsNotificationText !== "") {
                stepPanel.state = stepPanel.step4Warning;
            } else {
                stepPanel.state = stepPanel.step4;
            }
        }
    }

    Connections {
        target: BackupsController

        onBackupsCreationFinished: {
            root.enabled = true;
            footerButtons.rightPrimary.icons.busyIndicatorVisible = false;
            syncsPanel.state = syncsPanel.finalState;
        }
    }

}
