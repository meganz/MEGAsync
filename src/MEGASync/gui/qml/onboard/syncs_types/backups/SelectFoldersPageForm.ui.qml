import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import backups 1.0

import onboard 1.0

import BackupCandidatesProxyModel 1.0

FooterButtonsPage {
    id: root

    footerButtons.rightPrimary {
        text: BackupsStrings.backUp
        icons.source: Images.database
        enabled: backupCandidatesAccess.checkAllState !== Qt.Unchecked
    }

    ColumnLayout {
        id: selectFolderLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: Constants.defaultComponentSpacing

        HeaderTexts {
            id: headerItem

            title: OnboardingStrings.selectBackupFoldersTitle
            description: BackupsStrings.selectBackupFoldersDescription
        }

        InfoAccount {
            id: infoAccount

            Layout.preferredWidth: parent.width
        }
    }

    SelectTable {
        id: backupsTable

        anchors {
            top: selectFolderLayout.bottom
            left: parent.left
            right: parent.right
            bottom: footerButtons.top
            topMargin: 8
            bottomMargin: selectFolderLayout.spacing
        }
    }
}
