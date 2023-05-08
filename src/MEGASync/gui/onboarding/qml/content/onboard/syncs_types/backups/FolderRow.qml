// System
import QtQuick 2.12
import QtQuick.Layouts 1.12

// QML common
import Common 1.0
import Components 1.0 as Custom

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

                Custom.CheckBox {
                    Layout.leftMargin: 8
                    Layout.preferredWidth: 16
                    Layout.preferredHeight: 16
                    checked: selected
                    checkable: selectable
                    enabled: selectable
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
                    Layout.maximumWidth: 345
                    maximumLineCount: 1
                    wrapMode: Text.WrapAnywhere
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
            cursorShape: selectable ? Qt.PointingHandCursor : Qt.ArrowCursor
            onClicked: {
                if(selectable) {
                    selected = !selected;
                }
            }
        }

        Custom.ToolTip {
            visible: folderRowArea.containsMouse
            leftIconSource: (toolTip === folder) ? Images.pc : ""
            text: toolTip
            delay: 500
            timeout: 5000
        }
    }
}
