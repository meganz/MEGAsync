import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {
    id: mainItem

    objectName: "Backups"
    substackView.initialItem: selectFolders

    SelectFoldersPageForm {
        id: selectFolders

        objectName: mainItem.objectName
        next: confirmFolders
        footerLayout: mainItem.footerLayout
    }

    ConfirmFoldersPageForm {
        id: confirmFolders

        previous: selectFolders
        footerLayout: mainItem.footerLayout
        visible: false
    }
}
