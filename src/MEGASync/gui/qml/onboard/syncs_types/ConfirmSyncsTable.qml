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
import components.toolTips 1.0

Rectangle {
    id: root

    readonly property int headerMargin: 24
    readonly property int headerHeight: 40
    readonly property int headerRadius: 8
    readonly property int headerSpacing: 8
    readonly property int footerHeight: 40
    readonly property int footerRadius: 8
    readonly property int syncCandidateHeight: 24
    readonly property int syncCandidateRadius: 4
    readonly property int syncCandidateSpacing: 8
    readonly property int separatorHeight: 1
    readonly property int verticalMargin: 8
    readonly property int horitzontalMargin: 7
    readonly property int confirmTableHeight: 232
    readonly property int confirmTableRadius: 8
    readonly property int mainBorderZ: 1
    readonly property int leftMarginAddSync: 16
    readonly property int menuLeftMargin: 8
    readonly property int separatorMargins: 4
    readonly property int menuWidth: 179
    readonly property int leftPaddingRemoteSyncCandidateRow: headerMargin - horitzontalMargin

    signal moveBack

    width: parent.width
    height: confirmTableHeight
    radius: confirmTableRadius
    color: ColorTheme.pageBackground

    Rectangle {
        id: borderRectangle

        anchors.fill: parent
        color: "transparent"
        border.color: ColorTheme.borderStrong
        radius: confirmTableRadius
        z: mainBorderZ
    }

    ColumnLayout {
        anchors.fill: parent
        spacing: 0

        Rectangle {
            id: header

            Layout.fillWidth: true
            height: headerHeight
            color: ColorTheme.pageBackground
            radius: headerRadius

            Row {
                id: headerLayout

                anchors.verticalCenter: parent.verticalCenter
                width: parent.width
                spacing: 0

                Row {
                    id: localImageTextLayout

                    leftPadding: headerMargin
                    width: parent.width / 2
                    spacing: headerSpacing

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

                    leftPadding: leftPaddingRemoteSyncCandidateRow
                    width: parent.width / 2
                    spacing: headerSpacing

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
        }

        Rectangle {
            id: underHeaderLine

            height: separatorHeight
            color: ColorTheme.borderSubtle
            Layout.fillWidth: true
            Layout.leftMargin: horitzontalMargin
            Layout.rightMargin: horitzontalMargin
        }

        Rectangle {
            id: overListVerticalSpacer

            height: verticalMargin
        }

        ListView {
            id: listView

            Layout.fillHeight: true
            Layout.fillWidth: true
            model: syncsCandidatesModel
            focus: true
            clip: true
            delegate: delegateComponent
            spacing: verticalMargin

            ScrollBar.vertical: ScrollBar {
                    policy: ScrollBar.AsNeeded
                }
        }

        Rectangle {
            id: underListVerticalSpacer

            height: verticalMargin
        }

        Rectangle {
            id: overFooterLine

            height: separatorHeight
            color: ColorTheme.borderSubtle
            Layout.fillWidth: true
            Layout.leftMargin: horitzontalMargin
            Layout.rightMargin: horitzontalMargin
        }

        Rectangle {
            id: footer

            width: parent.width
            height: footerHeight
            color: ColorTheme.pageBackground

            radius: footerRadius

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
                    leftMargin: leftMarginAddSync
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
        }
    }

    Component {
        id: delegateComponent

        Rectangle {
            id: groundDelegate

            width: listView.width
            height: syncCandidateHeight

            Rectangle {
                id: syncRow

                anchors {
                    left: parent.left
                    right: parent.right

                    leftMargin: horitzontalMargin
                    rightMargin: horitzontalMargin
                }

                height: parent.height
                color: ColorTheme.surface1
                radius: syncCandidateRadius

                Row {
                    id: syncRowLayout

                    anchors.fill: parent
                    spacing: 0

                    RowLayout {
                        id: localFolderImageTextLayout

                        height: parent.height
                        width: parent.width / 2
                        spacing: 0

                        Rectangle {
                            color: ColorTheme.surface1
                            width: headerMargin - horitzontalMargin
                            height: parent.height
                        }

                        RowLayout {
                            id: localFolderImageText

                            height: parent.height
                            Layout.fillWidth: true
                            spacing: syncCandidateSpacing

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
                                Layout.fillWidth: true

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

                        Rectangle {
                            color: ColorTheme.surface1
                            width: syncCandidateSpacing
                            height: parent.height
                        }

                        SvgImage {
                            id: syncImage

                            source: Images.syncsConfirm
                            sourceSize: Qt.size(12, 12)
                        }
                    }

                    RowLayout {
                        id: remoteFolderImageTextLayout

                        height: parent.height
                        width: parent.width / 2
                        spacing: 0

                        Rectangle {
                            color: ColorTheme.surface1
                            width: leftPaddingRemoteSyncCandidateRow
                            height: parent.height
                        }

                        RowLayout {
                            id: remoteFolderImageText

                            height: parent.height
                            Layout.fillWidth: true
                            spacing: syncCandidateSpacing

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
                                Layout.fillWidth: true

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

                        Rectangle {
                            color: ColorTheme.surface1
                            width: syncCandidateSpacing
                            height: parent.height
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

                                width: menuWidth

                                onFocusChanged: {
                                    if (menu.activeFocus === false){
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
                                    topPadding: separatorMargins
                                    bottomPadding: separatorMargins

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

                                        titleText: qsTr("Remove sync?")
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

                        Rectangle {
                            color: ColorTheme.surface1
                            width: menuLeftMargin
                            height: parent.height
                        }
                    }
                }
            }
        }
    }

}

