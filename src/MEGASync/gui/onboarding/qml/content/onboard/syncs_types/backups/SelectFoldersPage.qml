// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import Onboard.Syncs_types 1.0

SelectFoldersPageForm {

    footerButtons {
        previousButton.onClicked: {
            syncsFlow.state = syncType;
        }

        nextButton {
            onClicked: {
                proxyModel.updateConfirmed();
                syncsFlow.state = confirmBackup;
            }
        }
    }

    Component.onCompleted: {
        proxyModel.selectedFilterEnabled = false;
    }

    addFoldersMouseArea.onClicked: {
        folderDialog.visible = true;
    }

    FileDialog {
        id: folderDialog

        title: "";
        folder: shortcuts.home;
        selectFolder: true
        onAccepted: {
            var processedFolder = folder.toString().substring(8);
            proxyModel.insertFolder(processedFolder);
        }
    }

}
