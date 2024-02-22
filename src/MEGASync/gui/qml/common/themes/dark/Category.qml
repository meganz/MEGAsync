import QtQuick 2.15

Item {
    property alias megaButtonRed: itemMegaButtonRed
    property alias megaButtonGreen: itemMegaButtonGreen

    Item {
        id: itemMegaButtonRed

        readonly property color iconButton: "#FF0000"
    }

    Item {
        id: itemMegaButtonGreen

        readonly property color iconButton: "#00FF00"
    }
}
