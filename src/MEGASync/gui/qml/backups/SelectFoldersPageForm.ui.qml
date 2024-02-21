import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.accountData 1.0
import components.pages 1.0

import BackupsModel 1.0

FooterButtonsPage {
    id: root

    required property bool isOnboardingRef

    footerButtons {
        leftIcon.visible: !root.isOnboardingRef
        leftSecondary.visible: root.isOnboardingRef
        rightSecondary.text: root.isOnboardingRef ? Strings.previous : Strings.cancel
        rightPrimary {
            text: root.isOnboardingRef ? BackupsStrings.backUp : Strings.next
            icons.source: root.isOnboardingRef ? Images.database : Images.arrowRight
            enabled: backupsModelAccess.checkAllState !== Qt.Unchecked
        }
    }

    ColumnLayout {
        id: selectFolderLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        HeaderTexts {
            id: headerItem

            title: BackupsStrings.selectBackupFoldersTitle
            titleVisible: root.isOnboardingRef
            description: BackupsStrings.selectBackupFoldersDescription
        }

        InfoAccount {
            id: infoAccount

            Layout.preferredWidth: parent.width
            visible: root.isOnboardingRef
        }
    }

    SelectTable {
        id: backupsTable

        anchors {
            top: selectFolderLayout.bottom
            left: parent.left
            right: parent.right
            bottom: footerButtons.top
            topMargin: root.isOnboardingRef ? 8 : selectFolderLayout.spacing
            bottomMargin: selectFolderLayout.spacing
        }
    }
}
