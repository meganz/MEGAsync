import QtQuick 2.15
import QtQuick.Controls 2.15

StackView {
    id: root

    replaceEnter: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 0.0
            to: 1.0
            duration: 100
            easing.type: Easing.OutQuad
        }
    }

    replaceExit: Transition {
        PropertyAnimation {
            property: "opacity"
            from: 1.0
            to: 0.0
            duration: 100
            easing.type: Easing.InQuad
        }
    }
}
