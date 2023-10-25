// System
import QtQuick 2.15
import QtQuick.Layouts 1.15

// Local
import Common 1.0

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

    visible: false
    height: backgroundRect.height
    Layout.preferredHeight: backgroundRect.height

    onVisibleChanged: {
        if(visible && root.time > 0) {
            animationToVisible.start();
            visibilityTimer.start();
        } else if(visibilityTimer.running) {
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
                backgroundColor = Styles.notificationWarning;
                break;
            case Constants.MessageType.ERROR:
                backgroundColor = Styles.notificationError;
                break;
            default:
                console.error("NotificationText: Constants.MessageType -> " + type + " does not exist");
                break;
        }

        hint.type = type;
    }

    Rectangle {
        id: backgroundRect

        anchors.left: parent.left
        anchors.right: parent.right
        height: hint.height + 2 * root.margin
        radius: root.radius

        MouseArea {
            anchors.fill: parent
            hoverEnabled: true
        }

        HintText {
            id: hint

            anchors.top: parent.top
            anchors.left: parent.left
            anchors.right: parent.right
            anchors.margins: root.margin
        }
    }

    Rectangle {
        height: root.radius
        color: backgroundRect.color
        anchors.top: parent.top
        anchors.left: parent.left
        anchors.right: parent.right
        visible: root.topBorderRect
    }

    SequentialAnimation {
        id: animationToVisible

        NumberAnimation {
            target: root
            property: "visible"
            from: 0
            to: 1
            duration: 0
        }

        NumberAnimation {
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
            target: root
            property: "opacity"
            from: 1
            to: 0
            duration: 200
        }

        NumberAnimation {
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
