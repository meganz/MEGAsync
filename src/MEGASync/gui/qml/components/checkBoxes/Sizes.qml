import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    property int indicatorWidth: 16
    property int indicatorRadius: 4
    property int indicatorBorderWidth: 2
    property size iconSize: Qt.size(8, 8)
    property size iconSizeIndeterminate: Qt.size(8, 2)
    property int spacing: 4
    property int focusBorderWidth: Constants.focusBorderWidth
    property int focusBorderRadius: 8
}
