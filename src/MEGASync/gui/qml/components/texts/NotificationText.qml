import QtQuick 2.15
import QtQuick.Layouts 1.15

import common 1.0

import components.texts 1.0 as Texts

BannerText {
    id: root

    property int time: 0

    signal visibilityTimerFinished

    onVisibleChanged: {
        if(visible && root.time > 0) {
            animationToVisible.start();
            visibilityTimer.start();
        }
        else if(visibilityTimer.running) {
            visibilityTimer.stop();
        }
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
