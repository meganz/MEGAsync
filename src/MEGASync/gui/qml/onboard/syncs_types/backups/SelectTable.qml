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
    width: parent.width

    Rectangle {
        id: backgroundRectangle

        anchors.fill: parent
        color: Styles.pageBackground
        radius: tableRadius
    }

    ListView {
        id: backupsListView

        anchors {
            fill: parent
            margins: borderRectangle.border.width
        }
        model: backupsModelAccess
        headerPositioning: ListView.OverlayHeader
        focus: true
        clip: true
        delegate: folderComponent
        header: headerComponent
        footerPositioning: ListView.OverlayFooter
        footer: fakeFooterComponent
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
            id: headerRectangle

            anchors {
                left: parent.left
                right: parent.right
            }
            height: headerFooterHeight
            color: Styles.pageBackground
            radius: tableRadius
            z: 2

            MouseArea {
                id: headerMouseArea

                anchors.fill: parent
                hoverEnabled: true
            }

            RowLayout {
                id: checkboxLayout

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

                    Component.onCompleted: {
                        selectAll.checkState = backupsModelAccess.checkAllState;
                    }

                    Connections {
                        target: backupsModelAccess

                        function onCheckAllStateChanged() {
                            selectAll.fromModel = true;
                            selectAll.checkState = backupsModelAccess.checkAllState;
                        }
                    }
                }
            }

            Rectangle {
                id: bottomLine

                anchors {
                    bottom: parent.bottom
                    left: parent.left
                    right: parent.right
                }
                height: borderRectangle.border.width
                color: Styles.borderSubtle
            }

        } // Rectangle: headerRectangle

    } // Component: headerComponent

    Component {
        id: folderComponent

        FolderRow {
            id: folderItem

            onFocusActivated: {
                backupsListView.positionViewAtIndex(index, ListView.Center)
            }
        }
    }

    // This component must be used to reserve space for the footer in the ListView
    // but the real footer is defined in the footerItem. This is neccessary because
    // there is a bug in Qt that causes the footer is not visible if the ListView
    // is empty, the clip property is enabled and the footerPositioning is ListView.OverlayFooter
    // https://bugreports.qt.io/browse/QTBUG-85302
    Component {
        id: fakeFooterComponent

        Rectangle {
            anchors {
                left: parent.left
                right: parent.right
                bottom: borderRectangle.bottom
            }
            height: headerFooterHeight
            radius: tableRadius
            color: "transparent"
        }
    }

    Rectangle {
        id: footerItem

        anchors {
            left: parent.left
            right: parent.right
            bottom: borderRectangle.bottom
            bottomMargin: borderRectangle.border.width
            leftMargin: borderRectangle.border.width
            rightMargin: 10
        }
        height: headerFooterHeight - 2 * borderRectangle.border.width
        radius: tableRadius
        color: Styles.pageBackground
        z: 2

        MouseArea {
            anchors.fill: parent
            hoverEnabled: false
            cursorShape: Qt.ArrowCursor
            onClicked: {
                mouse.accepted = false;
            }
        }

        TextButton {
            id: addFoldersButton

            anchors {
                left: parent.left
                verticalCenter: parent.verticalCenter
                leftMargin: 20
            }
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

    } // Rectangle: footerItem


    Rectangle {
        id: topLine

        anchors {
            top: footerItem.top
            left: parent.left
            right: parent.right
        }
        height: borderRectangle.border.width
        color: borderRectangle.border.color
        z: 2
    }

    Rectangle {
        id: borderRectangle

        anchors.fill: parent
        color: "transparent"
        border.color: Styles.borderStrong
        border.width: 1
        radius: tableRadius
        z: 4
    }

}
