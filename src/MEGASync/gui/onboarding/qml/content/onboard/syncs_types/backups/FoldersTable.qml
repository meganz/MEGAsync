// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12

// QML common
import Common 1.0
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.Texts 1.0 as MegaTexts

// Local
import Onboard 1.0

// C++
import BackupsProxyModel 1.0

Rectangle {
    id: tableRectangle

    property var model

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

        model: tableRectangle.model
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

            RowLayout {
                width: tableHeaderBackground.width
                anchors.verticalCenter: tableHeaderBackground.verticalCenter
                spacing: 0

                MegaCheckBoxes.CheckBox {
                    id: selectAll

                    property bool fromModel: false

                    implicitHeight: parent.height
                    Layout.leftMargin: 22
                    text: OnboardingStrings.selectAll
                    tristate: true
                    visible: !backupsProxyModel.selectedFilterEnabled

                    onCheckStateChanged: {
                        if (!selectAll.fromModel) {
                            _cppBackupsModel.mCheckAllState = checkState;
                        }
                        selectAll.fromModel = false;
                    }

                    Connections {
                        target: _cppBackupsModel

                        onCheckAllStateChanged: {
                            selectAll.fromModel = true;
                            selectAll.checkState = _cppBackupsModel.mCheckAllState;
                        }
                    }

                    Component.onCompleted: {
                        selectAll.checkState = _cppBackupsModel.mCheckAllState;
                    }
                }

                MegaTexts.Text {
                    id: headerText

                    Layout.leftMargin: 22
                    Layout.fillWidth: true
                    text: OnboardingStrings.backupFolders
                    visible: backupsProxyModel.selectedFilterEnabled
                }

                MegaTexts.Text {
                    id: totalSizeText

                    Layout.rightMargin: 22
                    Layout.alignment: Qt.AlignRight
                    text: _cppBackupsModel.totalSize
                    font.pixelSize: MegaTexts.Text.Size.Small
                    font.weight: Font.DemiBold
                    visible: backupsProxyModel.selectedFilterEnabled
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
