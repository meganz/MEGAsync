import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom
import Onboard.Syncs_types.Backups 1.0

Item {

    /*
     * Functions
     */

    function next() {
        if(configurationStack.currentItemStack === configurationStack.itemStack.length-1) {
            console.error("ButtonContentPanel: configurationStack.currentItemStack out of bounds on next");
            return;
        }

        configurationStack.replace(configurationStack.itemStack[++configurationStack.currentItemStack],
                                   StackView.Immediate);
    }

    function previous() {
        if(configurationStack.currentItemStack === 0) {
            console.error("ButtonContentPanel: configurationStack.currentItemStack out of bounds on previous");
            return;
        }

        configurationStack.replace(configurationStack.itemStack[--configurationStack.currentItemStack],
                                   StackView.Immediate);
    }

    /*
     * Object properties
     */

    width: parent.width

    /*
     * Child objects
     */

    StackView {
        id: configurationStack

        property int currentItemStack: 0
        property var itemStack: [
            computerNamePage,
            installationTypePage,
            backupSelectFolders
        ]

        width: parent.width
        initialItem: computerNamePage

        ComputerNamePageForm {
            id: computerNamePage
        }

        InstallationTypePageForm {
            id: installationTypePage
        }

        SelectFoldersPageForm {
            id: backupSelectFolders
        }

    } //StackView -> configurationStack

} // Item
