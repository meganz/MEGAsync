import QtQuick 2.15

import common 1.0

import components.textFields 1.0
import components.pages 1.0

import BackupsProxyModel 1.0
import BackupsModel 1.0
import QmlDeviceName 1.0

FooterButtonsPage {
    id: root

    required property BackupsProxyModel backupsProxyModelRef

    readonly property int spacing: 24

    footerButtons {
        leftPrimary.visible: false
        leftSecondary {
            text: Strings.setExclusions
            visible: true
        }
        rightTertiary.visible: true
        rightPrimary {
            text: BackupsStrings.backUp
            icons.source: Images.database
            enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                        || backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
        }
    }

    ConfirmTable {
        id: confirmFoldersTable

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
            bottom: deviceField.top
            topMargin: root.spacing
            bottomMargin: root.spacing
        }
        backupsProxyModelRef: root.backupsProxyModelRef
    }

    TextField {
        id: deviceField

        anchors {
            bottom: footerButtons.top
            left: parent.left
            right: parent.right
            bottomMargin: root.spacing - footerButtons.rightPrimary.sizes.focusBorderWidth
            leftMargin: -deviceField.sizes.focusBorderWidth
            rightMargin: -deviceField.sizes.focusBorderWidth
        }
        colors.text: colorStyle.textPlaceholder
        title: BackupsStrings.backupTo
        leftIconSource: Images.database
        textField.readOnly: true
        textField.text: "/" + deviceName.name

        QmlDeviceName {
            id: deviceName
        }
    }

}
