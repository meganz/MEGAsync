import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15
import QtQuick.Controls 2.12 as QmlControlsv212

import common 1.0

import components.texts 1.0 as Texts
import components.images 1.0
import components.busyIndicator 1.0
import components.buttons 1.0
import components.menus 1.0
import components.dialogs 1.0

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
        }

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

                            source: Images.localSyncFolder
                            sourceSize: Qt.size(16, 16)
                        }

                        Texts.Text {
                            id: localFolderText

                            text: syncsDataAccess.localFolderCandidate
                            font.weight: Font.DemiBold
                            color: ColorTheme.textPrimary
                            elide: Text.ElideMiddle
                            wrapMode: Text.NoWrap
                            width: parent.width - localFolderImage.width - parent.spacing

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: {
                                    ToolTip.show(localFolderText.text);
                                }

                                onExited: {
                                    ToolTip.hide();
                                }
                            }
                        }
                    }

                    SvgImage {
                        id: syncImage

                        source: Images.syncsConfirm
                        sourceSize: Qt.size(12, 12)
                        anchors.verticalCenter: parent.verticalCenter
                    }
                }

                Row {
                    id: remoteFolderImageTextLayout

                    leftPadding: root.headerMargin
                    width: parent.width / 2

                    Row {
                        id: remoteFolderImageText

                        width: parent.width - syncMenuButton.width - parent.leftPadding - root.headerMargin / 2
                        spacing: root.headerMargin / 2

                        SvgImage {
                            id: remoteFolderImage

                            source: Images.remoteSyncFolder
                            sourceSize: Qt.size(16, 16)
                        }

                        Texts.Text {
                            id: remoteFolderText

                            text: syncsDataAccess.remoteFolderCandidate
                            font.weight: Font.DemiBold
                            color: ColorTheme.textPrimary
                            elide: Text.ElideMiddle
                            wrapMode: Text.NoWrap
                            width: parent.width - remoteFolderImage.width - parent.spacing

                            MouseArea {
                                anchors.fill: parent
                                hoverEnabled: true
                                onEntered: {
                                    ToolTip.show(remoteFolderText.text);
                                }

                                onExited: {
                                    ToolTip.hide();
                                }
                            }
                        }
                    }

                    SvgImage {
                        id: syncMenuButton

                        source: Images.menuSync
                        sourceSize: Qt.size(16, 16)
                        color: ColorTheme.iconSecondary

                        MouseArea {
                            anchors.fill: parent
                            cursorShape: Qt.PointingHandCursor

                            onClicked: {
                                let buttonX = syncMenuButton.mapToItem(null, 0, 0).x;
                                let buttonY = syncMenuButton.mapToItem(null, 0, 0).y;
                                let menuHeight = menu.height;
                                let menuWidth = menu.width;
                                let xOffset = syncMenuButton.width/2;
                                let yOffset = syncMenuButton.height/2;
                                if (buttonX + menuWidth > window.width) {
                                    xOffset = xOffset - menuWidth; // Open left if close to right edge
                                }
                                if (buttonY + yOffset + menuHeight > window.height) {
                                    yOffset =  yOffset - menuHeight; // Open upwards if close to bottom edge
                                }
                                menu.popup(xOffset, yOffset);
                            }
                        }

                        ContextMenu {
                            id: menu

                            width: 200

                            onFocusChanged: {
                                if(menu.activeFocus === false){
                                    menu.close();
                                }
                            }

                            ContextMenuItem {
                                id: editLocalFolder

                                text: qsTr("Edit sync")
                                icon.source: Images.localFolderHeader
                                onTriggered: {
                                    console.log("edit sync : " + index)

                                    editSyncDialog.visible = true;
                                }

                                AddSyncDialog {
                                    id: editSyncDialog

                                    visible: false
                                }
                            }

                            QmlControlsv212.MenuSeparator {
                                padding: 0
                                topPadding: 4
                                bottomPadding: 4

                                contentItem: Rectangle {
                                    implicitWidth: parent.width
                                    implicitHeight: 1
                                    color: ColorTheme.surface2
                                }
                            }

                            ContextMenuItem {
                                id: removeSync

                                text: qsTr("Remove sync")
                                icon.source: Images.removeSync
                                onTriggered: {
                                    console.log("remove sync : "+ index);
                                    removeSyncDialog.visible = true;
                                }

                                ConfirmCloseDialog {
                                    id: removeSyncDialog

                                    titleText: qsTr("Remove sync")
                                    bodyText: qsTr("Are you sure you want to remove the selected sync?")
                                    cancelButtonText: qsTr("Cancel")
                                    acceptButtonText: qsTr("Remove")
                                    visible: false
                                    onAccepted: {
                                        console.log("sync : "+index+" removed!!")
                                    }
                                }
                            }
                        }
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

                AddSyncDialog {
                    id: addSyncDialog

                    visible: false
                }

                onClicked: {
                    addSyncDialog.visible = true;
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

