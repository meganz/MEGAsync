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
            syncsFlow.state = syncType;
        }

        nextButton {
            enabled: _cppBackupsModel.mCheckAllState !== Qt.Unchecked
            onClicked: {
                _cppBackupsModel.checkBackups();
                backupsProxyModel.selectedFilterEnabled = true;
                backupsFlow.state = confirmBackup;
            }
        }
    }

    addFoldersMouseArea.onClicked: {
        folderDialog.openFolderSelector();
    }
	
    ChooseLocalFolder {
        id: folderDialog

        onFolderChanged: {
            _cppBackupsModel.insertFolder(folderDialog.getFolder());
        }
    }

}
