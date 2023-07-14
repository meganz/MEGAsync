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

        rightSecondary.onClicked: {
            syncsPanel.state = syncType;
        }

        rightPrimary {
            enabled: BackupsModel.mCheckAllState !== Qt.Unchecked
            onClicked: {
                BackupsModel.check();
                backupsProxyModel.selectedFilterEnabled = true;
                backupsFlow.state = confirmBackup;
            }
        }
    }

}
