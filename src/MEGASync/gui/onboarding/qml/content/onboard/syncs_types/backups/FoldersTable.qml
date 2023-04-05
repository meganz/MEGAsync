import QtQuick 2.12
import QtQuick.Layouts 1.12

import Common 1.0
import Components 1.0 as Custom
import BackupFolderModel 1.0
import BackupFolderFilterProxyModel 1.0

Rectangle {
    id: tableRectangle

    property BackupFolderFilterProxyModel backupProxyModel
    property BackupFolderModel backupModel

    Layout.preferredWidth: 488
    Layout.preferredHeight: 186
    Layout.topMargin: 8
    radius: 8

    Rectangle {
        id: borderRectangle

        width: tableRectangle.width
        height: tableRectangle.height
        color: "transparent"
        border.color: Styles.borderStrong
        border.width: 2
        radius: 8
        z: 5
    }

    ListView {
        id: backupList

        model: backupProxyModel
        anchors.fill: parent
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: folderItem
        header: tableHeader
    }

    Component {
        id: tableHeader

        Rectangle {
            id: tableHeaderBackground

            height: 36
            width: tableRectangle.width - 2 * tableRectangle.border.width
            Layout.topMargin: tableRectangle.border.width
            Layout.leftMargin: tableRectangle.border.width
            color: Styles.surface2
            radius: tableRectangle.radius
            z: 3

            function checkSelectAll(checked) {
                if(selectAll.fromModel && selectAll.indeterminate) {
                    selectAll.indeterminate = false;
                    selectAll.checked = checked;
                } else {
                    if(selectAll.indeterminate && checked) {
                        checked = false;
                        selectAll.checked = false;
                    }
                    selectAll.indeterminate = false;
                    backupModel.setAllSelected(checked);
                }

                headerText.selectedRows = backupModel.getNumSelectedRows();
                totalSizeText.text = backupModel.getTotalSize();
                footerButtons.nextButton.enabled = checked;
                selectAll.fromModel = false;
            }

            RowLayout {
                width: tableHeaderBackground.width
                anchors.verticalCenter: tableHeaderBackground.verticalCenter
                spacing: 0

                Custom.CheckBox {
                    id: selectAll

                    property bool fromModel: false

                    implicitHeight: parent.height
                    Layout.leftMargin: 22
                    text: qsTr("[b]Select all[/b]");
                    indeterminate: false
                    visible: !backupProxyModel.selectedFilterEnabled

                    onCheckedChanged: {
                        checkSelectAll(checked);
                    }
                }

                Text {
                    id: headerText

                    property int selectedRows: 0

                    text: backupProxyModel.selectedFilterEnabled ? qsTr("Backup Folders") : "(" + selectedRows + ")"
                    font.pixelSize: 12
                    Layout.leftMargin: backupProxyModel.selectedFilterEnabled ? 22 : 4
                    Layout.fillWidth: true
                }

                Text {
                    id: totalSizeText

                    text: "0 B"
                    Layout.rightMargin: 22
                    Layout.alignment: Qt.AlignRight
                    font.pixelSize: 10
                    font.weight: Font.DemiBold
                    visible: backupProxyModel.selectedFilterEnabled
                }

                Connections {
                    target: backupModel

                    onRowSelectedChanged: (selectedRow, selectedAll) => {
                        if(selectedRow) {
                            selectAll.indeterminate = true;
                            headerText.selectedRows = backupModel.getNumSelectedRows();
                            totalSizeText.text = backupModel.getTotalSize();
                            footerButtons.nextButton.enabled = true;
                        } else {
                            selectAll.fromModel = true;
                            checkSelectAll(selectedAll);
                        }
                    }
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
                            checkable: !backupProxyModel.selectedFilterEnabled
                            hoverEnabled: !backupProxyModel.selectedFilterEnabled
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
                    cursorShape: backupProxyModel.selectedFilterEnabled ? Qt.ArrowCursor : Qt.PointingHandCursor
                    onClicked: {
                        backupList.currentIndex = index;
                        selected = !selected;
                    }
                    enabled: !backupProxyModel.selectedFilterEnabled
                }
            }
        }

    }

}
