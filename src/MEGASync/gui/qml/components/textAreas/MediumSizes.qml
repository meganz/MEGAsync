import QtQuick 2.15

import common 1.0

import components.texts 1.0 as Texts

QtObject {
    id: root

    property bool adaptableHeight: false

    property real height: 36.0
    property real maxHeight: 0.0
    property real verticalContentMargin: 4.0
    property real rightContentMargin: 2.0
    property real heightContentAdjustment: 6.0
    property real rightTextAreaMargin: 6.0
    property real focusBorderRadius: 12.0
    property real borderRadius: 8.0
    property real hintTopMargin: 2.0
    property real lineHeight: 6.0
    property int focusBorderWidth: Constants.focusBorderWidth
    property int borderWidth: 1
    property int textSize: Texts.Text.Size.NORMAL
    property int hintTextSize: Texts.Text.Size.NORMAL

}
