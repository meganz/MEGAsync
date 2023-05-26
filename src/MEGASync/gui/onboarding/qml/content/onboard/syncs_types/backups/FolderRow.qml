// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components.Texts 1.0 as MegaTexts
import Components.Images 1.0 as MegaImages
import Components.CheckBoxes 1.0 as MegaCheckBoxes
import Components.ToolTips 1.0 as MegaToolTips

Rectangle {
    id: root

    readonly property int totalHeight: 32
    readonly property int horizontalMargin: 6
    readonly property int internalMargin: 8

    height: totalHeight
    anchors.right: parent.right
    anchors.left: parent.left
    anchors.rightMargin: horizontalMargin
    anchors.leftMargin: horizontalMargin

    Rectangle {
        id: folderRowItem

        anchors.right: root.right
        anchors.left: root.left
        anchors.top: root.top
        anchors.bottom: root.bottom
        anchors.rightMargin: internalMargin
        anchors.leftMargin: internalMargin
        radius: internalMargin
        color: (index % 2 === 0) ? "transparent" : Styles.surface2

        RowLayout {
            anchors.verticalCenter: parent.verticalCenter
            anchors.fill: parent

            RowLayout {
                Layout.alignment: Qt.AlignLeft | Qt.AlignVCenter

                MegaCheckBoxes.CheckBox {
                    Layout.leftMargin: 8
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    checked: selected
                    checkable: selectable
                    enabled: selectable
                }

                MegaImages.SvgImage {
                    Layout.leftMargin: 18
                    source: error ? Images.alertTriangle : Images.folder
                    sourceSize: Qt.size(14, 14)
                    color: error ? Styles.textWarning : color
                    opacity: selectable ? 1.0 : 0.3
                }

                MegaTexts.Text {
                    Layout.leftMargin: 13
                    Layout.maximumWidth: 345
                    maximumLineCount: 1
                    wrapMode: Text.WrapAnywhere
                    text: display
                }
            }

            MegaTexts.Text {
                Layout.alignment: Qt.AlignRight
                Layout.rightMargin: 8
                text: size
                font.pixelSize: MegaTexts.Text.Size.Small
            }
        }

        MouseArea {
            id: folderRowArea

            anchors.fill: folderRowItem
            hoverEnabled: true
            cursorShape: selectable ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: {
                if(selectable) {
                    selected = !selected;
                }
            }
        }

        MegaToolTips.ToolTip {
            visible: folderRowArea.containsMouse
            leftIconSource: (toolTip === folder) ? Images.pc : ""
            text: toolTip
            delay: 500
            timeout: 5000
        }
    }
}
