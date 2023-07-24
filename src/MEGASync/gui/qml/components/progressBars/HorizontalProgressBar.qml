// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml

// Local
import Common 1.0

Qml.ProgressBar {
    id: progressBar

    height: 2
    value: 0.0

    background: Rectangle {
        anchors.fill: parent
        radius: height
        color: Styles.indicatorBackground
    }

    contentItem: Item {
        Rectangle {
            width: progressBar.visualPosition * parent.width
            height: parent.height
            radius: height
            color: Styles.buttonPrimary
            visible: !indeterminate
        }

        Rectangle {
            id: indeterminateRect

            width: 30
            height: parent.height
            color: Styles.buttonPrimary
            visible: indeterminate

            SequentialAnimation {

                running: true
                loops: Animation.Infinite

                XAnimator {
                    target: indeterminateRect
                    from: 0
                    to: progressBar.width - indeterminateRect.width
                    duration: 1500
                }

                XAnimator {
                    target: indeterminateRect
                    from: progressBar.width - indeterminateRect.width
                    to: 0
                    duration: 1500
                }

            }
        }
    }

}
