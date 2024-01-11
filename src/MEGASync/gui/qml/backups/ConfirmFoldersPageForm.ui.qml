import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.textFields 1.0
import components.pages 1.0

import BackupsModel 1.0
import QmlDeviceName 1.0

FooterButtonsPage {
    id: root

    property alias enableConfirmHeader: confirmHeader.enabled

    footerButtons.rightPrimary {
        text: BackupsStrings.backUp
        icons.source: Images.database
        enabled: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.NONE
                    || (backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
                            && backupsModelAccess.existsOnlyGlobalError)
    }

    ColumnLayout {
        id: externalLayout

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        spacing: 24

        HeaderTexts {
            id: confirmHeader

            title: BackupsStrings.confirmBackupFoldersTitle
        }

        ColumnLayout {
            id: mainLayout

            Layout.preferredWidth: parent.width
            spacing: 24

            ConfirmTable {
                id: confirmFoldersTable
            }

            TextField {
                id: deviceField

                colors.text: colorStyle.textPlaceholder
                Layout.preferredWidth: parent.width + 2 * deviceField.sizes.focusBorderWidth
                Layout.leftMargin: -deviceField.sizes.focusBorderWidth
                title: BackupsStrings.backupTo
                leftIconSource: Images.database
                textField.readOnly: true
                textField.text: "/" + deviceName.name
            }
        }
    }

    QmlDeviceName {
        id: deviceName
    }
}
