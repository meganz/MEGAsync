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
            configurationStack.currentItem.updateFooter();
        } else {
            configurationStack.currentItem.nextPage();
        }
    }

    function previousPage() {
        if(!configurationStack.currentItem.isStacked
                || configurationStack.currentItem.isFirstPage) {

            configurationStack.replace(configurationStack.currentItem.previous,
                                       StackView.Immediate);

            if(configurationStack.currentItem == installationTypePage) {
                installationTypePage.content.clear();
            }

            configurationStack.currentItem.updateFooter();

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
    property var footerLayout: Footer {}

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

        property bool isOptionSelected: false
        property int lastTypeSelected: -1

        width: parent.width
        initialItem: computerNamePage

        ComputerNamePageForm {
            id: computerNamePage

            next: installationTypePage
            footerLayout: mainItem.footerLayout
            footerState: Footer.ToStates.CancelNext
        }

        InstallationTypePageForm {
            id: installationTypePage

            previous: computerNamePage
            footerLayout: mainItem.footerLayout
            footerState: Footer.ToStates.CancelPreviousNextDisabled
            visible: false
        }

        BackupPage {
            id: backupPage

            previous: installationTypePage
            footerLayout: mainItem.footerLayout
            footerState: Footer.ToStates.CancelPreviousNextDisabled
            isStacked: true
            visible: false
        }

        Connections {
            target: installationTypePage.content

            onOptionChanged: (type, checked) => {
                configurationStack.isOptionSelected = checked;
                if(checked) {
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
                    syncsFooter.show(Footer.ToStates.CancelPreviousNextEnabled);
                } else {
                    syncsFooter.show(Footer.ToStates.CancelPreviousNextDisabled);
                }
            }
        }

    } //StackView -> configurationStack

} // Item
