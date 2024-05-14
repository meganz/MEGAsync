import common 1.0

import components.pages 1.0

import BackupsProxyModel 1.0
import BackupsModel 1.0

FooterButtonsPage {
    id: root

    required property BackupsProxyModel backupsProxyModelRef

    readonly property int spacing: 24

    footerButtons {
        leftPrimary.visible: false
        leftSecondary {
            text: Strings.setExclusions
            visible: true
            enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                        || backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
        }
        rightTertiary.visible: true
        rightPrimary {
            text: BackupsStrings.backUp
            icons.source: Images.database
            enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                        || backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
        }
    }

    ConfirmFoldersContent {
        id: contentItem

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        backupsProxyModelRef: root.backupsProxyModelRef
    }

}
