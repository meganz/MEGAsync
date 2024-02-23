import QtQuick 2.15

Item {
    id: root

    //property alias style: loader.item
    property alias megaButtonRed: megaButtonRed
    property alias megaButtonGreen: megaButtonGreen

    Loader {
        id: loader

        source: "qrc:/common/themes/"+themeManager.theme+"/Category.qml"
    }


    Item {
        id: megaButtonRed

        readonly property color iconButton: loader.item.megaButtonRed.iconButton
    }

    Item {
        id: megaButtonGreen

        readonly property color iconButton: loader.item.megaButtonGreen.iconButton
    }
}

