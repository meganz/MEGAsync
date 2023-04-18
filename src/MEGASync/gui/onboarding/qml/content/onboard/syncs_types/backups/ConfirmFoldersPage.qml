// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

ConfirmFoldersPageForm {

    footerButtons {
        previousButton.onClicked: {
            syncsFlow.state = selectBackup;
        }

        nextButton.text: OnboardingStrings.backup
        nextButton.iconSource: Images.cloud
        nextButton.onClicked: {
            syncsFlow.state = finalState;
        }
    }

    onVisibleChanged: {
        if(visible) {
            backupTable.backupProxyModel.selectedFilterEnabled = true;
        }
    }

}
