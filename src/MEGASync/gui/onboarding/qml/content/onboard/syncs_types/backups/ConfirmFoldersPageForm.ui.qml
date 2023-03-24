import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {

    objectName: "ConfirmFoldersPageForm"

    ColumnLayout {
        spacing: 24
        width: 488

        Header {
            title: qsTr("Confirm folders to back up")
            description: qsTr("Will be added to your cloud and available in your other devices.")
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.topMargin: 32
            Layout.leftMargin: 32
        }

        ConfirmFolders {
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.leftMargin: 32
        }
    }
}
