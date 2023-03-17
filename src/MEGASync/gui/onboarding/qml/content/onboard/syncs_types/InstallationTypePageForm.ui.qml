import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

Component {

    ColumnLayout {
        spacing: 12

        Header {
            title: qsTr("Choose how you want to use MEGA")
            description: qsTr("Choose a installation type")
            Layout.fillWidth: false
            Layout.preferredWidth: 488
            Layout.topMargin: 32
            Layout.leftMargin: 32
        }

        InstallationType {
            id: installationTypeContent

            Layout.fillWidth: false
            Layout.preferredWidth: 488
            Layout.leftMargin: 32
        }

        Connections {
            target: installationTypeContent

            onOptionSelected: (option) => {
                console.log("Selected option -> " + option);
            }
        }
    }
}
