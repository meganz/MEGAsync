import QtQuick 2.15
import QtQuick.Layouts 1.15
import common 1.0
import components.images 1.0
import components.texts 1.0 as Texts
import components.buttons 1.0 as MegaButtons

Rectangle {
    id: root

    readonly property int componentHeight: 32
    readonly property int componentRadius: 24
    readonly property int iconSize: 16
    readonly property int buttonIconSize: 24
    readonly property int margins: 8

    color: "transparent"
    width:  transfersRect.width
    height: componentHeight

    property int completedTransfers: 0
    property int totalTransfers: 0
    property bool isTopTransferUpload: true
    property bool paused: false
    property bool pauseEnabled: true
    property bool areThereTransfers : (completedTransfers !== 0 || totalTransfers !== 0)

    signal transferManagerClicked
    signal pauseResumeClicked

    Rectangle {
        id: transfersRect
        color: areThereTransfers? ColorTheme.surface3 : "transparent"
        radius: componentRadius
        width: layout.implicitWidth + root.margins*2
        height: componentHeight

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.transferManagerClicked();
            }
        }

        property bool expanded: false

        Behavior on width {
            NumberAnimation {
                duration: 1000
                easing.type: Easing.InOutBack
            }
        }

        RowLayout {
            id: layout
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.verticalCenter: parent.verticalCenter
            anchors.leftMargin: root.margins
            anchors.rightMargin: root.margins

            SvgImage {
                id: directionIcon
                Layout.alignment: Qt.AlignVCenter
                source: isTopTransferUpload? Images.upArrow : Images.downArrow
                sourceSize: Qt.size(root.iconSize, root.iconSize)
                color: ColorTheme.textPrimary
                visible: areThereTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.transferManagerClicked();
                    }
                }
            }

            Texts.Text {
                id: transferText
                Layout.alignment: Qt.AlignVCenter
                visible: areThereTransfers
                text: completedTransfers + "/" + totalTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.transferManagerClicked();
                    }
                }
            }

            SvgImage {
                id: pauseResumeButton
                Layout.alignment: Qt.AlignVCenter
                source: paused? Images.playCircle : Images.pauseCircle
                sourceSize: Qt.size(root.buttonIconSize, root.buttonIconSize)
                color: ColorTheme.textPrimary
                visible: areThereTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.pauseResumeClicked();
                    }
                }
            }

            SvgImage {
                id: transferManagerButton
                Layout.alignment: Qt.AlignVCenter
                source: Images.arrowUpDown
                sourceSize: Qt.size(root.buttonIconSize, root.buttonIconSize)
                color: ColorTheme.textPrimary
                visible: !areThereTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    onClicked: {
                        root.transferManagerClicked();
                    }
                }
            }
        }


    }
}
