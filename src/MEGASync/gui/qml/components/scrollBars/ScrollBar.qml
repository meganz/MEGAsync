// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0
import Components.Images 1.0 as MegaImages

Qml.ScrollBar {
    id: scrollbar

    enum Direction {
        None = 0,
        Vertical = 1,
        Horizontal = 2
    }

    property int direction: ScrollBar.Direction.None

    property int scrollBarAnchor: 8
    property int scrollBarRadius: 10
    property real scrollBarPressedOpacity: 0.6
    property real scrollBarBackgroundOpacity: 0.2
    property real backgroundContentHeight: scrollbar.height - 2 * backgroundSpacing - 2 * iconSize
    property real backgroundContentWidth: scrollbar.width - 2 * backgroundSpacing - 2 * iconSize
    property int backgroundSpacing: 4
    property int backgroundMargin: 8
    property int iconSize: 16
    property int buttonFocusBorder: 2
    property int buttonFocusRadius: 6
    property int buttonSize: iconSize + buttonFocusBorder
    property int backgroundContentIconMargin: iconSize + backgroundSpacing
    property int backgroundContentMargin: scrollBarAnchor / 2 + buttonFocusBorder / 2

    height: direction === ScrollBar.Direction.Vertical ? parent.height : scrollBarAnchor
    width: direction === ScrollBar.Direction.Vertical ? scrollBarAnchor : parent.width
    visible: visualSize < 1.0

    contentItem: Loader { id: contentLoader }

    background: Loader { id: backgroundLoader }

    onDirectionChanged: {
        if(direction === ScrollBar.Direction.Vertical) {
            backgroundLoader.sourceComponent = verticalBackground;
            contentLoader.sourceComponent = verticalContent;
        } else if(direction === ScrollBar.Direction.Horizontal) {
            backgroundLoader.sourceComponent = horizontalBackground;
            contentLoader.sourceComponent = horizontalContent;
        }
    }

    Keys.onUpPressed: {
        if(direction === ScrollBar.Direction.Vertical) {
            scrollbar.decrease();
        }
    }

    Keys.onDownPressed: {
        if(direction === ScrollBar.Direction.Vertical) {
            scrollbar.increase();
        }
    }

    Keys.onLeftPressed: {
        if(direction === ScrollBar.Direction.Horizontal) {
            scrollbar.decrease();
        }
    }

    Keys.onRightPressed: {
        if(direction === ScrollBar.Direction.Horizontal) {
            scrollbar.increase();
        }
    }

    Component {
        id: verticalContent

        Loader {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: backgroundContentMargin
            anchors.topMargin: backgroundContentIconMargin
            anchors.bottomMargin: backgroundContentIconMargin - 4
            sourceComponent: contentComponent
        }
    }

    Component {
        id: horizontalContent

        Loader {
            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: backgroundContentMargin
            anchors.leftMargin: backgroundContentIconMargin
            anchors.rightMargin: backgroundContentIconMargin
            sourceComponent: contentComponent
        }
    }

    Component {
        id: contentComponent

        Rectangle {
            radius: scrollBarRadius
            color: Styles.iconPrimary
            opacity: scrollbar.pressed ? scrollBarPressedOpacity : 1.0
        }
    }

    Component {
        id: verticalBackground

        Column {
            spacing: backgroundSpacing
            width: scrollbar.width
            height: scrollbar.height

            Loader {
                sourceComponent: decreaseArrow
            }

            Loader {
                anchors.left: parent.left
                anchors.leftMargin: backgroundMargin
                sourceComponent: backgroundRectangle
            }

            Loader {
                sourceComponent: increaseArrow
            }
        }
    }

    Component {
        id: horizontalBackground

        Row {
            spacing: backgroundSpacing
            width: scrollbar.width
            height: scrollbar.height

            Loader {
                sourceComponent: decreaseArrow
            }

            Loader {
                anchors.top: parent.top
                anchors.topMargin: backgroundMargin
                sourceComponent: backgroundRectangle
            }

            Loader {
                sourceComponent: increaseArrow
            }
        }
    }

    Component {
        id: decreaseArrow

        Loader {
            property var buttonRotation: direction === ScrollBar.Direction.Vertical ? -90 : 180

            enabled: scrollbar.visualPosition !== 0
            sourceComponent: button

            Keys.onEnterPressed: {
                scrollbar.decrease();
            }

            Keys.onReturnPressed: {
                scrollbar.decrease();
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: scrollbar.decrease()
            }
        }
    }

    Component {
        id: backgroundRectangle

        Rectangle {
            id: backgroundPainted

            height: direction === ScrollBar.Direction.Vertical ? backgroundContentHeight : 2
            width: direction === ScrollBar.Direction.Vertical ? 2 : backgroundContentWidth
            radius: scrollBarRadius
            color: Styles.iconButton
            opacity: scrollBarBackgroundOpacity
        }

    }

    Component {
        id: increaseArrow

        Loader {
            property var buttonRotation: direction === ScrollBar.Direction.Vertical ? 90 : 0

            enabled: visualPosition + visualSize < 1.0
            sourceComponent: button

            Keys.onEnterPressed: {
                scrollbar.increase();
            }

            Keys.onReturnPressed: {
                scrollbar.increase();
            }

            MouseArea {
                anchors.fill: parent
                cursorShape: Qt.PointingHandCursor
                onClicked: scrollbar.increase()
            }
        }
    }

    Component {
        id: button

        Rectangle {
            border.color: enabled && activeFocus ? Styles.focus : "transparent"
            border.width: buttonFocusBorder
            color: "transparent"
            width: buttonSize
            height: buttonSize
            radius: buttonFocusRadius

            activeFocusOnTab: true

            MegaImages.SvgImage {
                source: Images.arrowRight
                sourceSize: Qt.size(iconSize, iconSize)
                color: enabled ? Styles.iconButton : Styles.iconButtonDisabled
                rotation: buttonRotation
                anchors.left: parent.left
                anchors.top: parent.top
                anchors.topMargin: buttonFocusBorder / 2
                anchors.leftMargin: buttonFocusBorder / 2
            }
        }
    }

}
