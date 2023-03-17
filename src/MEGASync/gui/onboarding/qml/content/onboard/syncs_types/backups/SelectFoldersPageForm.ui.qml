import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

import Onboard.Syncs_types 1.0

Component {

    ColumnLayout {
        spacing: 34
        width: 488

        Header {
            title: qsTr("Select folders to back up")
            description: qsTr("Selected folders from your computer to MEGA. Files will automatically back up when the desktop application is running.")
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.topMargin: 32
            Layout.leftMargin: 32
        }

        SelectFolders {
            Layout.fillWidth: false
            Layout.preferredWidth: parent.width
            Layout.leftMargin: 32
        }
    }
}
