// System
import QtQuick 2.12
import QtQuick.Layouts 1.12
import QtQuick.Controls 2.12
import QtQuick.Dialogs 1.1

// QML common
import Common 1.0
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

// C++
import BackupsProxyModel 1.0
import BackupsModel 1.0
import ChooseLocalFolder 1.0

Rectangle {
    id: tableRectangle

    property var model
    property int headerFooterMargin: 24
    property int headerFooterHeight: 40

    height: 186
    radius: 8

    color: "white"

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
        delegate: folderComponent
        header: headerComponent
        footerPositioning: ListView.OverlayFooter
        footer: footerComponent
    }

    Component {
        id: headerComponent

        Rectangle {
            height: headerFooterHeight
            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            color: "white"
            radius: tableRectangle.radius
            z: 3

            RowLayout {
                width: parent.width
                anchors.verticalCenter: parent.verticalCenter
                spacing: 0

                MegaCheckBoxes.CheckBox {
                    id: selectAll

                    property bool fromModel: false

                    implicitHeight: parent.height
                    Layout.leftMargin: headerFooterMargin
                    text: OnboardingStrings.selectAll
                    tristate: true
                    visible: !backupsProxyModel.selectedFilterEnabled

                    onCheckStateChanged: {
                        if (!selectAll.fromModel) {
                            BackupsModel.mCheckAllState = checkState;
                        }
                        selectAll.fromModel = false;
                    }

                    Connections {
                        target: BackupsModel

                        onCheckAllStateChanged: {
                            selectAll.fromModel = true;
                            selectAll.checkState = BackupsModel.mCheckAllState;
                        }
                    }

                    Component.onCompleted: {
                        selectAll.checkState = BackupsModel.mCheckAllState;
                    }
                }

                RowLayout {
                    Layout.leftMargin: headerFooterMargin
                    Layout.fillWidth: true
                    spacing: headerFooterMargin / 2
                    visible: backupsProxyModel.selectedFilterEnabled

                    MegaImages.SvgImage {
                        source: Images.database
                        color: Styles.iconPrimary
                        sourceSize: Qt.size(16, 16)
                    }

                    MegaTexts.Text {
                        text: OnboardingStrings.backupFolders
                    }
                }

                MegaTexts.Text {
                    Layout.rightMargin: headerFooterMargin
                    Layout.alignment: Qt.AlignRight
                    text: BackupsModel.mTotalSize
                    font.pixelSize: MegaTexts.Text.Size.Small
                    font.weight: Font.DemiBold
                    visible: backupsProxyModel.selectedFilterEnabled
                }
            }

            Rectangle {
                height: borderRectangle.border.width
                color: borderRectangle.border.color
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
        }
    }

    Component {
        id: footerComponent

        Rectangle {
            anchors.right: parent.right
            anchors.left: parent.left
            height: headerFooterHeight
            z: 3
            visible: !backupsProxyModel.selectedFilterEnabled
            color: "white"
            radius: tableRectangle.radius

            RowLayout {
                id: addFoldersButton

                anchors.left: parent.left
                anchors.top: parent.top
                anchors.bottom: parent.bottom
                anchors.leftMargin: headerFooterMargin
                spacing: headerFooterMargin / 2

                MegaImages.SvgImage {
                    source: Images.plus
                    color: Styles.buttonPrimary
                    sourceSize: Qt.size(16, 16)
                }

                MegaTexts.Text {
                    text: OnboardingStrings.addFolders
                    font.weight: Font.DemiBold
                }
            }

            MouseArea {
                anchors.fill: addFoldersButton
                cursorShape: Qt.PointingHandCursor
                z: 3
                onClicked: {
                    folderDialog.openFolderSelector();
                }
            }

            ChooseLocalFolder {
                id: folderDialog

                onFolderChanged: {
                    _cppBackupsModel.insertFolder(folderDialog.getFolder());
                }
            }

            Rectangle {
                height: borderRectangle.border.width
                color: borderRectangle.border.color
                anchors.top: parent.top
                anchors.left: parent.left
                anchors.right: parent.right
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.ArrowCursor
            }
        }

    }

}
