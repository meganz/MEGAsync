import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

import components.scrollBars 1.0

Item {
    id: root

    property alias flickable: flickable
    property alias showVerticalBar: scrollbarVertical.visible
    property alias showHorizontalBar: scrollbarHorizontal.visible

    property int scrollBarVerticalMargin: 8
    property int scrollBarHorizontalMargin: 8

    Keys.onUpPressed: {
        scrollbarVertical.decrease();
    }

    Keys.onDownPressed: {
        scrollbarVertical.increase();
    }

    Keys.onLeftPressed: {
        scrollbarHorizontal.decrease();
    }

    Keys.onRightPressed: {
        scrollbarHorizontal.increase();
    }

    Flickable {
        id: flickable

        anchors {
            left: parent.left
            top: parent.top
        }
        height: root.height
        width: root.width
        clip: true
        interactive: false

        MouseArea {
            id: flickableMouseArea

            anchors.fill: parent
            onWheel: {
                if(scrollbarVertical.visible) {
                    flickable.flick(0, wheel.angleDelta.y * 5);
                }
            }
        }

        Qml.ScrollBar.vertical: scrollbarVertical
        Qml.ScrollBar.horizontal: scrollbarHorizontal
    }

    ScrollBar {
        id: scrollbarVertical

        anchors {
            left: flickable.right
            top: flickable.top
            bottom: flickable.bottom
            leftMargin: scrollBarVerticalMargin
        }
        direction: ScrollBar.Direction.VERTICAL
    }

    ScrollBar {
        id: scrollbarHorizontal

        anchors {
            left: flickable.left
            right: flickable.right
            top: flickable.bottom
            topMargin: scrollBarHorizontalMargin
        }
        direction: ScrollBar.Direction.HORIZONTAL
    }

}
