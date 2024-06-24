import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import onboard 1.0

FooterButtonsPage {
    id: root

    property alias localFolderChooser: localFolder

    footerButtons.rightPrimary {
        text: OnboardingStrings.sync
        icons.source: Images.syncIcon
    }

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 32

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: OnboardingStrings.fullSync
            description: OnboardingStrings.fullSyncDescription
        }

        InfoAccount {
            id: accountData

            Layout.topMargin: 16
            Layout.preferredWidth: parent.width
        }

        ChooseSyncFolder {
            id: localFolder

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4            
        }
    }
}
