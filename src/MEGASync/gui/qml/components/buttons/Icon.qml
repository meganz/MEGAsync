// System
import QtQuick 2.12

// Local
import Common 1.0

QtObject {

    enum Position {
        LEFT = 0,
        RIGHT,
        BOTH
    }

    property color colorEnabled: Styles.textInverseAccent
    property color colorDisabled: Styles.textDisabled
    property color colorHovered: Styles.textInverseAccent
    property color colorPressed: Styles.textInverseAccent
    property string source
    property int position: Icon.Position.RIGHT
    property int busyIndicatorPosition: Icon.Position.RIGHT
    property bool busyIndicatorVisible: false

    onSourceChanged: {
        switch(position) {
            case Icon.Position.LEFT:
                leftLoader.sourceComponent = image;
                break;
            case Icon.Position.RIGHT:
                rightLoader.sourceComponent = image;
                break;
            case Icon.Position.BOTH:
                leftLoader.sourceComponent = image;
                rightLoader.sourceComponent = image;
                break;
        }
    }

    onBusyIndicatorVisibleChanged: {
        if(busyIndicatorVisible) {
            switch(busyIndicatorPosition) {
                case Icon.Position.LEFT:
                    leftLoader.sourceComponent = busyIndicator;
                    break;
                case Icon.Position.RIGHT:
                    rightLoader.sourceComponent = busyIndicator;
                    break;
                case Icon.Position.BOTH:
                    leftLoader.sourceComponent = busyIndicator;
                    rightLoader.sourceComponent = busyIndicator;
                    break;
            }
        } else {
            sourceChanged();
        }
    }
}

