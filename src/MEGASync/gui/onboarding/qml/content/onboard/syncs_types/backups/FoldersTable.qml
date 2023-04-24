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
import BackupFolderModel 1.0
import BackupFolderFilterProxyModel 1.0

Rectangle {
    id: tableRectangle

    property BackupFolderFilterProxyModel backupProxyModel
    property BackupFolderModel backupModel

    height: 186
    radius: 8

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
                    visible: !backupProxyModel.selectedFilterEnabled

                    onCheckedChanged: {
                        checkSelectAll(checked);
                    }
                }

                Text {
                    id: headerText

                    property int selectedRows: 0

                    text: backupProxyModel.selectedFilterEnabled
                          ? OnboardingStrings.backupFolders
                          : "(" + selectedRows + ")"
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

                    onVisibleChanged: {
                        if(visible) {
                            totalSizeText.text = backupModel.getTotalSize();
                        }
                    }
                }

                Connections {
                    target: backupModel

                    onRowSelectedChanged: (selectedRow, selectedAll) => {
                        if(selectedRow) {
                            selectAll.indeterminate = true;
                            headerText.selectedRows = backupModel.getNumSelectedRows();
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
                Layout.preferredWidth: parent.width - 32
                Layout.leftMargin: 16
                radius: 8

                color: (index % 2 === 0) ? "transparent" : Styles.surface2

                RowLayout {
                    anchors.verticalCenter: parent.verticalCenter
                    anchors.fill: parent

                    RowLayout {
                        Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                        Custom.CheckBox {
                            id: rowCheckbox

                            Layout.leftMargin: 8
                            Layout.preferredWidth: 16
                            Layout.preferredHeight: 16
                            enabled: selectable
                            checkable: selectable
                            checked: selected
                        }

                        Custom.SvgImage {
                            Layout.leftMargin: 18
                            source: error ? Images.alertTriangle : Images.folder
                            sourceSize: Qt.size(14, 14)
                            color: error ? Styles.textWarning : color
                            opacity: selectable ? 1.0 : 0.3
                        }

                        Text {
                            Layout.leftMargin: 13
                            text: display
                            font.family: "Inter"
                            font.styleName: "normal"
                            font.weight: Font.Normal
                            font.pixelSize: 12
                            color: selectable ? Styles.textPrimary : Styles.textDisabled
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
                        color: selectable ? Styles.textPrimary : Styles.textDisabled
                    }
                }

                MouseArea {
                    id: folderRowArea

                    anchors.fill: folderRowItem
                    hoverEnabled: true
                    cursorShape: !selectable ? Qt.ArrowCursor : Qt.PointingHandCursor
                    onClicked: {
                        if(selectable) {
                            backupList.currentIndex = index;
                            selected = !selected;
                        }
                    }
                }

                Custom.ToolTip {
                    visible: folderRowArea.containsMouse
                    leftIcon.source: Images.pc
                    text: toolTip
                    delay: 500
                    timeout: 5000
                }
            }
        }

    }

}
