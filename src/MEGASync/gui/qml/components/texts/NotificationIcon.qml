// System
import QtQuick 2.12

QtObject {

    property string source
    property bool visible: true
    property size size: Qt.size(16, 16)

    onSourceChanged: {
        if(source.length === 0 || !visible) {
            return;
        }

        iconLoader.sourceComponent = iconComponent;
    }

}
