import QtQuick 2.15

Item {
    property alias primaryButton: primaryButton
    property alias secondaryButton: secondaryButton

    Item {
        id: primaryButton

        readonly property color background: "#FF0000"
    }

    Item {
        id: secondaryButton

        readonly property color background: "#FF0000"
    }
}
