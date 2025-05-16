import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.busyIndicator 1.0

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
        model: 1
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: delegateComponent
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

                        source: Images.monitor
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

                        source: Images.megaOutline
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
        id: delegateComponent

        Rectangle {
            id: syncRow

            anchors {
                left: parent.left
                right: parent.right
            }

            height: root.headerHeight
            color: ColorTheme.pageBackground

            radius: root.radius
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
                    spacing: root.headerMargin / 2

                    SvgImage {
                        id: localFolderImage

                        source: Images.folderOpen
                        //color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    Texts.Text {
                        id: localFolderText

                        text: "Documents/MEGA"
                        font.weight: Font.DemiBold
                        color: ColorTheme.textPrimary
                    }
                }

                Row {
                    id: remoteFolderImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2
                    spacing: root.headerMargin / 2

                    SvgImage {
                        id: remoteFolderImage

                        source: Images.folderOpen
                        //color: ColorTheme.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    Texts.Text {
                        id: remoteFolderText

                        text: "/MEGA"
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
                }
                height: borderRectangle.border.width
                color: ColorTheme.borderSubtle
            }

            MouseArea {
                id: headerMouseArea

                anchors.fill: parent
                hoverEnabled: true
            }

        } // Rectangle: syncRow

    } // Component: delegateComponent
}
