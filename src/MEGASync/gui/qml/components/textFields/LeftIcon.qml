// System
import QtQuick 2.15

//Local
import Common 1.0

QtObject {
    property bool visible: false
    property url source: ""
    property color color: enabled ? colors.icon : colors.iconDisabled

    onSourceChanged: {
        if(source === "") {
            return;
        }

        visible = true;
        leftIconLoader.sourceComponent = leftIconComponent;
    }
}

