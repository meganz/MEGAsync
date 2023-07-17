// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

// C++
import BackupsModel 1.0
import ChooseLocalFolder 1.0

Rectangle {
    visible: addFoldersButton.visible || notification.visible
    height: backupsProxyModel.selectedFilterEnabled ? notification.height : headerFooterHeight
    radius: tableRectangle.radius
    color: "white"
    z: 3

    RowLayout {
        id: addFoldersButton

        anchors.left: parent.left
        anchors.top: parent.top
        anchors.bottom: parent.bottom
        anchors.leftMargin: headerFooterMargin
        spacing: headerFooterMargin / 2
        visible: !backupsProxyModel.selectedFilterEnabled

        MegaImages.SvgImage {
            source: Images.plus
            color: Styles.buttonPrimary
            sourceSize: Qt.size(16, 16)
        }

        MegaTexts.Text {
            text: OnboardingStrings.addFolders
            font.weight: Font.DemiBold
        }
    }

    MouseArea {
        anchors.fill: addFoldersButton
        cursorShape: Qt.PointingHandCursor
        enabled: addFoldersButton.visible
        z: 3
        onClicked: {
            folderDialog.openFolderSelector();
        }
    }

    ChooseLocalFolder {
        id: folderDialog

        onFolderChanged: {
            BackupsModel.insert(folderDialog.getFolder());
        }
    }

    MegaTexts.NotificationText {
        id: notification

        width: parent.width
        attributes.type: MegaTexts.NotificationInfo.Type.Warning
        attributes.icon.source: ""
        attributes.icon.visible: false
        attributes.radius: parent.radius
        attributes.topBorderRect: true
        text: BackupsModel.mConflictsNotificationText
        visible: backupsProxyModel.selectedFilterEnabled
                 && (BackupsModel.mConflictsNotificationText !== "")
    }

    Rectangle {
        height: borderRectangle.border.width
        color: borderRectangle.border.color
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        visible: addFoldersButton.visible
    }

    MouseArea {
        anchors.fill: parent
        cursorShape: Qt.ArrowCursor
    }
}
