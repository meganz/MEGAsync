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
            id: localFolderChooser

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            Layout.topMargin: 28
        }

        ChooseSyncFolder {
            id: remoteFolderChooser

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            local: false
        }
    }
}
