import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

FooterButtonsPage {
    id: root

    property alias localFolderChooser: localFolder
    property alias remoteFolderChooser: remoteFolder

    footerButtons.rightPrimary {
        text: SyncsStrings.sync
        icons.source: Images.syncIcon
    }

    footerButtons.rightSecondary.visible : syncsComponentAccess.comesFromOnboarding

    ColumnLayout {
        id: column

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            margins: 0
        }
        spacing: Constants.defaultComponentSpacing

        HeaderTexts {
            id: header

            Layout.preferredWidth: parent.width
            title: SyncsStrings.selectiveSyncTitle
            description: SyncsStrings.selectiveSyncDescription
        }

        InfoAccount {
            id: accountData

            Layout.preferredWidth: parent.width
            Layout.topMargin: 8
        }

        ChooseSyncLocalFolder {
            id: localFolder

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
            Layout.topMargin: 16
        }

        ChooseSyncRemoteFolder {
            id: remoteFolder

            Layout.preferredWidth: parent.width + 8
            Layout.leftMargin: -4
        }
    }

}
