import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

SyncsPage {

    ColumnLayout {
        spacing: 34
        anchors.left: parent.left
        anchors.leftMargin: 32
        width: 488

        Header {
            title: qsTr("Selective sync")
            description: qsTr("Lorem ipsum dolor asitmet")
            Layout.fillWidth: false
            Layout.preferredWidth: 488
            Layout.topMargin: 32
        }

        InfoAccount {}

        ChooseSyncFolder {}

        ChooseSyncFolder {
            local: false
        }
    }

    footerButtons.nextButton.text: qsTr("Sync")
    footerButtons.nextButton.iconSource: "../../../images/sync.svg"
}
