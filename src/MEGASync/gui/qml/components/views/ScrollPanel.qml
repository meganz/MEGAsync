// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0
import Components 1.0 as Custom

Item {
    id: root

    property alias flickable: flickable
    property alias showVerticalBar: scrollbarVertical.visible
    property alias showHorizontalBar: scrollbarHorizontal.visible

    property int scrollBarVerticalMargin: 8
    property int scrollBarHorizontalMargin: 8

    activeFocusOnTab: true

    Flickable {
        id: flickable

        anchors.left: parent.left
        anchors.top: parent.top
        height: root.height
        width: root.width
        clip: true
        boundsBehavior: Flickable.StopAtBounds

        Qml.ScrollBar.vertical: scrollbarVertical
        Qml.ScrollBar.horizontal: scrollbarHorizontal

        focus: true
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

    Custom.ScrollBar {
        id: scrollbarVertical

        anchors {
            left: flickable.right
            top: flickable.top
            bottom: flickable.bottom
            leftMargin: scrollBarVerticalMargin
        }
        direction: Custom.ScrollBar.Direction.Vertical
    }

    Custom.ScrollBar {
        id: scrollbarHorizontal

        anchors {
            left: flickable.left
            right: flickable.right
            top: flickable.bottom
            topMargin: scrollBarHorizontalMargin
        }
        direction: Custom.ScrollBar.Direction.Horizontal
    }

}
