// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages

// Local
import Onboard 1.0

// C++
import BackupsModel 1.0
import ChooseLocalFolder 1.0

Rectangle {

    readonly property int headerFooterMargin: 24
    readonly property int headerFooterHeight: 40
    readonly property int tableRadius: 8

    Layout.preferredWidth: parent.width
    Layout.preferredHeight: height
    height: 224
    width: parent.width
    radius: tableRadius

    color: Styles.pageBackground

    Rectangle {
        id: borderRectangle

        width: parent.width
        height: parent.height
        color: "transparent"
        border.color: Styles.borderStrong
        border.width: 1
        radius: 8
        z: 5
    }

    ListView {
        id: backupsListView

        model: BackupsModel
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
            anchors.left: parent.left
            anchors.right: parent.right
            color: Styles.pageBackground
            radius: tableRadius
            z: 3

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
            }

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
                    sizes.spacing: 12
                    enabled: backupsListView.count > 0

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
            z: 3

            RowLayout {
                id: addFoldersButton

                anchors.left: parent.left
                anchors.verticalCenter: parent.verticalCenter
                anchors.leftMargin: headerFooterMargin
                spacing: headerFooterMargin / 2

                MegaImages.SvgImage {
                    source: Images.plus
                    color: Styles.buttonPrimary
                    sourceSize: Qt.size(16, 16)
                }

                MegaTexts.Text {
                    text: OnboardingStrings.addFolder
                    font.weight: Font.DemiBold
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
                    BackupsModel.insert(folderDialog.getFolder());
                }
            }

            MouseArea {
                anchors.fill: parent
                hoverEnabled: true
            }
        }
    }

}
