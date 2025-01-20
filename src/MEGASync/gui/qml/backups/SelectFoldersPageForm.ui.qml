import QtQuick 2.15

import common 1.0

import components.pages 1.0
import components.texts 1.0 as Texts

import BackupCandidatesProxyModel 1.0

FooterButtonsPage {
    id: root

    readonly property int spacing: 24

    footerButtons {
        leftPrimary.visible: false
        rightSecondary.text: Strings.cancel
        rightPrimary {
            text: Strings.next
            icons.source: Images.arrowRight
            enabled: backupCandidatesAccess.checkAllState !== Qt.Unchecked
        }
    }

    Texts.SecondaryText {
        id: descriptionItem

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        font.pixelSize: Texts.Text.Size.MEDIUM
        wrapMode: Text.WordWrap
        text: BackupsStrings.selectBackupFoldersDescription
    }

    SelectTable {
        id: backupsTable

        anchors {
            top: descriptionItem.bottom
            left: parent.left
            right: parent.right
            bottom: footerButtons.top
            topMargin: root.spacing
            bottomMargin: root.spacing
        }
    }

}
