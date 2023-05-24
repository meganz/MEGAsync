// System
import QtQuick 2.12

//Local
import Common 1.0

QtObject {
    property bool visible: false
    property url source: ""
    property color color: enabled ? Styles.iconSecondary : Styles.iconDisabled

    onSourceChanged: {
        if(source === "") {
            return;
        }

        visible = true;
        leftIconLoader.sourceComponent = leftIconComponent;
    }
}

