// System
import QtQuick 2.12

//Local
import Components 1.0

QtObject {
    property bool visible: false
    property url icon: ""
    property string title: ""
    property string text: ""
    property HintStyle styles: HintStyle {}

    onVisibleChanged: {
        if(!visible) {
            return;
        }

        hintLoader.sourceComponent = hintComponent;
    }
}

