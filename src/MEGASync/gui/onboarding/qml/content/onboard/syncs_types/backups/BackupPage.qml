import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {
    id: mainItem

    stackView.initialItem: selectFolders
    isFirstPage: true

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
