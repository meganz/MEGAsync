import QtQuick 2.15

Item {
    property alias megaButtonRed: megaButtonRed
    property alias megaButtonGreen: megaButtonGreen

    Item {
        id: megaButtonRed

        readonly property color iconButton: "#00FF00"
    }

    Item {
        id: megaButtonGreen

        readonly property color iconButton: "#00FF00"
    }
}
