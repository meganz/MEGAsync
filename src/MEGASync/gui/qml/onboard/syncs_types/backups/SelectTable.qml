import QtQuick 2.15
import QtQuick.Layouts 1.15
import QtQuick.Controls 2.15

import common 1.0

import components.checkBoxes 1.0
import components.texts 1.0 as Texts
import components.images 1.0
import components.buttons 1.0

import onboard 1.0

import BackupsModel 1.0
import ChooseLocalFolder 1.0

Item {
    id: root

    readonly property int headerFooterMargin: 24
    readonly property int headerFooterHeight: 40
    readonly property int tableRadius: 8

    Layout.preferredWidth: parent.width
    Layout.preferredHeight: height
    height: 210
    width: parent.width

    Rectangle {
        id: borderRectangle

        anchors.fill: parent
        color: "transparent"
        border.color: Styles.borderStrong
        border.width: 1
        radius: tableRadius
        z: 2
    }

    Rectangle {
        id: backgroundRectangle

        anchors.fill: parent
        color: Styles.pageBackground
        radius: tableRadius
    }

    ListView {
        id: backupsListView

        model: backupsModelAccess
        anchors.fill: parent
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: folderComponent
        header: headerComponent
        footerPositioning: ListView.OverlayFooter
        footer: footerComponent
        ScrollBar.vertical: ScrollBar {}
    }

    Connections {
        id: backupsModelAccessConnection

        target: backupsModelAccess

        function onNewFolderAdded(newFolderIndex) {
            backupsListView.positionViewAtIndex(newFolderIndex, ListView.Center)
        }
    }

    Component {
        id: headerComponent

        Rectangle {
            height: headerFooterHeight
            anchors.left: parent.left
            anchors.right: parent.right
            color: Styles.pageBackground
            radius: tableRadius
            z: 2

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
            }

            RowLayout {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                CheckBox {
                    id: selectAll

                    property bool fromModel: false

                    implicitHeight: parent.height
                    Layout.leftMargin: headerFooterMargin
                    text: OnboardingStrings.selectAll
                    tristate: true
                    sizes.spacing: 8
                    enabled: backupsListView.count > 0

                    onCheckStateChanged: {
                        if (!selectAll.fromModel) {
                            backupsModelAccess.checkAllState = checkState;
                        }
                        selectAll.fromModel = false;
                    }

                    Connections {
                        target: backupsModelAccess

                        function onCheckAllStateChanged() {
                            selectAll.fromModel = true;
                            selectAll.checkState = backupsModelAccess.checkAllState;
                        }
                    }

                    Component.onCompleted: {
                        selectAll.checkState = backupsModelAccess.checkAllState;
                    }
                }
            }

            Rectangle {
                height: borderRectangle.border.width
                color: Styles.borderSubtle
                anchors.bottom: parent.bottom
                anchors.left: parent.left
                anchors.right: parent.right
            }
        }
    }

    Component {
        id: folderComponent

        FolderRow {
            anchors.right: parent.right
            anchors.left: parent.left

            onFocusActivated: {
                backupsListView.positionViewAtIndex(index, ListView.Center)
            }
        }
    }

    Component {
        id: footerComponent

        Rectangle {
            anchors.left: parent.left
            anchors.right: parent.right
            height: headerFooterHeight
            radius: tableRadius
            color: Styles.pageBackground
            z: 2

            TextButton {
                id: addFoldersButton

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: 20
                text: OnboardingStrings.addFolder
                sizes: SmallSizes { borderLess: true }
                icons {
                    source: Images.plus
                    position: Icon.Position.LEFT
                }
                onClicked: {
                    folderDialog.openFolderSelector();
                }
            }

            Rectangle {
                height: borderRectangle.border.width
                color: borderRectangle.border.color
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
            }

            ChooseLocalFolder {
                id: folderDialog
            }

            Connections {
                id: chooseLocalFolderConnection

                target: folderDialog

                function onFolderChoosen(folderPath) {
                    backupsModelAccess.insert(folderPath);
                }
            }
        }
    }

}
