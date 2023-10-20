// System
import QtQuick 2.15
import QtQml 2.15

//Local
import Common 1.0

Item {
    id: root

    property double value: 0 // from 0 to 1

    Rectangle {
        id: progressBarRect

        height: root.height
        width: root.width * root.value
        color: Styles.buttonPrimaryPressed
    }
}

