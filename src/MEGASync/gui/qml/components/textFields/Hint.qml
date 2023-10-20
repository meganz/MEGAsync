// System
import QtQuick 2.15

//Local
import Components.Texts 1.0 as MegaTexts

QtObject {
    property bool visible: false
    property url icon: ""
    property string title: ""
    property string text: ""
    property MegaTexts.HintStyle styles: MegaTexts.HintStyle {}

    onVisibleChanged: {
        if(!visible) {
            return;
        }

        hintLoader.sourceComponent = hintComponent;
    }
}

