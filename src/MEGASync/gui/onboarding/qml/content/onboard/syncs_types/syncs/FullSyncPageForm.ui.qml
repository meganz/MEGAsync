// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0

// Local
import Onboard 1.0
import Onboard.Syncs_types 1.0

SyncsPage {

    property alias localFolderChooser: localFolderChooser

    footerButtons.rightPrimary {
        enabled: localFolderChooser.isValid
        text: OnboardingStrings.sync
        icons.source: Images.sync
    }

    ColumnLayout {

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 32

        Header {
            Layout.preferredWidth: parent.width
            title: OnboardingStrings.fullSync
            description: OnboardingStrings.fullSyncDescription
        }

        InfoAccount {
            Layout.preferredWidth: parent.width
        }

        ChooseSyncFolder {
            id: localFolderChooser

            Layout.preferredWidth: parent.width
            Layout.leftMargin: -4
        }
    }
}
