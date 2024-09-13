import QtQuick.Layouts 1.15

import common 1.0

import components.pages 1.0

import backups 1.0

import onboard 1.0

import BackupsProxyModel 1.0
import BackupsModel 1.0

FooterButtonsPage {
    id: root

    required property BackupsProxyModel backupsProxyModelRef

    property alias enableConfirmHeader: confirmHeader.enabled

    readonly property int spacing: 24

    footerButtons.rightPrimary {
        text: BackupsStrings.backUp
        icons.source: Images.database
        enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                    || backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
    }

    ColumnLayout {
        id: externalLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: root.spacing

        HeaderTexts {
            id: confirmHeader

            title: OnboardingStrings.confirmBackupFoldersTitle
        }

        ConfirmFoldersContent {
            id: contentItem

            Layout.preferredWidth: parent.width

            backupsProxyModelRef: root.backupsProxyModelRef
        }
    }

}
