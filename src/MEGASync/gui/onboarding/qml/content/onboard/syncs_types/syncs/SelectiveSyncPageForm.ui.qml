// System
import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import Onboard 1.0

SyncsPage {

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder

    footerButtons.rightPrimary {
        enabled: localFolder.isValid && remoteFolder.isValid
        text: OnboardingStrings.sync
        icons.source: Images.syncIcon
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }
        spacing: 24

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.selectiveSync
            description: OnboardingStrings.selectiveSyncDescription
        }

        InfoAccount {
            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
        }

        ChooseSyncFolder {
            id: localFolder

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            Layout.topMargin: 28
        }

        ChooseSyncFolder {
            id: remoteFolder

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            local: false
        }
    }
}
