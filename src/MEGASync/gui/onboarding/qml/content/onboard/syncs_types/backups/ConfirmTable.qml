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
import BackupsProxyModel 1.0
import BackupsModel 1.0

Rectangle {
    id: tableRectangle

    readonly property int headerFooterMargin: 24
    readonly property int headerFooterHeight: 40
    readonly property int tableRadius: 8

    Layout.preferredWidth: parent.width
    Layout.preferredHeight: height
    height: BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.None
            ? 192 : 192 - headerFooterHeight + listView.footerItem.height
    width: parent.width
    radius: tableRadius
    color: "white"

    Rectangle {
        id: borderRectangle

        width: tableRectangle.width
        height: tableRectangle.height
        color: "transparent"
        border.color: tableRectangle.enabled ? Styles.borderStrong : Styles.borderDisabled
        border.width: 1
        radius: 8
        z: 5
    }

    ListView {
        id: listView

        model: backupsProxyModel
        anchors.fill: parent
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: folderComponent
        header: headerComponent
        footerPositioning: ListView.OverlayFooter
        footer: footerComponent
    }

    Component {
        id: headerComponent

        Rectangle {
            height: headerFooterHeight
            anchors.left: parent.left
            anchors.right: parent.right
            color: "white"
            radius: tableRectangle.radius
            z: 3

            RowLayout {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                RowLayout {
                    Layout.leftMargin: headerFooterMargin
                    Layout.fillWidth: true
                    spacing: headerFooterMargin / 2

                    MegaImages.SvgImage {
                        source: Images.database
                        color: tableRectangle.enabled ? Styles.iconPrimary : Styles.iconDisabled
                        sourceSize: Qt.size(16, 16)
                    }

                    MegaTexts.Text {
                        text: OnboardingStrings.backupFolders
                        font.weight: Font.DemiBold
                    }
                }

                MegaTexts.Text {
                    Layout.rightMargin: headerFooterMargin
                    Layout.alignment: Qt.AlignRight
                    text: BackupsModel.mTotalSize
                    font.pixelSize: MegaTexts.Text.Size.Small
                    font.weight: Font.DemiBold
                }
            }

            Rectangle {
                height: borderRectangle.border.width
                color: tableRectangle.enabled ? Styles.borderSubtle : Styles.borderDisabled
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
    }

    Component {
        id: folderComponent

        FolderRow {
            anchors.right: parent.right
            anchors.left: parent.left
        }
    }

    Component {
        id: footerComponent

        Rectangle {
            id: notificationFooter

            anchors.left: parent.left
            anchors.right: parent.right
            height: notificationItem.height
            radius: tableRadius
            color: "white"
            z: 3
            visible: BackupsModel.mGlobalError !== BackupsModel.BackupErrorCode.None

            MegaTexts.NotificationText {
                id: notificationItem

                width: parent.width
                attributes.type: BackupsModel.mGlobalError === BackupsModel.BackupErrorCode.SDKCreation
                                 ? MegaTexts.NotificationInfo.Type.Error
                                 : MegaTexts.NotificationInfo.Type.Warning
                attributes.icon.source: ""
                attributes.icon.visible: false
                attributes.radius: parent.radius
                attributes.topBorderRect: true
                text: BackupsModel.mConflictsNotificationText
                visible: parent.visible
            }
        }
    }

}
