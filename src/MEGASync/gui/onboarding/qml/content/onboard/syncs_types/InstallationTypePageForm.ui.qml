import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.0

SyncsPage {

    objectName: "InstallationTypePageForm"

    ColumnLayout {
        spacing: 12
        width: 488

        Header {
            title: qsTr("Choose how you want to use MEGA")
            description: qsTr("Choose a installation type")
            Layout.preferredWidth: parent.width
            Layout.topMargin: 32
            Layout.leftMargin: 32
        }

        InstallationType {
            id: content

            Layout.preferredWidth: parent.width
            Layout.leftMargin: 32
            Layout.topMargin: 24
        }
    }

    Connections {
        target: footerLayout

        onNextButtonClicked: {
            if(visible) {
                switch(content.getTypeSelected()) {
                    case InstallationTypeButton.Type.Sync:
                        console.debug("TODO: Sync clicked");
                        next = syncPage;
                        break;
                    case InstallationTypeButton.Type.Backup:
                        console.debug("Backup clicked");
                        next = backupPage;
                        break;
                    case InstallationTypeButton.Type.Fuse:
                        console.debug("TODO: Fuse clicked");
                        break;
                    default:
                        console.error("Undefined option clicked -> " + option);
                        return;
                }
            }
        }
    }

}
