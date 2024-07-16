import QtQuick 2.15

import common 1.0

InstallationTypePageForm {
    id: root

    signal installationTypeMoveToBack
    signal installationTypeMoveToSync
    signal installationTypeMoveToBackup

    footerButtons {

        rightSecondary.onClicked: {
            root.installationTypeMoveToBack();
        }

        rightPrimary.onClicked: {
            switch(buttonGroup.checkedButton.type) {
                case Constants.SyncType.SYNC:
                    root.installationTypeMoveToSync();
                    break;
                case Constants.SyncType.BACKUP:
                    root.installationTypeMoveToBackup();
                    break;
                default:
                    console.error("Button type does not exist -> "
                                  + buttonGroup.checkedButton.type);
                    break;
            }
        }

        leftPrimary {
            text: Strings.skip
            onClicked: {
            window.close();
            }
        }
    }

    buttonGroup.onCheckStateChanged: {
        if(buttonGroup.checkedButton != null) {
            footerButtons.rightPrimary.enabled = true;
        }
    }

    Connections {
        target: window

        function onInitializePageFocus() {
            syncButton.forceActiveFocus();
        }
    }
}
