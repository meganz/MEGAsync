import QtQuick 2.15
import QtQuick.Controls 2.15 as Qml

import common 1.0

Qml.ProgressBar {
    id: root

    height: 2
    value: 0.0

    background: Rectangle {
        id: backgroundRect

        anchors.fill: parent
        radius: height
        color: colorStyle.indicatorBackground
    }

    contentItem: Item {
        id: content

        Rectangle {
            id: contentRect

            width: root.visualPosition * parent.width
            height: parent.height
            radius: height
            color: colorStyle.buttonPrimary
            visible: !indeterminate
        }

        Rectangle {
            id: indeterminateRect

            width: 30
            height: parent.height
            color: colorStyle.buttonPrimary
            visible: indeterminate

            SequentialAnimation {
                id: animation

                running: true
                loops: Animation.Infinite

                XAnimator {
                    id: animatorStart

                    target: indeterminateRect
                    from: 0
                    to: root.width - indeterminateRect.width
                    duration: 1500
                }

                XAnimator {
                    id: animatorFinal

                    target: indeterminateRect
                    from: root.width - indeterminateRect.width
                    to: 0
                    duration: 1500
                }
            }
        }

    } // Item: content

}
