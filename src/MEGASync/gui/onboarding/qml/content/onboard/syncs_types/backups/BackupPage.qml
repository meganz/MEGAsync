import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {
    id: mainItem

    /*
     * Functions
     */

    function resetToInitialPage() {
        backupStack.replace(backupStack.initialItem, StackView.Immediate);
        backupStack.currentItem.updateFooter();

        isFirstPage = true;
    }

    function nextPage() {
        backupStack.replace(backupStack.currentItem.next, StackView.Immediate);
        backupStack.currentItem.updateFooter();

        isFirstPage = backupStack.currentItem == backupStack.initialItem;
    }

    function previousPage() {
        backupStack.replace(backupStack.currentItem.previous, StackView.Immediate);
        backupStack.currentItem.updateFooter();

        isFirstPage = backupStack.currentItem == backupStack.initialItem;
    }

    /*
     * Child components
     */

    StackView {
        id: backupStack

        initialItem: selectFolders

        SelectFoldersPageForm {
            id: selectFolders

            next: confirmFolders
            footerLayout: mainItem.footerLayout
            footerState: Footer.ToStates.CancelPreviousNextDisabled
        }

        ConfirmFoldersPageForm {
            id: confirmFolders

            previous: selectFolders
            footerLayout: mainItem.footerLayout
            footerState: Footer.ToStates.CancelPreviousNextBackup
            visible: false
        }
    }

    Component.onCompleted: {
        isFirstPage = true;
    }
}
