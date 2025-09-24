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
    readonly property int contentLeftMargin: 8
    readonly property int contentRighttMargin: 4

    color: "transparent"
    width:  transfersRect.width
    height: componentHeight

    property int completedTransfers: 0
    property int totalTransfers: 0
    property int ongoingTransfers: 0
    property bool isTopTransferUpload: true
    property bool paused: false
    property bool pauseEnabled: true
    property bool areThereTransfers : (completedTransfers !== 0 || totalTransfers !== 0)
    readonly property int margins: areThereTransfers? 8: 0;

    signal transferManagerClicked
    signal pauseResumeClicked

    Rectangle {
        id: transfersRect

        color: areThereTransfers? ColorTheme.surface3 : "transparent"
        radius: componentRadius

        width: layout.implicitWidth + root.contentLeftMargin + root.contentRighttMargin
        height: componentHeight

        MouseArea {
            anchors.fill: parent
            cursorShape: Qt.PointingHandCursor
            onClicked: {
                root.transferManagerClicked();
            }
        }

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
            anchors.leftMargin: 8
            anchors.rightMargin: 4
            spacing: 16

            RowLayout{

                spacing: 4
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
                    text: (ongoingTransfers + completedTransfers) + "/" + totalTransfers

                    MouseArea {
                        anchors.fill: parent
                        cursorShape: Qt.PointingHandCursor
                        onClicked: {
                            root.transferManagerClicked();
                        }
                    }
                }
            }

            SvgImage {
                id: pauseResumeButton

                Layout.alignment: Qt.AlignVCenter
                source: paused? Images.play_circle_medium_thin_outline : Images.pause_circle_medium_thin_outline
                sourceSize: Qt.size(root.buttonIconSize, root.buttonIconSize)
                color: ColorTheme.buttonOutline
                visible: areThereTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true
                    onClicked: {
                        root.pauseResumeClicked();
                    }
                    onPressed: {
                        pauseResumeButton.color = ColorTheme.buttonOutlinePressed
                    }

                    onReleased: {
                        // When released, check if mouse is still hovering
                        if (containsMouse) {
                            pauseResumeButton.color = ColorTheme.buttonOutlineHover
                        } else {
                            pauseResumeButton.color = ColorTheme.buttonOutline
                        }
                    }

                    onEntered: {
                        // Only change to hover color if not currently pressed
                        if (!pressed) {
                            pauseResumeButton.color = ColorTheme.buttonOutlineHover
                        }
                    }

                    onExited: {
                        // Only change to normal if not currently pressed
                        if (!pressed) {
                            pauseResumeButton.color = ColorTheme.buttonOutline
                        }
                    }
                }
            }

            SvgImage {
                id: transferManagerButton

                Layout.alignment: Qt.AlignVCenter
                source: Images.arrowUpDown
                sourceSize: Qt.size(root.buttonIconSize, root.buttonIconSize)
                color: ColorTheme.buttonOutline
                visible: !areThereTransfers

                MouseArea {
                    anchors.fill: parent
                    cursorShape: Qt.PointingHandCursor
                    hoverEnabled: true

                    onClicked: {
                        root.transferManagerClicked();
                    }
                    onPressed: {
                        transferManagerButton.color = ColorTheme.buttonOutlinePressed
                    }

                    onReleased: {
                        // When released, check if mouse is still hovering
                        if (containsMouse) {
                            transferManagerButton.color = ColorTheme.buttonOutlineHover
                        } else {
                            transferManagerButton.color = ColorTheme.buttonOutline
                        }
                    }

                    onEntered: {
                        // Only change to hover color if not currently pressed
                        if (!pressed) {
                            transferManagerButton.color = ColorTheme.buttonOutlineHover
                        }
                    }

                    onExited: {
                        // Only change to normal if not currently pressed
                        if (!pressed) {
                            transferManagerButton.color = ColorTheme.buttonOutline
                        }
                    }
                }
            }
        }


    }
}
