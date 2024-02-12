import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

Item {
    id: root

    property alias icon: hint.icon
    property alias title: hint.title
    property alias text: hint.text
    property alias iconColor: hint.iconColor
    property alias titleColor: hint.titleColor
    property alias textColor: hint.textColor
    property alias backgroundColor: backgroundRect.color

    property int type: Constants.MessageType.NONE
    property int time: 0
    property int margin: 12
    property int radius: 8
    property bool topBorderRect: false

    signal visibilityTimerFinished

    height: backgroundRect.height
    Layout.preferredHeight: backgroundRect.height
    visible: false

    onVisibleChanged: {
        if(visible && root.time > 0) {
            animationToVisible.start();
            visibilityTimer.start();
        }
        else if(visibilityTimer.running) {
            visibilityTimer.stop();
        }
    }

    onTypeChanged: {
        switch(type) {
            case Constants.MessageType.NONE:
            case Constants.MessageType.SUCCESS:
            case Constants.MessageType.INFO:
                console.warn("NotificationText: Constants.MessageType -> " + type + " not defined yet");
                break;
            case Constants.MessageType.WARNING:
                backgroundColor = colorStyle.notificationWarning;
                break;
            case Constants.MessageType.ERROR:
                backgroundColor = colorStyle.notificationError;
                break;
            default:
                console.error("NotificationText: Constants.MessageType -> " + type + " does not exist");
                break;
        }

        hint.type = type;
    }

    Rectangle {
        id: backgroundRect

        anchors {
            left: parent.left
            right: parent.right
        }
        height: hint.height + 2 * root.margin
        radius: root.radius

        MouseArea {
            id: backgroundMouseArea

            anchors.fill: parent
            hoverEnabled: true
        }

        Texts.HintText {
            id: hint

            anchors {
                top: parent.top
                left: parent.left
                right: parent.right
                margins: root.margin
            }
            textSpacing: 4
        }
    }

    Rectangle {
        id: topBorderRectangle

        anchors {
            top: parent.top
            left: parent.left
            right: parent.right
        }
        height: root.radius
        color: backgroundRect.color
        visible: root.topBorderRect
    }

    SequentialAnimation {
        id: animationToVisible

        NumberAnimation {
            id: animationStart

            target: root
            property: "visible"
            from: 0
            to: 1
            duration: 0
        }

        NumberAnimation {
            id: animationFinal

            target: root
            property: "opacity"
            from: 0
            to: 1
            duration: 200
        }
    }

    SequentialAnimation {
        id: animationToInvisible

        NumberAnimation {
            id: animationStartInvisible

            target: root
            property: "opacity"
            from: 1
            to: 0
            duration: 200
        }

        NumberAnimation {
            id: animationFinalInvisible

            target: root
            property: "visible"
            from: 1
            to: 0
            duration: 0
        }

        onFinished: {
            visibilityTimerFinished();
        }
    }

    Timer {
        id: visibilityTimer

        interval: root.time
        running: false
        repeat: false
        onTriggered: {
            animationToInvisible.start();
        }
    }
}
