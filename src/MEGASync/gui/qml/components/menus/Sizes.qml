import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts

QtObject {
    id: root

    // Medium sizes
    property int horizontalPadding: 4
    property int verticalPadding: 4
    property int itemHeight: 40
    property int itemContentSpacing: 12
    property real focusRadius: 6
    property int focusBorderWidth: Constants.focusBorderWidth
    property int iconWidth: 16
    property int iconHeight: 16
    property int textFontSize: Texts.Text.Size.MEDIUM
}
