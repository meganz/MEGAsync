import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.textFields 1.0

import BackupsModel 1.0
import BackupsProxyModel 1.0
import QmlDeviceName 1.0

Column {
    id: root

    required property BackupsProxyModel backupsProxyModelRef

    spacing: notificationItem.visible ? 8 : (24 - Constants.focusBorderWidth)

    ConfirmTable {
        id: confirmFoldersTable

        anchors {
            left: parent.left
            right: parent.right
        }
        backupsProxyModelRef: root.backupsProxyModelRef
    }

    Texts.BannerText {
        id: notificationItem

        anchors {
            left: parent.left
            right: parent.right
        }
        showBorder: true
        type: backupsModelAccess.globalError === backupsModelAccess.BackupErrorCode.SDK_CREATION
                ? Constants.MessageType.ERROR
                : Constants.MessageType.WARNING
        text: backupsModelAccess.conflictsNotificationText
        visible: backupsModelAccess.globalError !== backupsModelAccess.BackupErrorCode.NONE
    }

    TextField {
        id: deviceField

        anchors {
            left: parent.left
            right: parent.right
            leftMargin: Constants.focusAdjustment
            rightMargin: Constants.focusAdjustment
        }
        colors.text: ColorTheme.textPlaceholder
        title: BackupsStrings.backupTo
        leftIconSource: Images.database
        textField.readOnly: true
        textField.text: "/" + deviceName.name

        QmlDeviceName {
            id: deviceName
        }
    }

}
