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
import components.toolTips 1.0

Rectangle {
    id: root

    readonly property int headerMargin: 24
    readonly property int headerHeight: 42
    readonly property int tableRadius: 8
    readonly property int verticalMargin: 8
    readonly property int horitzontalMargin: 7

    signal moveBack

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

    Column {
        anchors.fill: parent

        Rectangle {
            id: header

            width: listView.width
            height: root.headerHeight + root.verticalMargin
            color: ColorTheme.pageBackground
            radius: root.radius
            z: 10

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

                        text: qsTr("Local folders")
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

                        text: qsTr("MEGA folders")
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
                    leftMargin: root.horitzontalMargin
                    rightMargin: root.horitzontalMargin
                }
                height: borderRectangle.border.width
                color: ColorTheme.borderSubtle
            }
        }

        ListView {
            id: listView

            anchors.right: parent.right
            anchors.left: parent.left
            height: parent.height - header.height - footer.height

            model: syncsCandidatesModel
            focus: true
            clip: true
            delegate: delegateComponent
            headerPositioning: ListView.OverlayHeader
            spacing: root.verticalMargin

            ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                    z: 1
                }
        }

        Rectangle {
            id: footer

            width: listView.width
            height: root.headerHeight
            color: ColorTheme.pageBackground

            radius: root.radius
            z: 10

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

                HandleSyncCandidatesDialog {
                    id: handleSyncCandidate

                    visible: false
                }

                onClicked: {
                    handleSyncCandidate.visible = true;
                }
            }

            Rectangle {
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
    }

    Component {
        id: delegateComponent

        Rectangle {
            id: groundDelegate

            height: 24
            width: listView.width

            Rectangle {
                id: syncRow

                anchors {
                    left: parent.left
                    right: parent.right

                    leftMargin: root.horitzontalMargin
                    rightMargin: root.horitzontalMargin
                }

                height: parent.height
                color: ColorTheme.surface1
                radius: 4
                z: 5

                Row {
                    id: syncRowLayout

                    anchors.verticalCenter: parent.verticalCenter
                    width: parent.width
                    spacing: 0

                    Row {
                        id: localFolderImageTextLayout

                        width: parent.width / 2
                        spacing: root.headerMargin / 2

                        Row {
                            id: localFolderImageText

                            width: parent.width - syncImage.width
                            spacing: root.headerMargin / 2
                            leftPadding: root.headerMargin - root.horitzontalMargin

                            SvgImage {
                                id: localFolderImage

                                source: Images.localSyncFolder
                                sourceSize: Qt.size(16, 16)
                            }

                            Texts.Text {
                                id: localFolderText

                                text: model.localFolder
                                font.weight: Font.DemiBold
                                color: ColorTheme.textPrimary
                                elide: Text.ElideMiddle
                                wrapMode: Text.NoWrap
                                width: parent.width - localFolderImage.width - parent.spacing - parent.leftPadding

                                SmallToolTip {
                                    id: localTooltip
                                }

                                TextMetrics {
                                    id: localTextMetrics

                                    font: localFolderText.font
                                    text: localFolderText.text
                                }

                                MouseArea {
                                    enabled: localTextMetrics.width > localFolderText.width
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: {
                                        localTooltip.text = localFolderText.text;
                                        localTooltip.open();
                                    }

                                    onExited: {
                                        localTooltip.close();
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

                                text: model.megaFolder
                                font.weight: Font.DemiBold
                                color: ColorTheme.textPrimary
                                elide: Text.ElideMiddle
                                wrapMode: Text.NoWrap
                                width: parent.width - remoteFolderImage.width - parent.spacing

                                TextMetrics {
                                    id: remoteTextMetrics

                                    font: remoteFolderText.font
                                    text: remoteFolderText.text
                                }

                                SmallToolTip {
                                    id: remoteTooltip
                                }

                                MouseArea {
                                    enabled: remoteTextMetrics.width > remoteFolderText.width
                                    anchors.fill: parent
                                    hoverEnabled: true
                                    onEntered: {
                                        remoteTooltip.text = remoteFolderText.text;
                                        remoteTooltip.open();
                                    }

                                    onExited: {
                                        remoteTooltip.close();
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
                                    menu.popup(mouseX, mouseY);
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
                                        editSyncCandidateDialog.visible = true;
                                    }

                                    HandleSyncCandidatesDialog {
                                        id: editSyncCandidateDialog

                                        title: qsTr("Edit sync")
                                        rightPrimaryButton.text: qsTr("Save")
                                        editLocalPath: model.localFolder
                                        editRemotePath: model.megaFolder
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
                                        removeSyncDialog.modelCount = listView.count;
                                        removeSyncDialog.visible = true;
                                    }

                                    RemoveSyncCandidateConfirmationDialog {
                                        id: removeSyncDialog

                                        titleText: qsTr("Remove sync")
                                        bodyText: modelCount === 1 ? qsTr("Removing this sync will take you back to the start of setup.") : qsTr("Are you sure you want to remove the selected sync?")
                                        cancelButtonText: qsTr("Cancel")
                                        acceptButtonText: qsTr("Remove")
                                        visible: false

                                        onAccepted: {
                                            syncsComponentAccess.removeSyncCandidadeButtonClicked(model.localFolder, model.megaFolder);

                                            if (modelCount === 1) {
                                                root.moveBack();
                                            }
                                        }
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

}

