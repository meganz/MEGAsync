// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0
import Components 1.0 as Custom

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
    property int backgroundMargin: 7
    property int iconSize: 16
    property int backgroundContentIconMargin: iconSize + backgroundSpacing - 2
    property int backgroundContentMargin: scrollBarAnchor / 2

    height: direction === ScrollBar.Direction.Vertical ? parent.height : scrollBarAnchor
    width: direction === ScrollBar.Direction.Vertical ? scrollBarAnchor : parent.width
    visible: visualSize < 1.0
    focusPolicy: Qt.WheelFocus

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

    Component {
        id: verticalContent

        Rectangle {
            id: content

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.bottom: parent.bottom
            anchors.leftMargin: backgroundContentMargin
            anchors.topMargin: backgroundContentIconMargin
            anchors.bottomMargin: backgroundContentIconMargin
            radius: scrollBarRadius
            color: Styles.iconPrimary
            opacity: scrollbar.pressed ? scrollBarPressedOpacity : 1.0
        }
    }

    Component {
        id: horizontalContent

        Rectangle {
            id: content

            anchors.left: parent.left
            anchors.top: parent.top
            anchors.right: parent.right
            anchors.topMargin: backgroundContentMargin
            anchors.leftMargin: backgroundContentIconMargin
            anchors.rightMargin: backgroundContentIconMargin
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

        Custom.SvgImage {
            id: arrowTop

            source: Images.arrowRight
            sourceSize: Qt.size(iconSize, iconSize)
            color: Styles.iconButton
            rotation: direction === ScrollBar.Direction.Vertical ? -90 : 180
            enabled: scrollbar.visualPosition !== 0

            MouseArea {
                anchors.fill: arrowTop
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

        Custom.SvgImage {
            id: arrowBottom

            source: Images.arrowRight
            sourceSize: Qt.size(iconSize, iconSize)
            color: Styles.iconButton
            rotation: direction === ScrollBar.Direction.Vertical ? 90 : 0
            enabled: visualPosition < (1.0 - visualSize)

            MouseArea {
                anchors.fill: arrowBottom
                cursorShape: Qt.PointingHandCursor
                onClicked: scrollbar.increase()
            }
        }
    }

}
