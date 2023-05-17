// System
import QtQuick 2.12
import QtQml 2.12

//Local
import Common 1.0

Item {
    id: root
    property int duration: 1000 //ms
    property double value: 0 // from 0 to 1

    signal animationFinished(bool completed)

    onValueChanged: {
        if(value > 0)
        {
            backgroundLoader.sourceComponent = progressBarComp
        }

        if(value < 1)
        {
            if(busyTimer.running)
            {
                return;
            }
            busyTimer.start();
        }

        if(value > 1)
        {
            value = 1;
        }
        root.width = backgroundRect.width * value
    }

    Component {
        id: progressBarComp
        Rectangle {
            id: progressBarRect
            height: backgroundRect.height
            width: root.width
            color: Styles.buttonPrimaryPressed

            Behavior on width {
                NumberAnimation {
                    id: animation
                    duration: duration
                    onRunningChanged: {
                        if(!running)
                        {
                            var finished = progressBarRect.width === backgroundRect.width;
                            animationFinished(finished);
                            if(finished)
                            {
                                backgroundLoader.destroy();
                            }
                        }
                    }
                }
            }
        }
    }
}

