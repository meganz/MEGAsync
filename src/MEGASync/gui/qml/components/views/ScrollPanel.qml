// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0
import Components.ScrollBars 1.0 as MegaScrollBar

Item {
    id: root

    property alias flickable: flickable
    property alias showVerticalBar: scrollbarVertical.visible
    property alias showHorizontalBar: scrollbarHorizontal.visible

    property int scrollBarVerticalMargin: 8
    property int scrollBarHorizontalMargin: 8

    Flickable {
        id: flickable

        anchors.left: parent.left
        anchors.top: parent.top
        height: root.height
        width: root.width
        clip: true
        interactive: false

        MouseArea {
            anchors.fill: parent
            onWheel: if(scrollbarVertical.visible) {
                        flickable.flick(0, wheel.angleDelta.y * 5);
                    }
        }

        Qml.ScrollBar.vertical: scrollbarVertical
        Qml.ScrollBar.horizontal: scrollbarHorizontal
    }

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

    MegaScrollBar.ScrollBar {
        id: scrollbarVertical

        anchors {
            left: flickable.right
            top: flickable.top
            bottom: flickable.bottom
            leftMargin: scrollBarVerticalMargin
        }
        direction: MegaScrollBar.ScrollBar.Direction.Vertical
    }

    MegaScrollBar.ScrollBar {
        id: scrollbarHorizontal

        anchors {
            left: flickable.left
            right: flickable.right
            top: flickable.bottom
            topMargin: scrollBarHorizontalMargin
        }
        direction: MegaScrollBar.ScrollBar.Direction.Horizontal
    }

}
