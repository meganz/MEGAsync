import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.textFields 1.0

import BackupCandidates 1.0
import BackupCandidatesProxyModel 1.0
import QmlDeviceName 1.0

Column {
    id: root

    spacing: notificationItem.visible ? 8 : (24 - Constants.focusBorderWidth)

    ConfirmTable {
        id: confirmFoldersTable

        anchors {
            left: parent.left
            right: parent.right
        }
    }

    Texts.BannerText {
        id: notificationItem

        anchors {
            left: parent.left
            right: parent.right
        }
        showBorder: true
        type: backupCandidatesAccess.globalError === BackupCandidates.SDK_CREATION
                ? Constants.MessageType.ERROR
                : Constants.MessageType.WARNING
        text: backupCandidatesAccess.conflictsNotificationText
        visible: backupCandidatesAccess.globalError !== BackupCandidates.NONE
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
