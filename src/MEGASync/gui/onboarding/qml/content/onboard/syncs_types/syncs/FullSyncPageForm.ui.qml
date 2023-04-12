import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0
import Components 1.0 as Custom

SyncsPage {

    ColumnLayout {
        spacing: 34
        anchors.left: parent.left
        anchors.leftMargin: 32
        width: 488

        Header {
            title: qsTr("Full sync")
            description: qsTr("Your entire Cloud Drive will be synchronized with a local folder")
            Layout.fillWidth: false
            Layout.preferredWidth: 488
            Layout.topMargin: 32
        }

        InfoAccount {}

        ChooseSyncFolder {}
    }

    footerButtons.nextButton.text: qsTr("Sync")
    footerButtons.nextButton.iconSource: "../../../images/sync.svg"
}
