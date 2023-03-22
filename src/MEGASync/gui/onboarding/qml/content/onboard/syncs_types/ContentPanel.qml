import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Components 1.0 as Custom
import Onboard.Syncs_types.Backups 1.0

Item {
    id: mainItem

    /*
     * Functions
     */

    function nextPage() {
        if(!configurationStack.currentItem.isStacked) {
            configurationStack.replace(configurationStack.currentItem.next,
                                       StackView.Immediate);
        } else {
            configurationStack.currentItem.nextPage();
        }
    }

    function previousPage() {
        if(!configurationStack.currentItem.isStacked
                || configurationStack.currentItem.isFirstPage) {

            configurationStack.replace(configurationStack.currentItem.previous,
                                       StackView.Immediate);
        } else {
            configurationStack.currentItem.previousPage();
        }
    }

    function showPage(type) {
        var item;

        // Reset the corresponding sub-stack
        switch(type) {
            case InstallationTypeButton.Type.Sync:
                console.debug("TODO: Add Sync page");
                break;
            case InstallationTypeButton.Type.Backup:
                backupPage.resetToInitialPage();
                item = backupPage;
                break;
            case InstallationTypeButton.Type.Fuse:
                console.debug("TODO: Add Fuse page");
                break;
            default:
                console.error("Undefined option clicked -> " + option);
                return;
        }

        // Only if the new type is different from the last one then the stack is replaced
        if(configurationStack.lastTypeSelected !== type) {
            configurationStack.replace(item, StackView.Immediate);
        }
    }

    /*
     * Properties
     */

    property alias installationTypePage: installationTypePage
    property Footer footerLayout

    /*
     * Signals
     */

    signal optionSelected

    /*
     * Object properties
     */

    width: parent.width

    /*
     * Child objects
     */

    StackView {
        id: configurationStack

        property int lastTypeSelected: -1

        width: parent.width
        initialItem: computerNamePage

        ComputerNamePageForm {
            id: computerNamePage

            next: installationTypePage
            footerLayout: mainItem.footerLayout
            visible: false
        }

        InstallationTypePageForm {
            id: installationTypePage

            previous: computerNamePage
            footerLayout: mainItem.footerLayout
            visible: false
        }

        BackupPage {
            id: backupPage

            previous: installationTypePage
            footerLayout: mainItem.footerLayout
            isStacked: true
            visible: false
        }

        Connections {
            target: installationTypePage.content

            onOptionChanged: (type) => {
                switch(type) {
                    case InstallationTypeButton.Type.Sync:
                        console.debug("TODO: Sync clicked");
                        break;
                    case InstallationTypeButton.Type.Backup:
                        console.debug("Backup clicked");
                        installationTypePage.next = backupPage;
                        break;
                    case InstallationTypeButton.Type.Fuse:
                        console.debug("TODO: Fuse clicked");
                        break;
                    default:
                        console.error("Undefined option clicked -> " + option);
                        return;
                }
                configurationStack.lastTypeSelected = type;
            }
        }

    } //StackView -> configurationStack

} // Item
