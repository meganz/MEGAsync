// System
import QtQuick 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0

RenameBackupFolderPageForm {
    id: root

    footerButtons {

        previousButton.onClicked: {
            mainFlow.state = mainFlow.confirmBackup;
        }

        nextButton {
            text: OnboardingStrings.rename
            icons.source: Images.edit
            onClicked: {
                root.enabled = false;
                footerButtons.nextButton.icons.busyIndicatorVisible = true;
                Onboarding.createNextBackup(renameTextField.text);
            }
        }
    }

    hint.notificationText.onLinkActivated: {
        Qt.openUrlExternally(Links.backupCentre);
    }

    Connections {
        target: Onboarding

        onBackupConflict: (folder, name, isNew) => {
            root.enabled = true;
            footerButtons.nextButton.icons.busyIndicatorVisible = false;

            const regex = /\".*\"/;
            if(isNew) {
                headerDescription = headerDescription.replace(regex, "\"" + folder + "\"");
                renameTextField.text = folder + " (1)";
            } else {
                renameTextField.hint.text = renameTextField.hint.text.replace(regex, "\"" + name + "\"");
                renameTextField.hint.visible = true;
            }

            mainFlow.state = mainFlow.renameBackupFolder;
            renameTextField.textField.forceActiveFocus();
        }
    }

}
