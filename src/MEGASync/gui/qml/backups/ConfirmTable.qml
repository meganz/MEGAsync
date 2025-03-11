import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.busyIndicator 1.0

import BackupCandidatesProxyModel 1.0

Rectangle {
    id: root

    readonly property int headerMargin: 24
    readonly property int headerHeight: 40
    readonly property int tableRadius: 8

    Layout.preferredWidth: width
    Layout.preferredHeight: height
    width: 400
    height: 184
    radius: tableRadius
    color: ColorTheme.pageBackground

    Rectangle {
        id: borderRectangle

        width: root.width
        height: root.height
        color: "transparent"
        border.color: ColorTheme.borderStrong
        border.width: 1
        radius: 8
        z: 5
    }

    ListView {
        id: listView

        anchors.fill: parent
        model: backupCandidatesProxyModelAccess
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: folderComponent
        header: headerComponent
        ScrollBar.vertical: ScrollBar {}
    }

    Component {
        id: headerComponent

        Rectangle {
            id: headerRectangle

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.headerHeight
            color: ColorTheme.pageBackground

            radius: root.radius
            z: 3

            RowLayout {
                id: headerLayout

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                spacing: 0

                RowLayout {
                    id: imageTextLayout

                    Layout.leftMargin: root.headerMargin
                    Layout.fillWidth: true
                    spacing: root.headerMargin / 2

                    SvgImage {
                        id: headerImage

                        source: Images.database
                        color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    Texts.Text {
                        id: headerText

                        text: BackupsStrings.backupFolders
                        font.weight: Font.DemiBold
                        color: ColorTheme.textPrimary
                    }
                }

                Texts.Text {
                    id: totalSizeText

                    Layout.rightMargin: root.headerMargin
                    Layout.alignment: Qt.AlignRight
                    text: backupCandidatesAccess.totalSize
                    color: ColorTheme.textPrimary
                    visible: backupCandidatesAccess.totalSizeReady
                    font {
                        pixelSize: Texts.Text.Size.SMALL
                        weight: Font.DemiBold
                    }
                }

                BusyIndicator {
                    id: busyIndicatorItem

                    Layout.rightMargin: root.headerMargin
                    Layout.alignment: Qt.AlignRight
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    imageSize: Qt.size(16, 16)
                    visible: !backupCandidatesAccess.totalSizeReady
                    color: ColorTheme.textAccent
                }
            }

            Rectangle {
                id: lineRectangle

                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: borderRectangle.border.width
                color: ColorTheme.borderSubtle

            }

            MouseArea {
                id: headerMouseArea

                anchors.fill: parent
                hoverEnabled: true
            }

        } // Rectangle: headerRectangle

    } // Component: headerComponent

    Component {
        id: folderComponent

        FolderRow {
            id: folderRow
        }
    }
}
