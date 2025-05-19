import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.busyIndicator 1.0
import components.buttons 1.0

Rectangle {
    id: root

    readonly property int headerMargin: 24
    readonly property int headerHeight: 42
    readonly property int tableRadius: 8
    readonly property int verticalMargin: 8
    readonly property int horitzontalMargin: 7

    Layout.preferredWidth: width
    Layout.preferredHeight: height
    width: 420
    height: 232
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
        model: 1
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: delegateComponent
        header: headerComponent
        ScrollBar.vertical: ScrollBar {}
        spacing: root.verticalMargin
        footer: footerComponent
        footerPositioning: ListView.OverlayFooter
    }

    Component {
        id: headerComponent

        Rectangle {
            id: headerRectangle

            anchors {
                left: parent.left
                right: parent.right
                leftMargin: root.horitzontalMargin
                rightMargin: root.horitzontalMargin
            }

            height: root.headerHeight + root.verticalMargin
            color: ColorTheme.pageBackground

            radius: root.radius
            z: 3

            Row {
                id: headerLayout

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                spacing: 0

                Row {
                    id: localImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2
                    spacing: root.headerMargin / 2

                    SvgImage {
                        id: localImage

                        source: Images.localFolderHeader
                        color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    Texts.Text {
                        id: localText

                        text: "Local folders"
                        font.weight: Font.DemiBold
                        color: ColorTheme.textPrimary
                    }
                }

                Row {
                    id: remoteImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2
                    spacing: root.headerMargin / 2

                    SvgImage {
                        id: remoteImage

                        source: Images.remoteMegaHeader
                        color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    Texts.Text {
                        id: remoteText

                        text: "MEGA folders"
                        font.weight: Font.DemiBold
                        color: ColorTheme.textPrimary
                    }
                }
            }

            Rectangle {
                id: lineRectangle

                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                    bottomMargin: root.verticalMargin
                }
                height: borderRectangle.border.width
                color: ColorTheme.borderSubtle
            }
        } // Rectangle: headerRectangle

    } // Component: headerComponent

    Component {
        id: delegateComponent

        Rectangle {
            id: syncRow

            anchors {
                left: parent.left
                right: parent.right
                leftMargin: root.horitzontalMargin
                rightMargin: root.horitzontalMargin
            }

            height: 24
            color: ColorTheme.surface1

            radius: 4
            z: 3

            Row {
                id: syncRowLayout

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                spacing: 0

                Row {
                    id: localFolderImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2

                    Row {
                        id: localFolderImageText

                        width: parent.width - syncImage.width - parent.leftPadding
                        spacing: root.headerMargin / 2

                        SvgImage {
                            id: localFolderImage

                            source: Images.folderOpen
                            //color: ColorTheme.iconPrimary
                            sourceSize: Qt.size(16, 16)
                        }

                        Texts.Text {
                            id: localFolderText

                            text: syncsDataAccess.localFolderCandidate
                            font.weight: Font.DemiBold
                            color: ColorTheme.textPrimary
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                            width: parent.width - localFolderImage.width - parent.spacing
                        }
                    }

                    SvgImage {
                        id: syncImage

                        source: Images.syncsConfirm
                        //color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }
                }

                Row {
                    id: remoteFolderImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2

                    Row {
                        id: remoteFolderImageText

                        width: parent.width - menuImage.width - parent.leftPadding - root.headerMargin / 2
                        spacing: root.headerMargin / 2

                        SvgImage {
                            id: remoteFolderImage

                            source: Images.remoteSyncFolder
                            //color: ColorTheme.iconPrimary
                            sourceSize: Qt.size(16, 16)
                        }

                        Texts.Text {
                            id: remoteFolderText

                            text: syncsDataAccess.remoteFolderCandidate
                            font.weight: Font.DemiBold
                            color: ColorTheme.textPrimary
                            elide: Text.ElideRight
                            wrapMode: Text.NoWrap
                            width: parent.width - remoteFolderImage.width - parent.spacing
                        }
                    }

                    SvgImage {
                        id: menuImage

                        source: Images.menuSync
                        //color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }
                }
            }
        }

    } // Component: delegateComponent

    Component {
        id: footerComponent

        Rectangle {
            id: footerItem

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.headerHeight
            color: ColorTheme.pageBackground

            radius: root.radius
            z: 3

            MouseArea {
                anchors.fill: parent
                hoverEnabled: false
                cursorShape: Qt.ArrowCursor
                onClicked: {
                    mouse.accepted = false;
                }
            }

            TextButton {
                id: addSyncButton

                anchors {
                    left: parent.left
                    verticalCenter: parent.verticalCenter
                    leftMargin: 20
                }
                text: qsTr("Add more syncs")
                sizes: SmallSizes { borderLess: true }
                icons {
                    source: Images.plus
                    position: Icon.Position.LEFT
                }

                onClicked: {
                    console.log("open add new sync dialog...")
                }
            }

            Rectangle {
                id: lineRectangle

                anchors {
                    top: parent.top
                    left: parent.left
                    right: parent.right
                    leftMargin: root.horitzontalMargin
                    rightMargin: root.horitzontalMargin
                }
                height: borderRectangle.border.width
                color: ColorTheme.borderSubtle
            }
        }
    } // Component: footerComponent
}

