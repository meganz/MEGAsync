// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import ChooseLocalFolder 1.0

// C++
import BackupsModel 1.0

SelectFoldersPageForm {

    footerButtons {
        previousButton.onClicked: {
            mainFlow.state = syncType;
        }

        nextButton {
            enabled: BackupsModel.mCheckAllState !== Qt.Unchecked
            onClicked: {
                BackupsModel.checkBackups();
                backupsProxyModel.selectedFilterEnabled = true;
                backupsFlow.state = confirmBackup;
            }
        }
    }

}
