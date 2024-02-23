pragma Singleton

import QtQuick 2.15

Item {
    id: root

    Loader {
        id: loader

        source: "qrc:/common/themes/"+themeManager.theme+"/Category.qml"
    }

    property alias primaryButton: primaryButton
    property alias secondaryButton: secondaryButton


    Item {
        id: primaryButton

        readonly property color background: loader.item.primaryButton.background
    }

    Item {
        id: secondaryButton

        readonly property color background: loader.item.secondaryButton.background
    }
}

