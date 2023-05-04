// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

// C++
import Onboarding 1.0

RenameBackupFolderPageForm {

    footerButtons {

        previousButton.onClicked: {
            syncsFlow.state = syncsFlow.confirmBackup;
        }

        nextButton {
            text: OnboardingStrings.rename
            iconSource: Images.edit
            onClicked: {
                Onboarding.createNextBackup(renameTextField.text);
            }
        }
    }

    hint.notificationText.onLinkActivated: {
        Qt.openUrlExternally(Links.backupCentre);
    }

    Connections {
        target: Onboarding

        onBackupConflict: (folder) => {
            const regex = /\".*\"/;
            headerDescription = headerDescription.replace(regex, "\"" + folder + "\"");
            renameTextField.text = folder + " (1)";
            syncsFlow.state = syncsFlow.renameBackupFolder;
            renameTextField.textField.forceActiveFocus();
        }
    }

}
