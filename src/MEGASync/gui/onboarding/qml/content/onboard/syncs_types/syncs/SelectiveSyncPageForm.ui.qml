// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard.Syncs_types 1.0
import Onboard 1.0

SyncsPage {

    property alias localFolderChooser: localFolderChooser
    property alias remoteFolderChooser: remoteFolderChooser

    footerButtons.rightPrimary {
        enabled: localFolderChooser.isValid && remoteFolderChooser.isValid
        text: OnboardingStrings.sync
        icons.source: Images.sync
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }
        spacing: contentMargin

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.selectiveSync
            description: OnboardingStrings.selectiveSyncDescription
        }

        InfoAccount {
            Layout.preferredWidth: parent.width
        }

        ChooseSyncFolder {
            id: localFolderChooser

            Layout.fillWidth: true
            Layout.leftMargin: -4
        }

        ChooseSyncFolder {
            id: remoteFolderChooser

            Layout.fillWidth: true
            Layout.leftMargin: -4
            local: false
        }
    }
}
