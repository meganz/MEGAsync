import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.images 1.0

Qml.ScrollBar {
    id: root

    enum Direction {
        NONE = 0,
        VERTICAL,
        HORIZONTAL
    }

    property int direction: ScrollBar.Direction.NONE
    property int scrollBarAnchor: 8
    property int scrollBarRadius: 10
    property int backgroundSpacing: 4
    property int backgroundMargin: 8
    property int iconSize: 16
    property int buttonFocusBorder: 2
    property int buttonFocusRadius: 6
    property int buttonSize: iconSize + buttonFocusBorder
    property int backgroundContentIconMargin: iconSize + backgroundSpacing
    property int backgroundContentMargin: scrollBarAnchor / 2 + buttonFocusBorder / 2
    property real scrollBarPressedOpacity: 0.6
    property real scrollBarBackgroundOpacity: 0.2
    property real backgroundContentHeight: root.height - 2 * backgroundSpacing - 2 * iconSize
    property real backgroundContentWidth: root.width - 2 * backgroundSpacing - 2 * iconSize

    height: direction === ScrollBar.Direction.VERTICAL ? parent.height : scrollBarAnchor
    width: direction === ScrollBar.Direction.VERTICAL ? scrollBarAnchor : parent.width
    visible: visualSize < 1.0

    contentItem: Loader { id: contentLoader }

    background: Loader { id: backgroundLoader }

    onDirectionChanged: {
        if(direction === ScrollBar.Direction.VERTICAL) {
            backgroundLoader.sourceComponent = verticalBackground;
            contentLoader.sourceComponent = verticalContent;
        }
        else if(direction === ScrollBar.Direction.HORIZONTAL) {
            backgroundLoader.sourceComponent = horizontalBackground;
            contentLoader.sourceComponent = horizontalContent;
        }
    }

    Keys.onUpPressed: {
        if(direction === ScrollBar.Direction.VERTICAL) {
            root.decrease();
        }
    }

    Keys.onDownPressed: {
        if(direction === ScrollBar.Direction.VERTICAL) {
            root.increase();
        }
    }

    Keys.onLeftPressed: {
        if(direction === ScrollBar.Direction.HORIZONTAL) {
            root.decrease();
        }
    }

    Keys.onRightPressed: {
        if(direction === ScrollBar.Direction.HORIZONTAL) {
            root.increase();
        }
    }

    Component {
        id: verticalContent

        Loader {
            anchors {
                left: parent.left
                top: parent.top
                bottom: parent.bottom
                leftMargin: backgroundContentMargin
                topMargin: backgroundContentIconMargin
                bottomMargin: backgroundContentIconMargin - 4
            }
            sourceComponent: contentComponent
        }
    }

    Component {
        id: horizontalContent

        Loader {
            anchors {
                left: parent.left
                top: parent.top
                right: parent.right
                topMargin: backgroundContentMargin
                leftMargin: backgroundContentIconMargin
                rightMargin: backgroundContentIconMargin
            }
            sourceComponent: contentComponent
        }
    }

    Component {
        id: contentComponent

        Rectangle {
            radius: scrollBarRadius
            color: Styles.iconPrimary
            opacity: root.pressed ? scrollBarPressedOpacity : 1.0
        }
    }

    Component {
        id: verticalBackground

        Column {
            id: verticalBackgroundColumn

            width: root.width
            height: root.height
            spacing: backgroundSpacing

            Loader {
                id: verticalDecreaseArrowLoader

                sourceComponent: decreaseArrow
            }

            Loader {
                id: verticalBackgroundLoader

                anchors {
                    left: parent.left
                    leftMargin: backgroundMargin
                }
                sourceComponent: backgroundRectangle
            }

            Loader {
                id: verticalIncreaseArrowLoader

                sourceComponent: increaseArrow
            }
        }
    }

    Component {
        id: horizontalBackground

        Row {
            id: verticalBackgroundRow

            width: root.width
            height: root.height
            spacing: backgroundSpacing

            Loader {
                id: horizontalDecreaseArrowLoader

                sourceComponent: decreaseArrow
            }

            Loader {
                id: horizontalBackgroundLoader

                anchors {
                    top: parent.top
                    topMargin: backgroundMargin
                }
                sourceComponent: backgroundRectangle
            }

            Loader {
                id: horizontalIncreaseArrowLoader

                sourceComponent: increaseArrow
            }
        }
    }

    Component {
        id: decreaseArrow

        Loader {
            id: decreaseArrowLoader

            property var buttonRotation: direction === ScrollBar.Direction.VERTICAL ? -90 : 180

            enabled: root.visualPosition !== 0
            sourceComponent: button

            Keys.onEnterPressed: {
                root.decrease();
            }

            Keys.onReturnPressed: {
                root.decrease();
            }

            MouseArea {
                id: decreaseArrowMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.decrease()
            }
        }
    }

    Component {
        id: backgroundRectangle

        Rectangle {
            id: backgroundPainted

            height: direction === ScrollBar.Direction.VERTICAL ? backgroundContentHeight : 2
            width: direction === ScrollBar.Direction.VERTICAL ? 2 : backgroundContentWidth
            radius: scrollBarRadius
            color: Styles.iconButton
            opacity: scrollBarBackgroundOpacity
        }

    }

    Component {
        id: increaseArrow

        Loader {
            id: increaseArrowLoader

            property var buttonRotation: direction === ScrollBar.Direction.VERTICAL ? 90 : 0

            enabled: visualPosition + visualSize < 1.0
            sourceComponent: button

            Keys.onEnterPressed: {
                root.increase();
            }

            Keys.onReturnPressed: {
                root.increase();
            }

            MouseArea {
                id: increaseArrowMouseArea

                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: root.increase()
            }
        }
    }

    Component {
        id: button

        Rectangle {
            id: buttonRect

            width: buttonSize
            height: buttonSize
            color: "transparent"
            radius: buttonFocusRadius
            activeFocusOnTab: true
            border {
                color: enabled && activeFocus ? Styles.focus : "transparent"
                width: buttonFocusBorder
            }

            SvgImage {
                id: image

                anchors {
                    left: parent.left
                    top: parent.top
                    topMargin: buttonFocusBorder / 2
                    leftMargin: buttonFocusBorder / 2
                }
                source: Images.arrowRight
                sourceSize: Qt.size(iconSize, iconSize)
                color: enabled ? Styles.iconButton : Styles.iconButtonDisabled
                rotation: buttonRotation
            }
        }
    }

}
