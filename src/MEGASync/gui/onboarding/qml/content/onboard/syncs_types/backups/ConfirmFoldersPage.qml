// System
import QtQuick 2.12

// Local
import Onboard 1.0

ConfirmFoldersPageForm {

    footerButtons {
        previousButton.onClicked: {
            syncsFlow.state = selectBackup;
        }

        nextButton.text: OnboardingStrings.backup
        nextButton.iconSource: "../../../images/Onboarding/cloud.svg"
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
