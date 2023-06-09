// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import ChooseLocalFolder 1.0

SelectFoldersPageForm {

    footerButtons {
        previousButton.onClicked: {
            mainFlow.state = syncType;
        }

        nextButton {
            onClicked: {
                proxyModel.updateConfirmed();
                backupsFlow.state = confirmBackup;
            }
        }
    }

    Component.onCompleted: {
        proxyModel.selectedFilterEnabled = false;
    }

    addFoldersMouseArea.onClicked: {
        folderDialog.openFolderSelector();
    }

    ChooseLocalFolder {
        id: folderDialog

        onFolderChanged: {
            proxyModel.insertFolder(folderDialog.getFolder());
        }
    }

}
