// System
import QtQuick 2.12
import QtQuick.Controls 2.12 as Qml
import QtQuick.Layouts 1.12

// Local
import Components 1.0 as Custom
import Common 1.0

Qml.RoundButton {
    id: button

    enum Position{
        LEFT = 0,
        RIGHT,
        BOTH
    }

    property alias color: backgroundRect.color
    property alias textColor: buttonText.color
    property color borderColor: borderColor
    property color iconColor: iconColor
    property string iconSource: ""
    property int decorationPosition: Button.Position.RIGHT
    property int busyIndicatorPosition: Button.Position.RIGHT
    property bool busyIndicatorVisible: false
    property size iconSize: Qt.size(16, 16)
    property string busyIndicatorImage: ""
    property double progressValue: 0 // from 0 to 1
    property bool progressBar: false
    property int animationDuration: 1000 //ms

    signal animationFinished(bool completed)

    bottomPadding: 8
    topPadding: 8
    leftPadding: 16
    rightPadding: 16

    onProgressValueChanged: {
        if(progressValue > 1)
        {
            progressValue = 1;
        }
    }

    onProgressBarChanged: {
        if(progressBar)
        {
            backgroundLoader.sourceComponent = progressBarComp
        }
        else
        {
            backgroundLoader.destroy();
        }
    }

    onBusyIndicatorVisibleChanged:
    {
        if(busyIndicatorVisible)
        {
            switch(busyIndicatorPosition)
            {
            case Button.Position.LEFT:
                leftLoader.sourceComponent = busyIndicator;
                break;
            case Button.Position.RIGHT:
                rightLoader.sourceComponent = busyIndicator;
                break;
            case Button.Position.BOTH:
                leftLoader.sourceComponent = busyIndicator;
                rightLoader.sourceComponent = busyIndicator;
                break;
            }
        }
        else
        {
            decorationPositionChanged();
        }
    }

    onDecorationPositionChanged: {
        switch(decorationPosition)
        {
        case Button.Position.LEFT:
            leftLoader.sourceComponent = image;
            break;
        case Button.Position.RIGHT:
            rightLoader.sourceComponent = image;
            break;
        case Button.Position.BOTH:
            leftLoader.sourceComponent = image;
            rightLoader.sourceComponent = image;
            break;
        }
    }

    contentItem: RowLayout {
        width: button.width
        height: button.height
        spacing: 8

        Loader {
            id: leftLoader
        }

        Text {
            id: buttonText
            text: button.text
            horizontalAlignment: Text.AlignHCenter
            verticalAlignment: Text.AlignVCenter
            font {
                pixelSize: 14
                weight: Font.DemiBold
            }
        }

        Loader {
            id: rightLoader
        }
    }

    background: Rectangle {
        id: backgroundRect

        width: button.width
        height: button.height
        border.width: 2
        border.color: borderColor
        radius: 6
        opacity: button.enabled || busyIndicatorVisible? 1.0 : 0.5

        Loader{
            id: backgroundLoader
        }
    }

    MouseArea {
        id: mouseArea

        anchors.fill: parent
        onPressed: {mouse.accepted = false;}
        cursorShape: Qt.PointingHandCursor
    }

    Component {
        id: progressBarComp
        Rectangle {
            id: progressBarRect
            anchors.left: parent.left
            height: backgroundRect.height
            width: backgroundRect.width * progressValue
            radius: 6
            color: Styles.buttonPrimaryPressed

            Behavior on width {
                NumberAnimation {
                    id: animation
                    duration: animationDuration
                    onRunningChanged: {
                        if(!running)
                        {
                            var finished = progressBarRect.width === backgroundRect.width;
                            animationFinished(finished);
                            progressBar = !finished;
                        }
                    }
                }
            }
        }
    }

    Component {
        id: image
        Custom.SvgImage {
            visible: button.iconSource.length
            source: button.iconSource
            color: iconColor
            sourceSize: button.iconSize
        }
    }
    Component {
        id: busyIndicator
        Custom.BusyIndicator {
            imageSource: busyIndicatorImage
            color: iconColor
        }
    }
}

