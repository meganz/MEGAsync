import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Dialogs 1.1

import Common 1.0
import Components 1.0 as Custom
import Onboard.Syncs_types 1.0
import BackupFolderModel 1.0

SelectFoldersPageForm {

    backupList {
        header: tableHeader
        model: backupModel
        delegate: folderItem
    }

    addFoldersMouseArea.onClicked: {
        folderDialog.visible = true;
    }

    Component {
        id: tableHeader

        RowLayout {

            height: 36
            width: parent.width
            z: 3

            Rectangle {
                id: tableHeaderBackground

                height: parent.height
                width: tableRectangle.width - 2 * tableRectangle.border.width
                Layout.topMargin: tableRectangle.border.width
                Layout.leftMargin: tableRectangle.border.width
                color: Styles.surface2
                radius: tableRectangle.radius

                RowLayout {
                    anchors.verticalCenter: tableHeaderBackground.verticalCenter
                    spacing: 0

                    Custom.CheckBox {
                        id: selectAll

                        property bool fromModel: false

                        implicitHeight: parent.height
                        Layout.leftMargin: 22
                        text: qsTr("[b]Select all[/b]");
                        indeterminate: false

                        onCheckedChanged: {
                            if(selectAll.fromModel) {
                                selectAll.indeterminate = false;
                                selectAll.fromModel = false;
                            } else {
                                if(selectAll.checked && selectAll.indeterminate) {
                                    selectAll.checked = false;
                                }
                                selectAll.indeterminate = false;
                                backupModel.setAllSelected(checked);
                            }
                        }

                        Connections {
                            target: backupModel

                            onRowSelectedChanged: {
                                selectAll.indeterminate = true;
                                footerButtons.nextButton.enabled = true;
                            }

                            onAllRowsSelected: (selected) => {
                                selectAll.fromModel = true;
                                selectAll.checked = selected;
                                footerButtons.nextButton.enabled = selected;
                            }
                        }
                    }

                    Text {
                        id: selectText

                        text: "(3)"
                        font.pixelSize: 12
                        Layout.leftMargin: 4
                    }

                }

                Rectangle {
                    height: tableHeaderBackground.radius
                    color: tableHeaderBackground.color
                    anchors.bottom: tableHeaderBackground.bottom
                    anchors.left: tableHeaderBackground.left
                    anchors.right: tableHeaderBackground.right
                }
            }
        }

    }

    Component {
        id: folderItem

        RowLayout {

            height: 32
            width: parent.width

            Rectangle {
                id: folderRowItem

                Layout.preferredHeight: parent.height
                Layout.preferredWidth: 456
                Layout.leftMargin: 16
                radius: 8

                color: (index % 2 === 0) ? "transparent" : Styles.surface2

                RowLayout {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.fill: parent

                    RowLayout {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        Custom.CheckBox {
                            Layout.leftMargin: 8
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            checked: selected
                        }

                        Custom.SvgImage {
                            Layout.leftMargin: 18
                            source: "../../../../../../images/Onboarding/folder.svg"
                        }

                        Text {
                            Layout.leftMargin: 13
                            text: {
                                return folder.substring(folder.lastIndexOf('/') + 1);
                            }
                            font.family: "Inter"
                            font.styleName: "normal"
                            font.weight: Font.Normal
                            font.pixelSize: 12
                        }
                    }

                    Text {
                        Layout.alignment: Qt.AlignRight
                        Layout.rightMargin: 8
                        text: size
                        font.family: "Inter"
                        font.styleName: "normal"
                        font.weight: Font.Normal
                        font.pixelSize: 10
                    }
                }

                MouseArea {
                    anchors.fill: folderRowItem
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        backupList.currentIndex = index;
                        selected = !selected;
                    }
                }
            }
        }

    }

    BackupFolderModel {
        id: backupModel
    }

    FileDialog {
        id: folderDialog

        title: "";
        folder: shortcuts.home;
        selectFolder: true
        onAccepted: {
            var processedFolder = folder.toString().substring(8);
            backupModel.insertFolder(processedFolder);
        }
    }

}
