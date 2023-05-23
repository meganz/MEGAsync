// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components 1.0 as Custom

// Local
import Onboard 1.0

// C++
import BackupsProxyModel 1.0

Rectangle {
    id: tableRectangle

    property BackupsProxyModel backupsProxyModel

    height: 186
    radius: 8

    color: Styles.pageBackground

    Rectangle {
        id: borderRectangle

        width: tableRectangle.width
        height: tableRectangle.height
        color: "transparent"
        border.color: Styles.borderStrong
        border.width: 1
        radius: 8
        z: 5
    }

    ListView {
        id: backupList

        model: backupsProxyModel
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
            width: tableRectangle.width - tableRectangle.border.width
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
                    backupsProxyModel.setAllSelected(checked);
                }

                headerText.selectedRows = backupsProxyModel.getNumSelectedRows();
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
                    text: OnboardingStrings.selectAll
                    indeterminate: false
                    visible: !backupsProxyModel.selectedFilterEnabled

                    onCheckedChanged: {
                        checkSelectAll(checked);
                    }
                }

                Custom.Text {
                    id: headerText

                    property int selectedRows: 0

                    text: backupsProxyModel.selectedFilterEnabled
                          ? OnboardingStrings.backupFolders
                          : "(" + selectedRows + ")"
                    Layout.leftMargin: backupsProxyModel.selectedFilterEnabled ? 22 : 4
                    Layout.fillWidth: true
                }

                Custom.Text {
                    id: totalSizeText

                    text: backupsProxyModel.totalSize
                    Layout.rightMargin: 22
                    Layout.alignment: Qt.AlignRight
                    font.pixelSize: Custom.Text.Size.Small
                    font.weight: Font.DemiBold
                    visible: backupsProxyModel.selectedFilterEnabled
                }

                Connections {
                    target: backupsProxyModel

                    onRowSelectedChanged: (selectedRow, selectedAll) => {
                        if(selectedRow) {
                            selectAll.indeterminate = true;
                            headerText.selectedRows = backupsProxyModel.getNumSelectedRows();
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

        FolderRow {
            anchors.right: parent.right
            anchors.left: parent.left
        }
    }

}
