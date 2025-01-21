import QtQuick 2.15

import common 1.0

QtObject {
    id: root

    // Medium sizes
    property int spacing: 4
    property int focusBorderWidth: Constants.focusBorderWidth
    property int externalBorderWidth: Constants.focusBorderWidth / 2
    property int focusWidth: 24
    property int externalCircleWidth: 16
    property int internalCircleWidthPressed: 6
    property int internalCircleWidthHover: 10
    property int internalCircleWidth: 8

}
