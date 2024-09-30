import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import Syncs 1.0

FooterButtonsPage {
    id: root

    required property bool isOnboarding

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder
    property alias syncs: syncsItem

    footerButtons.rightPrimary {
        text: SyncsStrings.sync
        icons.source: Images.syncIcon
    }

    Syncs {
        id: syncsItem
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }
        spacing: 24

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: SyncsStrings.selectiveSync
            description: SyncsStrings.selectiveSyncDescription
        }

        InfoAccount {
            id: accountData

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
        }

        ChooseSyncFolder {
            id: localFolder

            syncs: syncsItem
            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            Layout.topMargin: 16
            isOnboarding: root.isOnboarding
        }

        ChooseSyncFolder {
            id: remoteFolder

            syncs: syncsItem
            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            local: false
            isOnboarding: root.isOnboarding
        }
    }

}
