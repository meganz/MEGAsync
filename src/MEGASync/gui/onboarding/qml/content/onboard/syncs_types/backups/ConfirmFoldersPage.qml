// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

ConfirmFoldersPageForm {
    id: root

    property bool success: false

    footerButtons {

        previousButton.onClicked: {
            backupsProxyModel.selectedFilterEnabled = false;
            backupsFlow.state = backupsFlow.selectBackup;
        }

        nextButton.onClicked: {
            success = false;
            root.enabled = false;
            footerButtons.nextButton.icons.busyIndicatorVisible = true;
            backupsProxyModel.createBackups();
        }
    }

    Component.onCompleted: {
        console.error("Created Confirm Folders");
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
        target: _cppBackupsController

        onBackupsCreationFinished: {
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;
            syncsFlow.state = syncsFlow.finalState;
        }
    }

}
